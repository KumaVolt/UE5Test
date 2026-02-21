// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AI/AtomAITypes.h"
#include "AtomEnemySpawner.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStartedSignature, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveCompletedSignature, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCompletedSignature);

UCLASS(Blueprintable, ClassGroup=(Custom))
class OUTLAW_API AAtomEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AAtomEnemySpawner();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StopSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void TriggerSpawn(int32 Count);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	EAtomSpawnerMode SpawnerMode = EAtomSpawnerMode::WaveBased;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TArray<FAtomWaveSpawnData> Waves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	int32 MaxActiveEnemies = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float AmbientSpawnInterval = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float SpawnRadius = 500.f;

	UPROPERTY(BlueprintAssignable, Category = "Spawner")
	FOnWaveStartedSignature OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "Spawner")
	FOnWaveCompletedSignature OnWaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Spawner")
	FOnAllWavesCompletedSignature OnAllWavesCompleted;

private:
	UPROPERTY(Replicated)
	int32 CurrentWaveIndex = 0;

	UPROPERTY(Replicated)
	int32 CurrentSpawnCount = 0;

	UPROPERTY(Replicated)
	bool bIsSpawning = false;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActiveEnemies;

	float SpawnTimer = 0.f;
	float AmbientSpawnTimer = 0.f;

	void SpawnEnemy(const TSoftClassPtr<AActor>& EnemyClass);
	void OnEnemyDestroyed(AActor* DestroyedActor);
	bool CanSpawn() const;
	FVector GetRandomSpawnLocation() const;
};
