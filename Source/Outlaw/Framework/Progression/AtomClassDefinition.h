// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OutlawProgressionTypes.h"
#include "OutlawClassDefinition.generated.h"

class UOutlawAbilitySet;
class UOutlawSkillTreeNodeDefinition;
class UOutlawLevelingConfig;

/**
 * Defines a character class — either a fixed class (Outriders-style) or
 * an ascendancy class (PoE 2-style). Contains stat growth, skill tree nodes,
 * and optional ascendancy sub-classes.
 */
UCLASS(BlueprintType, Const)
class OUTLAW_API UOutlawClassDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UOutlawClassDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ── Identity ────────────────────────────────────────────────

	/** Unique identity tag (e.g. Class.Devastator or Ascendancy.Slayer). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class", meta = (Categories = "Class"))
	FGameplayTag ClassTag;

	/** Display name shown in class selection UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class")
	FText DisplayName;

	/** Description shown in class selection UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class", meta = (MultiLine = true))
	FText Description;

	/** Icon for class selection UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class")
	TSoftObjectPtr<UTexture2D> ClassIcon;

	// ── Mode & Configuration ────────────────────────────────────

	/** Determines skill tree behavior: FixedClass or AscendancyClass. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class")
	EOutlawClassMode ClassMode = EOutlawClassMode::FixedClass;

	/** True for base classes, false for ascendancy sub-classes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class")
	bool bIsBaseClass = true;

	// ── Stat Growth ─────────────────────────────────────────────

	/** Per-level stat scaling (e.g. MaxHealth +12/level, Strength +2/level). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	TArray<FOutlawStatGrowthEntry> StatGrowthTable;

	// ── Abilities & Skill Tree ──────────────────────────────────

	/** Abilities granted immediately upon selecting this class. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UOutlawAbilitySet> ClassAbilitySet;

	/** All skill tree nodes for this class. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Tree")
	TArray<TObjectPtr<UOutlawSkillTreeNodeDefinition>> SkillTreeNodes;

	// ── Ascendancy ──────────────────────────────────────────────

	/** Sub-classes selectable from this base class. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascendancy", meta = (EditCondition = "bIsBaseClass"))
	TArray<TObjectPtr<UOutlawClassDefinition>> AvailableAscendancies;

	/** Minimum level required to select an ascendancy. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascendancy", meta = (ClampMin = "0", EditCondition = "bIsBaseClass"))
	int32 AscendancyRequiredLevel = 0;

	// ── Leveling Override ───────────────────────────────────────

	/** XP table override for this class. If null, the component default is used. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Leveling")
	TObjectPtr<UOutlawLevelingConfig> LevelingConfig;

	// ── API ─────────────────────────────────────────────────────

	/** Returns the base stat value for a given attribute at a given level (StatGrowthTable ValuePerLevel * Level). */
	UFUNCTION(BlueprintCallable, Category = "Class")
	float GetAttributeBaseValueAtLevel(FGameplayAttribute Attribute, int32 Level) const;

	/** Finds a skill tree node definition by its tag. Returns nullptr if not found. */
	UFUNCTION(BlueprintCallable, Category = "Class")
	UOutlawSkillTreeNodeDefinition* FindSkillNode(FGameplayTag NodeTag) const;
};
