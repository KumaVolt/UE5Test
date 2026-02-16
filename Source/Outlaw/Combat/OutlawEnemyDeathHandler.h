// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OutlawDeathTypes.h"
#include "OutlawEnemyDeathHandler.generated.h"

class UOutlawLootTable;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UOutlawEnemyDeathHandler : public UActorComponent
{
	GENERATED_BODY()

public:
	UOutlawEnemyDeathHandler(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	float RagdollDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	float DissolveDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	FVector DeathImpulse = FVector(0.f, 0.f, 500.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	int32 BaseXPReward = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	TObjectPtr<UOutlawLootTable> LootTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	int32 NumLootDrops = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	float RarityBonus = 0.0f;

	UPROPERTY(BlueprintAssignable, Category = "Death")
	FOnLootDropRequested OnLootDropRequested;

	UPROPERTY(BlueprintAssignable, Category = "Death")
	FOnXPAwarded OnXPAwarded;

private:
	UFUNCTION()
	void OnDeathStarted(AActor* Killer);

	void StartDissolve();
	void SpawnLootAndDestroy();

	FTimerHandle RagdollTimerHandle;
	FTimerHandle DissolveTimerHandle;
	TWeakObjectPtr<AActor> KillerActor;
	FVector DeathLocation;
	int32 EnemyLevel = 1;
};
