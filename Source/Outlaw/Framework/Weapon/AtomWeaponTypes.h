// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AtomWeaponTypes.generated.h"

class UAtomAffixDefinition;
class UAtomSkillGemDefinition;

// ────────────────────────────────────────────────────────────────
// Shooter Weapon Types (Outriders-style)
// ────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAtomShooterWeaponType : uint8
{
	AssaultRifle = 0,
	Shotgun,
	SMG,
	LMG,
	SniperRifle,
	Pistol,
	Revolver
};

// ────────────────────────────────────────────────────────────────
// ARPG Weapon Types (Path of Exile 2-style)
// ────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAtomARPGWeaponType : uint8
{
	Sword = 0,
	Axe,
	Mace,
	Dagger,
	Staff,
	Wand,
	Bow,
	Crossbow,
	Spear
};

// ────────────────────────────────────────────────────────────────
// Affix Slot
// ────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAtomAffixSlot : uint8
{
	Prefix = 0,
	Suffix
};

// ────────────────────────────────────────────────────────────────
// FAtomItemAffix — A rolled affix on an item instance
// ────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAtomItemAffix
{
	GENERATED_BODY()

	FAtomItemAffix()
		: AffixDef(nullptr)
		, RolledValue(0.0f)
		, Slot(EAtomAffixSlot::Prefix)
	{
	}

	/** The affix definition that was rolled. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	TObjectPtr<UAtomAffixDefinition> AffixDef;

	/** The rolled value within the affix's min/max range. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	float RolledValue;

	/** Whether this affix occupies a prefix or suffix slot. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	EAtomAffixSlot Slot;
};

// ────────────────────────────────────────────────────────────────
// FAtomSocketSlot — A single gem socket on an item instance
// ────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAtomSocketSlot
{
	GENERATED_BODY()

	FAtomSocketSlot()
		: SocketedGem(nullptr)
		, bLinkedToNext(false)
	{
	}

	/** The gem currently socketed, nullptr if empty. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Socket")
	TObjectPtr<UAtomSkillGemDefinition> SocketedGem;

	/** Socket color / type compatibility tag (e.g. Socket.Red, Socket.Blue). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Socket")
	FGameplayTag SocketTypeTag;

	/** Whether this socket is linked to the next socket in the array. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Socket")
	bool bLinkedToNext;
};
