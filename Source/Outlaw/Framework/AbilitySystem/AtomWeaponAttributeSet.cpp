// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawWeaponAttributeSet.h"
#include "Net/UnrealNetwork.h"

UOutlawWeaponAttributeSet::UOutlawWeaponAttributeSet()
{
}

void UOutlawWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Shooter attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UOutlawWeaponAttributeSet, Firepower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOutlawWeaponAttributeSet, RPM, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOutlawWeaponAttributeSet, Accuracy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOutlawWeaponAttributeSet, Stability, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOutlawWeaponAttributeSet, CritMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOutlawWeaponAttributeSet, WeaponRange, COND_None, REPNOTIFY_Always);

	// ARPG attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UOutlawWeaponAttributeSet, PhysicalDamageMin, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOutlawWeaponAttributeSet, PhysicalDamageMax, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOutlawWeaponAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOutlawWeaponAttributeSet, CriticalStrikeChance, COND_None, REPNOTIFY_Always);
}

// ── Rep Notifies ────────────────────────────────────────────────

void UOutlawWeaponAttributeSet::OnRep_Firepower(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOutlawWeaponAttributeSet, Firepower, OldValue);
}

void UOutlawWeaponAttributeSet::OnRep_RPM(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOutlawWeaponAttributeSet, RPM, OldValue);
}

void UOutlawWeaponAttributeSet::OnRep_Accuracy(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOutlawWeaponAttributeSet, Accuracy, OldValue);
}

void UOutlawWeaponAttributeSet::OnRep_Stability(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOutlawWeaponAttributeSet, Stability, OldValue);
}

void UOutlawWeaponAttributeSet::OnRep_CritMultiplier(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOutlawWeaponAttributeSet, CritMultiplier, OldValue);
}

void UOutlawWeaponAttributeSet::OnRep_WeaponRange(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOutlawWeaponAttributeSet, WeaponRange, OldValue);
}

void UOutlawWeaponAttributeSet::OnRep_PhysicalDamageMin(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOutlawWeaponAttributeSet, PhysicalDamageMin, OldValue);
}

void UOutlawWeaponAttributeSet::OnRep_PhysicalDamageMax(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOutlawWeaponAttributeSet, PhysicalDamageMax, OldValue);
}

void UOutlawWeaponAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOutlawWeaponAttributeSet, AttackSpeed, OldValue);
}

void UOutlawWeaponAttributeSet::OnRep_CriticalStrikeChance(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOutlawWeaponAttributeSet, CriticalStrikeChance, OldValue);
}
