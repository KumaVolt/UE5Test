// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/OutlawPlayerState.h"
#include "AbilitySystem/OutlawAbilitySystemComponent.h"

AOutlawPlayerState::AOutlawPlayerState()
{
	SetNetUpdateFrequency(100.f);
	AbilitySystemComponent = CreateDefaultSubobject<UOutlawAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

UAbilitySystemComponent* AOutlawPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
