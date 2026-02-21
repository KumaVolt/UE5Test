// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomWeaponAttributeSet.h"
#include "Net/UnrealNetwork.h"

UAtomWeaponAttributeSet::UAtomWeaponAttributeSet()
{
}

void UAtomWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Shooter attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UAtomWeaponAttributeSet, Firepower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtomWeaponAttributeSet, RPM, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtomWeaponAttributeSet, Accuracy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtomWeaponAttributeSet, Stability, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtomWeaponAttributeSet, CritMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtomWeaponAttributeSet, WeaponRange, COND_None, REPNOTIFY_Always);

	// ARPG attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UAtomWeaponAttributeSet, PhysicalDamageMin, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtomWeaponAttributeSet, PhysicalDamageMax, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtomWeaponAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtomWeaponAttributeSet, CriticalStrikeChance, COND_None, REPNOTIFY_Always);
}

// ── Rep Notifies ────────────────────────────────────────────────

void UAtomWeaponAttributeSet::OnRep_Firepower(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtomWeaponAttributeSet, Firepower, OldValue);
}

void UAtomWeaponAttributeSet::OnRep_RPM(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtomWeaponAttributeSet, RPM, OldValue);
}

void UAtomWeaponAttributeSet::OnRep_Accuracy(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtomWeaponAttributeSet, Accuracy, OldValue);
}

void UAtomWeaponAttributeSet::OnRep_Stability(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtomWeaponAttributeSet, Stability, OldValue);
}

void UAtomWeaponAttributeSet::OnRep_CritMultiplier(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtomWeaponAttributeSet, CritMultiplier, OldValue);
}

void UAtomWeaponAttributeSet::OnRep_WeaponRange(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtomWeaponAttributeSet, WeaponRange, OldValue);
}

void UAtomWeaponAttributeSet::OnRep_PhysicalDamageMin(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtomWeaponAttributeSet, PhysicalDamageMin, OldValue);
}

void UAtomWeaponAttributeSet::OnRep_PhysicalDamageMax(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtomWeaponAttributeSet, PhysicalDamageMax, OldValue);
}

void UAtomWeaponAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtomWeaponAttributeSet, AttackSpeed, OldValue);
}

void UAtomWeaponAttributeSet::OnRep_CriticalStrikeChance(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtomWeaponAttributeSet, CriticalStrikeChance, OldValue);
}
