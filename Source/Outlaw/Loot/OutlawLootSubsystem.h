// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OutlawLootTypes.h"
#include "OutlawLootSubsystem.generated.h"

class UOutlawLootTable;
class AOutlawLootPickup;

UCLASS()
class OUTLAW_API UOutlawLootSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UOutlawLootSubsystem();

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void SpawnLoot(const FVector& DeathLocation, UOutlawLootTable* LootTable, int32 EnemyLevel, float RarityBonus, int32 NumDrops = 1);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot|Config")
	TSubclassOf<AOutlawLootPickup> LootPickupClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot|Config")
	float ScatterRadius = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot|Config")
	float DropHeight = 50.0f;

private:
	void SpawnSinglePickup(const FOutlawLootDrop& Drop, const FVector& SpawnLocation);
};
