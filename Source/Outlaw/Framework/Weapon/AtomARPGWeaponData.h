// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AtomWeaponTypes.h"
#include "AtomARPGWeaponData.generated.h"

class UAtomAbilitySet;
class UAtomAffixPoolDefinition;
class USkeletalMesh;

/**
 * Data asset defining an ARPG-style weapon's properties (Path of Exile 2-style).
 * Referenced from UAtomItemDefinition via composition.
 */
UCLASS(BlueprintType, Const)
class OUTLAW_API UAtomARPGWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAtomARPGWeaponData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Weapon archetype (Sword, Axe, Staff, etc.). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EAtomARPGWeaponType WeaponType = EAtomARPGWeaponType::Sword;

	// ── Core Stats ──────────────────────────────────────────────

	/** Minimum physical damage per hit. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float PhysicalDamageMin = 10.0f;

	/** Maximum physical damage per hit. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float PhysicalDamageMax = 20.0f;

	/** Attacks per second. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.1"))
	float AttackSpeed = 1.2f;

	/** Critical strike chance (0.0 to 1.0). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CriticalStrikeChance = 0.05f;

	// ── Sockets ─────────────────────────────────────────────────

	/** Maximum number of sockets this weapon can have. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sockets", meta = (ClampMin = "0", ClampMax = "6"))
	int32 MaxSocketCount = 3;

	/** Default socket layout (socket types and links). Copied to item instance on creation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sockets")
	TArray<FAtomSocketSlot> DefaultSocketLayout;

	/** Whether this weapon requires both hands (prevents off-hand). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	bool bTwoHanded = false;

	// ── Affixes ─────────────────────────────────────────────────

	/** Affix pool used when rolling random affixes on this weapon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Affixes")
	TObjectPtr<UAtomAffixPoolDefinition> AffixPool;

	// ── Abilities ───────────────────────────────────────────────

	/** Default attack ability set granted when this weapon is active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UAtomAbilitySet> DefaultAttackAbilitySet;

	// ── Visual ──────────────────────────────────────────────────

	/** Weapon skeletal mesh. Soft reference to avoid loading all meshes at once. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<USkeletalMesh> WeaponMesh;

	// ── Computed ─────────────────────────────────────────────────

	/**
	 * Compute the base DPS factoring in quality.
	 * Formula: ((Min+Max)/2) * AttackSpeed * (1 + Quality*0.01)
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float ComputeBaseDPS(int32 Quality) const;
};
