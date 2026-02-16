// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/OutlawEnemySpawner.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/AssetManager.h"

AOutlawEnemySpawner::AOutlawEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AOutlawEnemySpawner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOutlawEnemySpawner, CurrentWaveIndex);
	DOREPLIFETIME(AOutlawEnemySpawner, CurrentSpawnCount);
	DOREPLIFETIME(AOutlawEnemySpawner, bIsSpawning);
}

void AOutlawEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnerMode == EOutlawSpawnerMode::WaveBased && HasAuthority())
	{
		StartSpawning();
	}
}

void AOutlawEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority() || !bIsSpawning)
	{
		return;
	}

	if (SpawnerMode == EOutlawSpawnerMode::WaveBased)
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

		const FOutlawWaveSpawnData& CurrentWave = Waves[CurrentWaveIndex];
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
	else if (SpawnerMode == EOutlawSpawnerMode::Ambient)
	{
		AmbientSpawnTimer += DeltaTime;

		if (AmbientSpawnTimer >= AmbientSpawnInterval && CanSpawn() && !Waves.IsEmpty())
		{
			const FOutlawWaveSpawnData& SpawnData = Waves[FMath::RandRange(0, Waves.Num() - 1)];
			SpawnEnemy(SpawnData.EnemyClass);
			AmbientSpawnTimer = 0.f;
		}
	}
}

void AOutlawEnemySpawner::StartSpawning()
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

	if (SpawnerMode == EOutlawSpawnerMode::WaveBased && !Waves.IsEmpty())
	{
		OnWaveStarted.Broadcast(CurrentWaveIndex);
	}
}

void AOutlawEnemySpawner::StopSpawning()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsSpawning = false;
}

void AOutlawEnemySpawner::TriggerSpawn(int32 Count)
{
	if (!HasAuthority() || SpawnerMode != EOutlawSpawnerMode::Triggered)
	{
		return;
	}

	for (int32 i = 0; i < Count; ++i)
	{
		if (!CanSpawn() || Waves.IsEmpty())
		{
			break;
		}

		const FOutlawWaveSpawnData& SpawnData = Waves[FMath::RandRange(0, Waves.Num() - 1)];
		SpawnEnemy(SpawnData.EnemyClass);
	}
}

void AOutlawEnemySpawner::SpawnEnemy(const TSoftClassPtr<AActor>& EnemyClass)
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
		SpawnedEnemy->OnDestroyed.AddDynamic(this, &AOutlawEnemySpawner::OnEnemyDestroyed);
	}
}

void AOutlawEnemySpawner::OnEnemyDestroyed(AActor* DestroyedActor)
{
	ActiveEnemies.Remove(DestroyedActor);
}

bool AOutlawEnemySpawner::CanSpawn() const
{
	return ActiveEnemies.Num() < MaxActiveEnemies;
}

FVector AOutlawEnemySpawner::GetRandomSpawnLocation() const
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
