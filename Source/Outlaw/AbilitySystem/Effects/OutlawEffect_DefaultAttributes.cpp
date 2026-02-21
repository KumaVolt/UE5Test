#include "AbilitySystem/Effects/OutlawEffect_DefaultAttributes.h"
#include "AbilitySystem/OutlawAttributeSet.h"

UOutlawEffect_DefaultAttributes::UOutlawEffect_DefaultAttributes()
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

	AddOverride(UOutlawAttributeSet::GetHealthAttribute(), 500.f);
	AddOverride(UOutlawAttributeSet::GetMaxHealthAttribute(), 500.f);
	AddOverride(UOutlawAttributeSet::GetStaminaAttribute(), 100.f);
	AddOverride(UOutlawAttributeSet::GetMaxStaminaAttribute(), 100.f);
	AddOverride(UOutlawAttributeSet::GetStrengthAttribute(), 20.f);
	AddOverride(UOutlawAttributeSet::GetMaxStrengthAttribute(), 100.f);
	AddOverride(UOutlawAttributeSet::GetArmorAttribute(), 30.f);
	AddOverride(UOutlawAttributeSet::GetMaxArmorAttribute(), 100.f);
}
