#include "AbilitySystem/Effects/OutlawEffect_PlayerDefaults.h"
#include "AbilitySystem/OutlawAttributeSet.h"

UOutlawEffect_PlayerDefaults::UOutlawEffect_PlayerDefaults()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	auto AddOverride = [this](const FGameplayAttribute& Attribute, float Value)
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = Attribute;
		Mod.ModifierOp = EGameplayModOp::Override;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Value));
		Modifiers.Add(Mod);
	};

	AddOverride(UOutlawAttributeSet::GetHealthAttribute(), 1000.f);
	AddOverride(UOutlawAttributeSet::GetMaxHealthAttribute(), 1000.f);
	AddOverride(UOutlawAttributeSet::GetStaminaAttribute(), 200.f);
	AddOverride(UOutlawAttributeSet::GetMaxStaminaAttribute(), 200.f);
	AddOverride(UOutlawAttributeSet::GetStrengthAttribute(), 50.f);
	AddOverride(UOutlawAttributeSet::GetMaxStrengthAttribute(), 100.f);
	AddOverride(UOutlawAttributeSet::GetArmorAttribute(), 50.f);
	AddOverride(UOutlawAttributeSet::GetMaxArmorAttribute(), 100.f);
}
