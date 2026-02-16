// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawDamageExecution.h"
#include "OutlawCombatTags.h"
#include "AbilitySystem/OutlawAttributeSet.h"
#include "AbilitySystem/OutlawWeaponAttributeSet.h"
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
		DEFINE_ATTRIBUTE_CAPTUREDEF(UOutlawWeaponAttributeSet, Firepower, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UOutlawWeaponAttributeSet, PhysicalDamageMin, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UOutlawWeaponAttributeSet, PhysicalDamageMax, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UOutlawWeaponAttributeSet, CritMultiplier, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UOutlawWeaponAttributeSet, CriticalStrikeChance, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UOutlawAttributeSet, Strength, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UOutlawAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UOutlawAttributeSet, IncomingDamage, Target, false);
	}
};

static const FDamageStatics& GetDamageStatics()
{
	static FDamageStatics DamageStatics;
	return DamageStatics;
}

UOutlawDamageExecution::UOutlawDamageExecution()
{
	RelevantAttributesToCapture.Add(GetDamageStatics().FirepowerDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().PhysicalDamageMinDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().PhysicalDamageMaxDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().CritMultiplierDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().CriticalStrikeChanceDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().StrengthDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().ArmorDef);
}

void UOutlawDamageExecution::Execute_Implementation(
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

	const float WeaponType = Spec.GetSetByCallerMagnitude(OutlawCombatTags::SetByCallerWeaponType, false, 0.f);
	const float TargetLevel = Spec.GetSetByCallerMagnitude(OutlawCombatTags::SetByCallerTargetLevel, false, 1.f);
	const float StrengthScaling = Spec.GetSetByCallerMagnitude(OutlawCombatTags::SetByCallerStrengthScaling, false, 0.5f);

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

		OutExecutionOutput.AddOutputTag(OutlawCombatTags::CriticalHit);
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
