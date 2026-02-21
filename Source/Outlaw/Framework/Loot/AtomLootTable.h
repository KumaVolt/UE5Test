// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OutlawLootTypes.h"
#include "OutlawLootTable.generated.h"

UCLASS(BlueprintType, Const)
class OUTLAW_API UOutlawLootTable : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UOutlawLootTable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot Table")
	TArray<FOutlawLootTableEntry> Entries;

	TArray<FOutlawLootDrop> RollLoot(int32 EnemyLevel, int32 NumDrops, float RarityBonus) const;
};
