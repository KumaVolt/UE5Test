// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AtomLootTypes.h"
#include "AtomLootTable.generated.h"

UCLASS(BlueprintType, Const)
class OUTLAW_API UAtomLootTable : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAtomLootTable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot Table")
	TArray<FAtomLootTableEntry> Entries;

	TArray<FAtomLootDrop> RollLoot(int32 EnemyLevel, int32 NumDrops, float RarityBonus) const;
};
