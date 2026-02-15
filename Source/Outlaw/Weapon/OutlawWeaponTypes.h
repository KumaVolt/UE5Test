// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "OutlawWeaponTypes.generated.h"

class UOutlawAffixDefinition;
class UOutlawSkillGemDefinition;

// ────────────────────────────────────────────────────────────────
// Shooter Weapon Types (Outriders-style)
// ────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EOutlawShooterWeaponType : uint8
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
enum class EOutlawARPGWeaponType : uint8
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
enum class EOutlawAffixSlot : uint8
{
	Prefix = 0,
	Suffix
};

// ────────────────────────────────────────────────────────────────
// FOutlawItemAffix — A rolled affix on an item instance
// ────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FOutlawItemAffix
{
	GENERATED_BODY()

	FOutlawItemAffix()
		: AffixDef(nullptr)
		, RolledValue(0.0f)
		, Slot(EOutlawAffixSlot::Prefix)
	{
	}

	/** The affix definition that was rolled. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	TObjectPtr<UOutlawAffixDefinition> AffixDef;

	/** The rolled value within the affix's min/max range. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	float RolledValue;

	/** Whether this affix occupies a prefix or suffix slot. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	EOutlawAffixSlot Slot;
};

// ────────────────────────────────────────────────────────────────
// FOutlawSocketSlot — A single gem socket on an item instance
// ────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FOutlawSocketSlot
{
	GENERATED_BODY()

	FOutlawSocketSlot()
		: SocketedGem(nullptr)
		, bLinkedToNext(false)
	{
	}

	/** The gem currently socketed, nullptr if empty. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Socket")
	TObjectPtr<UOutlawSkillGemDefinition> SocketedGem;

	/** Socket color / type compatibility tag (e.g. Socket.Red, Socket.Blue). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Socket")
	FGameplayTag SocketTypeTag;

	/** Whether this socket is linked to the next socket in the array. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Socket")
	bool bLinkedToNext;
};
