// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"
#include "OutlawProgressionTypes.generated.h"

// ── Enums ───────────────────────────────────────────────────────

/** Determines the class's skill tree behavior. */
UENUM(BlueprintType)
enum class EOutlawClassMode : uint8
{
	/** Outriders-style: predefined class with skill tree + auto-unlock abilities per level. */
	FixedClass,
	/** PoE 2-style: base class -> ascendancy specialization with node-based skill tree. */
	AscendancyClass,
};

/** How a skill tree node is unlocked. */
UENUM(BlueprintType)
enum class EOutlawSkillNodeUnlockType : uint8
{
	/** Player manually allocates using skill points. Requires prerequisites to be met. */
	Manual,
	/** Automatically granted when the character reaches a specific level. */
	AutoOnLevel,
};

// ── Structs ─────────────────────────────────────────────────────

/** A single row in the XP table — defines XP required and points awarded for one level. */
USTRUCT(BlueprintType)
struct FOutlawXPLevelEntry
{
	GENERATED_BODY()

	/** Total XP required to reach this level. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 RequiredXP = 0;

	/** Skill points awarded upon reaching this level. 0 = use config default. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 SkillPointsAwarded = 0;
};

/** Per-level stat scaling for a class (e.g. MaxHealth +12/level). */
USTRUCT(BlueprintType)
struct FOutlawStatGrowthEntry
{
	GENERATED_BODY()

	/** The attribute to scale. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayAttribute Attribute;

	/** Value added per level (or per rank, for node bonuses). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ValuePerLevel = 0.0f;
};

/** A prerequisite link to another skill tree node. */
USTRUCT(BlueprintType)
struct FOutlawSkillNodePrerequisite
{
	GENERATED_BODY()

	/** Tag identifying the required node. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Skill"))
	FGameplayTag RequiredNodeTag;

	/** Minimum rank the required node must be at. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 RequiredRank = 1;
};

/** Runtime allocation state for a single skill node (replicated). */
USTRUCT(BlueprintType)
struct FOutlawAllocatedSkillNode
{
	GENERATED_BODY()

	/** Tag identifying the allocated node. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag NodeTag;

	/** Current allocated rank. */
	UPROPERTY(BlueprintReadOnly)
	int32 AllocatedRank = 0;
};

/** Serializable snapshot of the full progression state for save/load. */
USTRUCT(BlueprintType)
struct FOutlawProgressionSaveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentLevel = 1;

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentXP = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 AvailableSkillPoints = 0;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag SelectedClassTag;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag SelectedAscendancyTag;

	UPROPERTY(BlueprintReadWrite)
	TArray<FOutlawAllocatedSkillNode> AllocatedNodes;
};

// ── Delegates ───────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLeveledUp, int32, NewLevel, int32, SkillPointsAwarded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnXPChanged, int32, NewXP, int32, DeltaXP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillNodeAllocated, FGameplayTag, NodeTag, int32, NewRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillNodeDeallocated, FGameplayTag, NodeTag, int32, NewRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClassChanged, FGameplayTag, NewClassTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAscendancySelected, FGameplayTag, AscendancyTag);
