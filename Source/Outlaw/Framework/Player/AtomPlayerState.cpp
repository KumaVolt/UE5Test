// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AtomPlayerState.h"
#include "AbilitySystem/AtomAbilitySystemComponent.h"

AAtomPlayerState::AAtomPlayerState()
{
	SetNetUpdateFrequency(100.f);
	AbilitySystemComponent = CreateDefaultSubobject<UAtomAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

UAbilitySystemComponent* AAtomPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
