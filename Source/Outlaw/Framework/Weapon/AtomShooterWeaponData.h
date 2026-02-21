// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AtomWeaponTypes.h"
#include "AtomShooterWeaponData.generated.h"

class UAtomAbilitySet;
class USkeletalMesh;
class UAnimMontage;

/**
 * Data asset defining a shooter-style weapon's properties (Outriders-style).
 * Referenced from UAtomItemDefinition via composition.
 */
UCLASS(BlueprintType, Const)
class OUTLAW_API UAtomShooterWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAtomShooterWeaponData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Weapon archetype (AssaultRifle, Shotgun, etc.). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EAtomShooterWeaponType WeaponType = EAtomShooterWeaponType::AssaultRifle;

	// ── Fire Mode ──────────────────────────────────────────────

	/** If true, weapon fires continuously while trigger is held. If false, fires once per click (or burst). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Firing")
	bool bAutomatic = true;

	/** Number of shots per trigger pull in semi-auto mode. 1 = single shot, >1 = burst. Ignored when bAutomatic is true. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Firing", meta = (EditCondition = "!bAutomatic", ClampMin = "1"))
	int32 BurstCount = 1;

	// ── Core Stats ──────────────────────────────────────────────

	/** Base damage per bullet. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float Firepower = 100.0f;

	/** Rounds per minute. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0"))
	float RPM = 600.0f;

	/** Time in seconds to reload. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.1"))
	float ReloadTime = 2.0f;

	/** Effective range in centimeters. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float Range = 3000.0f;

	/** Accuracy value (0.0 = worst, 100.0 = perfect). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Accuracy = 80.0f;

	/** Stability / recoil control (0.0 = worst, 100.0 = no recoil). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Stability = 70.0f;

	/** Critical hit damage multiplier. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0"))
	float CritMultiplier = 1.5f;

	/** Magazine capacity. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1"))
	int32 MagazineSize = 30;

	/** Ammo type tag — determines which reserve ammo pool is consumed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (Categories = "Ammo"))
	FGameplayTag AmmoTypeTag;

	// ── Visual ──────────────────────────────────────────────────

	/** Weapon skeletal mesh. Soft reference to avoid loading all meshes at once. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<USkeletalMesh> WeaponMesh;

	/** Reload animation montage. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<UAnimMontage> ReloadMontage;

	// ── Abilities ───────────────────────────────────────────────

	/** Ability set granted when this weapon fires (includes the fire ability). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UAtomAbilitySet> FireAbilitySet;

	/** Ability set granted for reload behavior. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UAtomAbilitySet> ReloadAbilitySet;
};
