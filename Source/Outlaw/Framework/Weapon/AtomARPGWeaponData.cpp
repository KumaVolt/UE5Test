// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomARPGWeaponData.h"

UAtomARPGWeaponData::UAtomARPGWeaponData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

float UAtomARPGWeaponData::ComputeBaseDPS(int32 Quality) const
{
	const float AverageDamage = (PhysicalDamageMin + PhysicalDamageMax) / 2.0f;
	const float QualityMultiplier = 1.0f + FMath::Clamp(Quality, 0, 20) * 0.01f;
	return AverageDamage * AttackSpeed * QualityMultiplier;
}
