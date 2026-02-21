// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "OutlawCombatTypes.generated.h"

class UOutlawAffixDefinition;

/**
 * Result data for damage application.
 * Contains final damage value, crit flag, and any applied tags.
 */
USTRUCT(BlueprintType)
struct FOutlawDamageResult
{
	GENERATED_BODY()

	/** Final damage value after all calculations. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float FinalDamage = 0.f;

	/** Was this a critical hit? */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bWasCritical = false;

	/** Additional tags applied during damage calculation (e.g. Combat.CriticalHit). */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayTagContainer AppliedTags;

	FOutlawDamageResult() = default;

	FOutlawDamageResult(float Damage, bool bCrit, const FGameplayTagContainer& Tags = FGameplayTagContainer())
		: FinalDamage(Damage), bWasCritical(bCrit), AppliedTags(Tags)
	{
	}
};

/**
 * Tracks a single active status effect on an actor.
 * Populated from ASC tag changes (Status.DoT, Status.CC.*, etc.).
 */
USTRUCT(BlueprintType)
struct FOutlawActiveStatusEffect
{
	GENERATED_BODY()

	/** Tag identifying the status effect (e.g. Status.DoT.Burn, Status.CC.Stun). */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayTag StatusTag;

	/** Stack count (if the effect allows stacking). */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 StackCount = 1;

	/** Time remaining in seconds (if durational, -1 if infinite). */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float TimeRemaining = -1.f;

	FOutlawActiveStatusEffect() = default;

	FOutlawActiveStatusEffect(const FGameplayTag& Tag, int32 Stacks, float Duration)
		: StatusTag(Tag), StackCount(Stacks), TimeRemaining(Duration)
	{
	}
};

/** Delegate fired when damage is dealt. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageDealt, AActor*, Target, float, DamageAmount, bool, bWasCritical);

/** Delegate fired when damage is received. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageReceived, AActor*, Source, float, DamageAmount, bool, bWasCritical);

/** Delegate fired when a status effect is added. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatusEffectAdded, FGameplayTag, StatusTag, int32, StackCount);

/** Delegate fired when a status effect is removed. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusEffectRemoved, FGameplayTag, StatusTag);
