// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomDamageExecution.h"
#include "AtomCombatTags.h"
#include "AbilitySystem/AtomAttributeSet.h"
#include "AbilitySystem/AtomWeaponAttributeSet.h"
#include "GameplayEffectTypes.h"

struct FDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Firepower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalDamageMin);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalDamageMax);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritMultiplier);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalStrikeChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Strength);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(IncomingDamage);

	FDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAtomWeaponAttributeSet, Firepower, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAtomWeaponAttributeSet, PhysicalDamageMin, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAtomWeaponAttributeSet, PhysicalDamageMax, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAtomWeaponAttributeSet, CritMultiplier, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAtomWeaponAttributeSet, CriticalStrikeChance, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAtomAttributeSet, Strength, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAtomAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAtomAttributeSet, IncomingDamage, Target, false);
	}
};

static const FDamageStatics& GetDamageStatics()
{
	static FDamageStatics DamageStatics;
	return DamageStatics;
}

UAtomDamageExecution::UAtomDamageExecution()
{
	RelevantAttributesToCapture.Add(GetDamageStatics().FirepowerDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().PhysicalDamageMinDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().PhysicalDamageMaxDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().CritMultiplierDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().CriticalStrikeChanceDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().StrengthDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().ArmorDef);
}

void UAtomDamageExecution::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float Firepower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetDamageStatics().FirepowerDef, EvaluationParameters, Firepower);

	float PhysicalDamageMin = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetDamageStatics().PhysicalDamageMinDef, EvaluationParameters, PhysicalDamageMin);

	float PhysicalDamageMax = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetDamageStatics().PhysicalDamageMaxDef, EvaluationParameters, PhysicalDamageMax);

	float CritMultiplier = 1.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetDamageStatics().CritMultiplierDef, EvaluationParameters, CritMultiplier);

	float CriticalStrikeChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetDamageStatics().CriticalStrikeChanceDef, EvaluationParameters, CriticalStrikeChance);

	float Strength = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetDamageStatics().StrengthDef, EvaluationParameters, Strength);

	float Armor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetDamageStatics().ArmorDef, EvaluationParameters, Armor);

	const float WeaponType = Spec.GetSetByCallerMagnitude(AtomCombatTags::SetByCallerWeaponType, false, 0.f);
	const float TargetLevel = Spec.GetSetByCallerMagnitude(AtomCombatTags::SetByCallerTargetLevel, false, 1.f);
	const float StrengthScaling = Spec.GetSetByCallerMagnitude(AtomCombatTags::SetByCallerStrengthScaling, false, 0.5f);

	constexpr float ArmorConstantBase = 50.f;
	constexpr float ArmorConstantPerLevel = 10.f;

	float BaseDamage = 0.f;
	if (FMath::IsNearlyEqual(WeaponType, 1.f, 0.01f))
	{
		BaseDamage = Firepower + (Strength * StrengthScaling);
	}
	else
	{
		BaseDamage = FMath::RandRange(PhysicalDamageMin, PhysicalDamageMax) + (Strength * StrengthScaling);
	}

	bool bWasCritical = false;
	if (FMath::FRand() < CriticalStrikeChance)
	{
		BaseDamage *= CritMultiplier;
		bWasCritical = true;
	}

	const float K = ArmorConstantBase + (ArmorConstantPerLevel * TargetLevel);
	const float ArmorFactor = Armor / (Armor + K);
	const float FinalDamage = FMath::Max(BaseDamage * (1.0f - ArmorFactor), 0.f);

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		GetDamageStatics().IncomingDamageDef.AttributeToCapture,
		EGameplayModOp::Additive,
		FinalDamage
	));
}
