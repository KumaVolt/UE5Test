// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomCharacterBase.h"
#include "AbilitySystem/AtomAbilitySystemComponent.h"
#include "AbilitySystem/AtomAbilitySet.h"

AAtomCharacterBase::AAtomCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

UAbilitySystemComponent* AAtomCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAtomCharacterBase::GrantDefaultAbilitySet()
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

void AAtomCharacterBase::RevokeDefaultAbilitySet()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->RevokeAbilitySet(DefaultAbilitySetHandles);
}
