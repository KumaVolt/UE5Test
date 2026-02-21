// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AtomDeathTypes.generated.h"

// ── Death Delegates ────────────────────────────────────────────

/** Fired when death begins (Health reaches 0). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathStarted, AActor*, Killer);

/** Fired when death finishes (all death animations/sequences complete). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathFinished, AActor*, DeadActor);

/** Fired when loot drop is requested (enemy death Phase 3). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLootDropRequested, FVector, Location, int32, EnemyLevel);

/** Fired when XP is awarded (enemy death Phase 3). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnXPAwarded, int32, XPAmount);

// ── Combat Log Entry ───────────────────────────────────────────

/**
 * Single entry in the combat log circular buffer.
 * Tracks damage events, kills, source/target info, and ability tags.
 */
USTRUCT(BlueprintType)
struct FAtomCombatLogEntry
{
	GENERATED_BODY()

	/** Timestamp of the event. */
	UPROPERTY(BlueprintReadOnly, Category = "CombatLog")
	FDateTime Timestamp;

	/** Display name of the damage source. */
	UPROPERTY(BlueprintReadOnly, Category = "CombatLog")
	FString SourceName;

	/** Display name of the damage target. */
	UPROPERTY(BlueprintReadOnly, Category = "CombatLog")
	FString TargetName;

	/** Amount of damage dealt. */
	UPROPERTY(BlueprintReadOnly, Category = "CombatLog")
	float DamageAmount = 0.f;

	/** Whether this damage resulted in a kill. */
	UPROPERTY(BlueprintReadOnly, Category = "CombatLog")
	bool bKilled = false;

	/** Ability tag used (if any). */
	UPROPERTY(BlueprintReadOnly, Category = "CombatLog")
	FGameplayTag AbilityTag;

	FAtomCombatLogEntry()
		: Timestamp(FDateTime::Now())
		, DamageAmount(0.f)
		, bKilled(false)
	{}
};

/** Delegate for combat log entry additions (for UI binding). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatLogEntryAdded, const FAtomCombatLogEntry&, Entry);
