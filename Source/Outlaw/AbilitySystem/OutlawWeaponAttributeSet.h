// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "OutlawWeaponAttributeSet.generated.h"

/**
 * Weapon stats exposed as GAS attributes so abilities and effects can read/modify them.
 * Populated by the weapon manager when a weapon becomes active.
 * Follows the same pattern as UOutlawAttributeSet.
 */
UCLASS()
class OUTLAW_API UOutlawWeaponAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UOutlawWeaponAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// ── Shooter Attributes ──────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Firepower, Category = "Weapon|Shooter")
	FGameplayAttributeData Firepower;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawWeaponAttributeSet, Firepower);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RPM, Category = "Weapon|Shooter")
	FGameplayAttributeData RPM;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawWeaponAttributeSet, RPM);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Accuracy, Category = "Weapon|Shooter")
	FGameplayAttributeData Accuracy;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawWeaponAttributeSet, Accuracy);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stability, Category = "Weapon|Shooter")
	FGameplayAttributeData Stability;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawWeaponAttributeSet, Stability);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CritMultiplier, Category = "Weapon|Shooter")
	FGameplayAttributeData CritMultiplier;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawWeaponAttributeSet, CritMultiplier);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WeaponRange, Category = "Weapon|Shooter")
	FGameplayAttributeData WeaponRange;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawWeaponAttributeSet, WeaponRange);

	// ── ARPG Attributes ─────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PhysicalDamageMin, Category = "Weapon|ARPG")
	FGameplayAttributeData PhysicalDamageMin;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawWeaponAttributeSet, PhysicalDamageMin);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PhysicalDamageMax, Category = "Weapon|ARPG")
	FGameplayAttributeData PhysicalDamageMax;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawWeaponAttributeSet, PhysicalDamageMax);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackSpeed, Category = "Weapon|ARPG")
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawWeaponAttributeSet, AttackSpeed);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalStrikeChance, Category = "Weapon|ARPG")
	FGameplayAttributeData CriticalStrikeChance;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawWeaponAttributeSet, CriticalStrikeChance);

	// ── Rep Notifies ────────────────────────────────────────────

	UFUNCTION()
	void OnRep_Firepower(const FGameplayAttributeData& OldValue) const;
	UFUNCTION()
	void OnRep_RPM(const FGameplayAttributeData& OldValue) const;
	UFUNCTION()
	void OnRep_Accuracy(const FGameplayAttributeData& OldValue) const;
	UFUNCTION()
	void OnRep_Stability(const FGameplayAttributeData& OldValue) const;
	UFUNCTION()
	void OnRep_CritMultiplier(const FGameplayAttributeData& OldValue) const;
	UFUNCTION()
	void OnRep_WeaponRange(const FGameplayAttributeData& OldValue) const;
	UFUNCTION()
	void OnRep_PhysicalDamageMin(const FGameplayAttributeData& OldValue) const;
	UFUNCTION()
	void OnRep_PhysicalDamageMax(const FGameplayAttributeData& OldValue) const;
	UFUNCTION()
	void OnRep_AttackSpeed(const FGameplayAttributeData& OldValue) const;
	UFUNCTION()
	void OnRep_CriticalStrikeChance(const FGameplayAttributeData& OldValue) const;
};
