// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/OutlawAbilityTypes.h"
#include "OutlawProgressionTypes.h"
#include "OutlawProgressionComponent.generated.h"

class UAbilitySystemComponent;
class UOutlawLevelingConfig;
class UOutlawClassDefinition;
class UOutlawSkillTreeNodeDefinition;

/**
 * Main progression system component — manages XP, leveling, class selection,
 * ascendancy, and skill tree allocation.
 *
 * Supports both Outriders-style fixed classes (auto-unlock + skill tree)
 * and PoE 2-style ascendancy classes (base class -> specialization with node prerequisites).
 *
 * Add via Blueprint to the character. Accesses the ASC via IAbilitySystemInterface,
 * matching the existing WeaponManagerComponent pattern.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UOutlawProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOutlawProgressionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ── Configuration ───────────────────────────────────────────

	/** Fallback XP table (used if the selected class has no LevelingConfig override). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression|Config")
	TObjectPtr<UOutlawLevelingConfig> DefaultLevelingConfig;

	/** Classes the player can choose from. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression|Config")
	TArray<TObjectPtr<UOutlawClassDefinition>> AvailableClasses;

	// ── Leveling API ────────────────────────────────────────────

	/** Awards XP, triggers level-ups, awards skill points, processes auto-unlock nodes. */
	UFUNCTION(BlueprintCallable, Category = "Progression|Leveling")
	void AwardXP(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Progression|Leveling")
	int32 GetCurrentLevel() const { return CurrentLevel; }

	UFUNCTION(BlueprintCallable, Category = "Progression|Leveling")
	int32 GetCurrentXP() const { return CurrentXP; }

	UFUNCTION(BlueprintCallable, Category = "Progression|Leveling")
	int32 GetAvailableSkillPoints() const { return AvailableSkillPoints; }

	/** Returns XP remaining until the next level. 0 if already at max level. */
	UFUNCTION(BlueprintCallable, Category = "Progression|Leveling")
	int32 GetXPToNextLevel() const;

	/** Returns 0.0-1.0 progress toward the next level (for UI bars). */
	UFUNCTION(BlueprintCallable, Category = "Progression|Leveling")
	float GetXPProgress() const;

	// ── Class API ───────────────────────────────────────────────

	/** Choose a base class. Revokes old class if re-selecting, grants ClassAbilitySet, recalculates stats. */
	UFUNCTION(BlueprintCallable, Category = "Progression|Class")
	void SelectClass(FGameplayTag ClassTag);

	/** Choose an ascendancy sub-class. Validates RequiredLevel and AvailableAscendancies. */
	UFUNCTION(BlueprintCallable, Category = "Progression|Class")
	void SelectAscendancy(FGameplayTag AscendancyTag);

	/** Returns the selected base class definition, or nullptr. */
	UFUNCTION(BlueprintCallable, Category = "Progression|Class")
	UOutlawClassDefinition* GetSelectedClass() const;

	/** Returns the selected ascendancy definition, or nullptr. */
	UFUNCTION(BlueprintCallable, Category = "Progression|Class")
	UOutlawClassDefinition* GetSelectedAscendancy() const;

	/** Returns ascendancy if selected, else base class. */
	UFUNCTION(BlueprintCallable, Category = "Progression|Class")
	UOutlawClassDefinition* GetActiveClassDefinition() const;

	// ── Skill Tree API ──────────────────────────────────────────

	/** Allocates one rank on a skill node. Validates prerequisites, level, and points. */
	UFUNCTION(BlueprintCallable, Category = "Progression|SkillTree")
	void AllocateSkillNode(FGameplayTag NodeTag);

	/** Deallocates one rank from a skill node. Only if no other nodes depend on this rank. */
	UFUNCTION(BlueprintCallable, Category = "Progression|SkillTree")
	void DeallocateSkillNode(FGameplayTag NodeTag);

	/** Deallocates everything, refunds all skill points. */
	UFUNCTION(BlueprintCallable, Category = "Progression|SkillTree")
	void RespecAllNodes();

	/** Returns the currently allocated rank for a node (0 if unallocated). */
	UFUNCTION(BlueprintCallable, Category = "Progression|SkillTree")
	int32 GetAllocatedRank(FGameplayTag NodeTag) const;

	/** Checks all requirements without allocating. */
	UFUNCTION(BlueprintCallable, Category = "Progression|SkillTree")
	bool CanAllocateNode(FGameplayTag NodeTag) const;

	/** Returns all nodes currently eligible for allocation. */
	UFUNCTION(BlueprintCallable, Category = "Progression|SkillTree")
	TArray<FGameplayTag> GetAllocableNodes() const;

	// ── Save/Load API ───────────────────────────────────────────

	/** Creates a serializable snapshot of the full progression state. */
	UFUNCTION(BlueprintCallable, Category = "Progression|SaveLoad")
	FOutlawProgressionSaveData SaveProgression() const;

	/** Restores progression state from saved data. Recalculates everything. */
	UFUNCTION(BlueprintCallable, Category = "Progression|SaveLoad")
	void LoadProgression(const FOutlawProgressionSaveData& Data);

	// ── Delegates ───────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnPlayerLeveledUp OnPlayerLeveledUp;

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnXPChanged OnXPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnSkillNodeAllocated OnSkillNodeAllocated;

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnSkillNodeDeallocated OnSkillNodeDeallocated;

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnClassChanged OnClassChanged;

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnAscendancySelected OnAscendancySelected;

private:
	// ── Helpers ──────────────────────────────────────────────────

	/** Resolve the ASC from the owning actor (same pattern as WeaponManagerComponent). */
	UAbilitySystemComponent* GetASC() const;

	/** Returns the class's leveling config override, or DefaultLevelingConfig. */
	UOutlawLevelingConfig* GetEffectiveLevelingConfig() const;

	/** Recalculates attribute base values from class stat growth + allocated node bonuses. */
	void RecalculateStatBases();

	/** Scans skill tree for AutoOnLevel nodes <= CurrentLevel and auto-allocates them. */
	void ProcessAutoUnlockNodes();

	/** Grants ability set for a node at the given rank. */
	void GrantNodeAbilities(FGameplayTag NodeTag, int32 Rank);

	/** Revokes currently granted abilities for a node. */
	void RevokeNodeAbilities(FGameplayTag NodeTag);

	/** Finds the node definition across the active class and ascendancy skill trees. */
	UOutlawSkillTreeNodeDefinition* FindNodeDefinition(FGameplayTag NodeTag) const;

	/** Returns mutable reference to the allocated node entry, creating one if needed. */
	FOutlawAllocatedSkillNode& FindOrAddAllocatedNode(FGameplayTag NodeTag);

	/** Checks if any other allocated node depends on the given node at its current rank. */
	bool IsNodeRequiredByOthers(FGameplayTag NodeTag, int32 AtRank) const;

	// ── Replicated State ────────────────────────────────────────

	UPROPERTY(Replicated)
	int32 CurrentLevel = 1;

	UPROPERTY(Replicated)
	int32 CurrentXP = 0;

	UPROPERTY(Replicated)
	int32 AvailableSkillPoints = 0;

	UPROPERTY(Replicated)
	FGameplayTag SelectedClassTag;

	UPROPERTY(Replicated)
	FGameplayTag SelectedAscendancyTag;

	UPROPERTY(Replicated)
	TArray<FOutlawAllocatedSkillNode> AllocatedNodes;

	// ── Server-Only Handles ─────────────────────────────────────

	/** Handles for the base class ability set. */
	FOutlawAbilitySetGrantedHandles ClassAbilityHandles;

	/** Handles for the ascendancy ability set. */
	FOutlawAbilitySetGrantedHandles AscendancyAbilityHandles;

	/** Per-node granted ability handles. */
	TMap<FGameplayTag, FOutlawAbilitySetGrantedHandles> NodeAbilityHandles;
};
