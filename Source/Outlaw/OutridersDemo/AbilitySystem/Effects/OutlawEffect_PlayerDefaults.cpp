#include "AbilitySystem/Effects/OutlawEffect_PlayerDefaults.h"
#include "AbilitySystem/AtomAttributeSet.h"

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

	AddOverride(UAtomAttributeSet::GetHealthAttribute(), 1000.f);
	AddOverride(UAtomAttributeSet::GetMaxHealthAttribute(), 1000.f);
	AddOverride(UAtomAttributeSet::GetStaminaAttribute(), 200.f);
	AddOverride(UAtomAttributeSet::GetMaxStaminaAttribute(), 200.f);
	AddOverride(UAtomAttributeSet::GetStrengthAttribute(), 50.f);
	AddOverride(UAtomAttributeSet::GetMaxStrengthAttribute(), 100.f);
	AddOverride(UAtomAttributeSet::GetArmorAttribute(), 50.f);
	AddOverride(UAtomAttributeSet::GetMaxArmorAttribute(), 100.f);
}
