// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AtomProgressionTypes.h"
#include "AtomSkillTreeNodeDefinition.generated.h"

class UAtomAbilitySet;

/**
 * Defines a single skill tree node — an allocable point on a class skill tree.
 * Supports single-rank nodes (grant one ability set) and multi-rank nodes
 * (e.g. +5% damage per rank, up to 5 ranks).
 */
UCLASS(BlueprintType, Const)
class OUTLAW_API UAtomSkillTreeNodeDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAtomSkillTreeNodeDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Unique identity tag (e.g. Skill.Devastator.GravityLeap). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node", meta = (Categories = "Skill"))
	FGameplayTag NodeTag;

	/** Display name shown in the skill tree UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node")
	FText DisplayName;

	/** Description shown in the skill tree tooltip. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node", meta = (MultiLine = true))
	FText Description;

	/** Icon for the skill tree UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node")
	TSoftObjectPtr<UTexture2D> Icon;

	/** How this node is unlocked. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node")
	EAtomSkillNodeUnlockType UnlockType = EAtomSkillNodeUnlockType::Manual;

	/** Maximum number of times this node can be allocated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node", meta = (ClampMin = "1"))
	int32 MaxRank = 1;

	/** Skill points consumed per rank allocation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node", meta = (ClampMin = "0"))
	int32 PointCostPerRank = 1;

	/** Minimum character level required to allocate this node. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node", meta = (ClampMin = "1"))
	int32 RequiredLevel = 1;

	/** If UnlockType == AutoOnLevel, auto-grant at this level. 0 = disabled. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node", meta = (ClampMin = "0", EditCondition = "UnlockType == EAtomSkillNodeUnlockType::AutoOnLevel"))
	int32 AutoUnlockLevel = 0;

	/** Other nodes that must be at a required rank before this node can be allocated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node")
	TArray<FAtomSkillNodePrerequisite> Prerequisites;

	/** Ability set granted at rank 1 (for single-rank nodes). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grants")
	TObjectPtr<UAtomAbilitySet> GrantedAbilitySet;

	/** Per-rank ability sets — index 0 = rank 1. Overrides GrantedAbilitySet if populated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grants")
	TArray<TObjectPtr<UAtomAbilitySet>> PerRankAbilitySets;

	/** Stat bonuses applied per rank (cumulative). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grants")
	TArray<FAtomStatGrowthEntry> StatBonusesPerRank;

	/** X position on the skill tree canvas (grid units, for UI layout). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node|Layout")
	float TreePositionX = 0.0f;

	/** Y position on the skill tree canvas (grid units, for UI layout). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node|Layout")
	float TreePositionY = 0.0f;

	/** Returns the ability set for the given rank (1-based). */
	UFUNCTION(BlueprintCallable, Category = "Node")
	UAtomAbilitySet* GetAbilitySetForRank(int32 Rank) const;
};
