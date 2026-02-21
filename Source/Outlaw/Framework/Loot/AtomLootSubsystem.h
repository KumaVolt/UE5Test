// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AtomLootTypes.h"
#include "AtomLootSubsystem.generated.h"

class UAtomLootTable;
class AAtomLootPickup;

UCLASS()
class OUTLAW_API UAtomLootSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UAtomLootSubsystem();

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void SpawnLoot(const FVector& DeathLocation, UAtomLootTable* LootTable, int32 EnemyLevel, float RarityBonus, int32 NumDrops = 1);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot|Config")
	TSubclassOf<AAtomLootPickup> LootPickupClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot|Config")
	float ScatterRadius = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot|Config")
	float DropHeight = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Loot|Config")
	bool bAutoLootPickups = false;

private:
	void SpawnSinglePickup(const FAtomLootDrop& Drop, const FVector& SpawnLocation);
};
