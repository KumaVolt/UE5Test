// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawGameplayAbility.h"
#include "AbilitySystemComponent.h"

UOutlawGameplayAbility::UOutlawGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Default: abilities require input
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UOutlawGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	TryActivateOnGranted(ActorInfo, Spec);
}

void UOutlawGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UOutlawGameplayAbility::TryActivateOnGranted(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (ActivationPolicy != EOutlawAbilityActivationPolicy::OnGranted)
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
