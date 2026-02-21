// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Namespace for Combat Gameplay Tags.
 * Register these tags in your project's DefaultGameplayTags.ini or via data asset.
 */
namespace AtomCombatTags
{
	// Combat.CriticalHit — added to spec on critical strike
	inline const FGameplayTag CriticalHit = FGameplayTag::RequestGameplayTag(TEXT("Combat.CriticalHit"));

	// Status.DoT — base tag for damage-over-time effects
	inline const FGameplayTag StatusDoT = FGameplayTag::RequestGameplayTag(TEXT("Status.DoT"));

	// Status.CC.Stun — stun crowd control
	inline const FGameplayTag StatusCCStun = FGameplayTag::RequestGameplayTag(TEXT("Status.CC.Stun"));

	// Status.CC.Slow — slow crowd control
	inline const FGameplayTag StatusCCSlow = FGameplayTag::RequestGameplayTag(TEXT("Status.CC.Slow"));

	// Status.CC.Root — root crowd control
	inline const FGameplayTag StatusCCRoot = FGameplayTag::RequestGameplayTag(TEXT("Status.CC.Root"));

	// SetByCaller.WeaponType — 1.0 = shooter path, else ARPG path
	inline const FGameplayTag SetByCallerWeaponType = FGameplayTag::RequestGameplayTag(TEXT("SetByCaller.WeaponType"));

	// SetByCaller.TargetLevel — target level for armor mitigation
	inline const FGameplayTag SetByCallerTargetLevel = FGameplayTag::RequestGameplayTag(TEXT("SetByCaller.TargetLevel"));

	// SetByCaller.StrengthScaling — strength attribute scaling factor
	inline const FGameplayTag SetByCallerStrengthScaling = FGameplayTag::RequestGameplayTag(TEXT("SetByCaller.StrengthScaling"));
}
