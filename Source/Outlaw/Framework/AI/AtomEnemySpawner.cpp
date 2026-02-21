// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/AtomEnemySpawner.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/AssetManager.h"

AAtomEnemySpawner::AAtomEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AAtomEnemySpawner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAtomEnemySpawner, CurrentWaveIndex);
	DOREPLIFETIME(AAtomEnemySpawner, CurrentSpawnCount);
	DOREPLIFETIME(AAtomEnemySpawner, bIsSpawning);
}

void AAtomEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnerMode == EAtomSpawnerMode::WaveBased && HasAuthority())
	{
		StartSpawning();
	}
}

void AAtomEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority() || !bIsSpawning)
	{
		return;
	}

	if (SpawnerMode == EAtomSpawnerMode::WaveBased)
	{
		if (CurrentWaveIndex >= Waves.Num())
		{
			if (ActiveEnemies.IsEmpty())
			{
				OnAllWavesCompleted.Broadcast();
				bIsSpawning = false;
			}
			return;
		}

		const FAtomWaveSpawnData& CurrentWave = Waves[CurrentWaveIndex];
		SpawnTimer += DeltaTime;

		if (SpawnTimer >= CurrentWave.SpawnInterval && CurrentSpawnCount < CurrentWave.Count)
		{
			if (CanSpawn())
			{
				SpawnEnemy(CurrentWave.EnemyClass);
				CurrentSpawnCount++;
				SpawnTimer = 0.f;
			}
		}

		if (CurrentSpawnCount >= CurrentWave.Count && ActiveEnemies.IsEmpty())
		{
			OnWaveCompleted.Broadcast(CurrentWaveIndex);
			CurrentWaveIndex++;
			CurrentSpawnCount = 0;
			SpawnTimer = 0.f;

			if (CurrentWaveIndex < Waves.Num())
			{
				OnWaveStarted.Broadcast(CurrentWaveIndex);
			}
		}
	}
	else if (SpawnerMode == EAtomSpawnerMode::Ambient)
	{
		AmbientSpawnTimer += DeltaTime;

		if (AmbientSpawnTimer >= AmbientSpawnInterval && CanSpawn() && !Waves.IsEmpty())
		{
			const FAtomWaveSpawnData& SpawnData = Waves[FMath::RandRange(0, Waves.Num() - 1)];
			SpawnEnemy(SpawnData.EnemyClass);
			AmbientSpawnTimer = 0.f;
		}
	}
}

void AAtomEnemySpawner::StartSpawning()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsSpawning = true;
	CurrentWaveIndex = 0;
	CurrentSpawnCount = 0;
	SpawnTimer = 0.f;
	AmbientSpawnTimer = 0.f;

	if (SpawnerMode == EAtomSpawnerMode::WaveBased && !Waves.IsEmpty())
	{
		OnWaveStarted.Broadcast(CurrentWaveIndex);
	}
}

void AAtomEnemySpawner::StopSpawning()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsSpawning = false;
}

void AAtomEnemySpawner::TriggerSpawn(int32 Count)
{
	if (!HasAuthority() || SpawnerMode != EAtomSpawnerMode::Triggered)
	{
		return;
	}

	for (int32 i = 0; i < Count; ++i)
	{
		if (!CanSpawn() || Waves.IsEmpty())
		{
			break;
		}

		const FAtomWaveSpawnData& SpawnData = Waves[FMath::RandRange(0, Waves.Num() - 1)];
		SpawnEnemy(SpawnData.EnemyClass);
	}
}

void AAtomEnemySpawner::SpawnEnemy(const TSoftClassPtr<AActor>& EnemyClass)
{
	if (!HasAuthority() || !EnemyClass.IsValid())
	{
		return;
	}

	UClass* LoadedClass = EnemyClass.LoadSynchronous();
	if (!LoadedClass)
	{
		return;
	}

	FVector SpawnLocation = GetRandomSpawnLocation();
	FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedEnemy = GetWorld()->SpawnActor<AActor>(LoadedClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (SpawnedEnemy)
	{
		ActiveEnemies.Add(SpawnedEnemy);
		SpawnedEnemy->OnDestroyed.AddDynamic(this, &AAtomEnemySpawner::OnEnemyDestroyed);
	}
}

void AAtomEnemySpawner::OnEnemyDestroyed(AActor* DestroyedActor)
{
	ActiveEnemies.Remove(DestroyedActor);
}

bool AAtomEnemySpawner::CanSpawn() const
{
	return ActiveEnemies.Num() < MaxActiveEnemies;
}

FVector AAtomEnemySpawner::GetRandomSpawnLocation() const
{
	FVector BaseLocation = GetActorLocation();
	FVector RandomOffset = FVector(
		FMath::FRandRange(-SpawnRadius, SpawnRadius),
		FMath::FRandRange(-SpawnRadius, SpawnRadius),
		0.f
	);

	FVector SpawnLocation = BaseLocation + RandomOffset;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation NavResult;
		if (NavSys->GetRandomReachablePointInRadius(SpawnLocation, 200.f, NavResult))
		{
			return NavResult.Location;
		}
	}

	return SpawnLocation;
}
