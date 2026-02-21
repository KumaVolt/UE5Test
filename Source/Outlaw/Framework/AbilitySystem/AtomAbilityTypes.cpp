// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawAbilityTypes.h"
#include "AbilitySystemComponent.h"

void FOutlawAbilitySetGrantedHandles::RevokeFromASC(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			ASC->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : EffectHandles)
	{
		if (Handle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}
	}

	for (TObjectPtr<UAttributeSet> AttribSet : AttributeSets)
	{
		if (AttribSet)
		{
			ASC->RemoveSpawnedAttribute(AttribSet);
		}
	}

	AbilitySpecHandles.Reset();
	EffectHandles.Reset();
	AttributeSets.Reset();
}
