// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomGameplayAbility.h"
#include "AbilitySystemComponent.h"

UAtomGameplayAbility::UAtomGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Default: abilities require input
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAtomGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	TryActivateOnGranted(ActorInfo, Spec);
}

void UAtomGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAtomGameplayAbility::TryActivateOnGranted(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (ActivationPolicy != EAtomAbilityActivationPolicy::OnGranted)
	{
		return;
	}

	if (ActorInfo && !Spec.IsActive())
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		if (ASC)
		{
			ASC->TryActivateAbility(Spec.Handle);
		}
	}
}
