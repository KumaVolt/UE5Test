// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomSTCondition_HealthThreshold.h"
#include "StateTreeExecutionContext.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AtomAttributeSet.h"

bool FAtomSTCondition_HealthThreshold::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.OwnerActor)
	{
		return false;
	}

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(InstanceData.OwnerActor);
	if (!ASI)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	const UAtomAttributeSet* AttributeSet = ASC->GetSet<UAtomAttributeSet>();
	if (!AttributeSet)
	{
		return false;
	}

	const float CurrentHealth = AttributeSet->GetHealth();
	const float MaxHealth = AttributeSet->GetMaxHealth();

	if (MaxHealth <= 0.f)
	{
		return false;
	}

	const float HealthPercent = (CurrentHealth / MaxHealth) * 100.f;
	return HealthPercent <= InstanceData.ThresholdPercent;
}
