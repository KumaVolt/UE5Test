// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawCharacterBase.h"
#include "AbilitySystem/OutlawAbilitySystemComponent.h"
#include "AbilitySystem/OutlawAbilitySet.h"

AOutlawCharacterBase::AOutlawCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

UAbilitySystemComponent* AOutlawCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AOutlawCharacterBase::GrantDefaultAbilitySet()
{
	if (!AbilitySystemComponent || !DefaultAbilitySet)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	DefaultAbilitySetHandles = AbilitySystemComponent->GrantAbilitySet(DefaultAbilitySet, this);
}

void AOutlawCharacterBase::RevokeDefaultAbilitySet()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->RevokeAbilitySet(DefaultAbilitySetHandles);
}
