// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AtomProgressionTypes.h"
#include "AtomLevelingConfig.generated.h"

/**
 * Data-driven XP table. Each entry maps to a level (index 0 = level 1).
 * Defines XP thresholds and skill point awards per level.
 */
UCLASS(BlueprintType, Const)
class OUTLAW_API UAtomLevelingConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAtomLevelingConfig(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** XP table — index 0 = level 1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Leveling")
	TArray<FAtomXPLevelEntry> LevelTable;

	/** Fallback skill points per level if the entry's SkillPointsAwarded is 0. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Leveling", meta = (ClampMin = "0"))
	int32 DefaultSkillPointsPerLevel = 1;

	/** Returns the maximum achievable level (LevelTable.Num()). */
	UFUNCTION(BlueprintCallable, Category = "Leveling")
	int32 GetMaxLevel() const;

	/** Returns the total XP required to reach the given level. */
	UFUNCTION(BlueprintCallable, Category = "Leveling")
	int32 GetXPForLevel(int32 Level) const;

	/** Returns the highest level achievable with the given total XP. */
	UFUNCTION(BlueprintCallable, Category = "Leveling")
	int32 GetLevelForXP(int32 TotalXP) const;

	/** Returns skill points awarded at a specific level. */
	UFUNCTION(BlueprintCallable, Category = "Leveling")
	int32 GetSkillPointsForLevel(int32 Level) const;

	/** Returns the cumulative skill points earned from level 1 through Level. */
	UFUNCTION(BlueprintCallable, Category = "Leveling")
	int32 GetTotalSkillPointsForLevel(int32 Level) const;
};
