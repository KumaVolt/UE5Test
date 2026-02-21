#include "AbilitySystem/Effects/OutlawEffect_DefaultAttributes.h"
#include "AbilitySystem/AtomAttributeSet.h"

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

	AddOverride(UAtomAttributeSet::GetHealthAttribute(), 500.f);
	AddOverride(UAtomAttributeSet::GetMaxHealthAttribute(), 500.f);
	AddOverride(UAtomAttributeSet::GetStaminaAttribute(), 100.f);
	AddOverride(UAtomAttributeSet::GetMaxStaminaAttribute(), 100.f);
	AddOverride(UAtomAttributeSet::GetStrengthAttribute(), 20.f);
	AddOverride(UAtomAttributeSet::GetMaxStrengthAttribute(), 100.f);
	AddOverride(UAtomAttributeSet::GetArmorAttribute(), 30.f);
	AddOverride(UAtomAttributeSet::GetMaxArmorAttribute(), 100.f);
}
