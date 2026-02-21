#include "Game/OutlawDemoAbilitySets.h"
#include "AbilitySystem/OutlawAbilityTypes.h"
#include "AbilitySystem/OutlawAttributeSet.h"
#include "AbilitySystem/OutlawWeaponAttributeSet.h"
#include "AbilitySystem/Abilities/OutlawAbility_FireWeapon.h"
#include "AbilitySystem/Abilities/OutlawAbility_MeleeAttack.h"
#include "AbilitySystem/Effects/OutlawEffect_PlayerDefaults.h"
#include "AbilitySystem/Effects/OutlawEffect_DefaultAttributes.h"

// ── Player Default Ability Set ─────────────────────────────────

UOutlawAbilitySet_PlayerDefault::UOutlawAbilitySet_PlayerDefault()
{
	// Fire weapon ability bound to Input.Ability.Fire
	FOutlawAbilityBindInfo FireInfo;
	FireInfo.AbilityClass = UOutlawAbility_FireWeapon::StaticClass();
	FireInfo.AbilityLevel = 1;
	FireInfo.InputTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Ability.Fire"));
	Abilities.Add(FireInfo);

	// Player default attributes effect
	FOutlawGrantedEffect AttrEffect;
	AttrEffect.EffectClass = UOutlawEffect_PlayerDefaults::StaticClass();
	AttrEffect.EffectLevel = 1;
	Effects.Add(AttrEffect);

	// Core attribute set
	FOutlawGrantedAttributeSet CoreAttrs;
	CoreAttrs.AttributeSetClass = UOutlawAttributeSet::StaticClass();
	AttributeSets.Add(CoreAttrs);

	// Weapon attribute set (for fire weapon ability to read from)
	FOutlawGrantedAttributeSet WeaponAttrs;
	WeaponAttrs.AttributeSetClass = UOutlawWeaponAttributeSet::StaticClass();
	AttributeSets.Add(WeaponAttrs);
}

// ── Enemy Default Ability Set ──────────────────────────────────

UOutlawAbilitySet_EnemyDefault::UOutlawAbilitySet_EnemyDefault()
{
	// Melee attack ability bound to Input.Ability.MeleeAttack
	FOutlawAbilityBindInfo MeleeInfo;
	MeleeInfo.AbilityClass = UOutlawAbility_MeleeAttack::StaticClass();
	MeleeInfo.AbilityLevel = 1;
	MeleeInfo.InputTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Ability.MeleeAttack"));
	Abilities.Add(MeleeInfo);

	// Enemy default attributes effect
	FOutlawGrantedEffect AttrEffect;
	AttrEffect.EffectClass = UOutlawEffect_DefaultAttributes::StaticClass();
	AttrEffect.EffectLevel = 1;
	Effects.Add(AttrEffect);

	// Core attribute set
	FOutlawGrantedAttributeSet CoreAttrs;
	CoreAttrs.AttributeSetClass = UOutlawAttributeSet::StaticClass();
	AttributeSets.Add(CoreAttrs);
}
