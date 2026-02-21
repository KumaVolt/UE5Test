// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomStatusEffectComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

UAtomStatusEffectComponent::UAtomStatusEffectComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAtomStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UAbilitySystemComponent* ASC = nullptr;
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		ASC = ASI->GetAbilitySystemComponent();
	}

	if (!ASC && Owner->GetInstigator())
	{
		AActor* Instigator = Owner->GetInstigator();
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Instigator))
		{
			ASC = ASI->GetAbilitySystemComponent();
		}
	}

	if (ASC)
	{
		BoundASC = ASC;
		FGameplayTag StatusRootTag = FGameplayTag::RequestGameplayTag(FName("Status"));
		ASC->RegisterGameplayTagEvent(StatusRootTag, EGameplayTagEventType::NewOrRemoved).AddUObject(
			this, &UAtomStatusEffectComponent::OnStatusTagChanged);
	}
}

void UAtomStatusEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	BoundASC.Reset();
	ActiveEffects.Empty();

	Super::EndPlay(EndPlayReason);
}

TArray<FAtomActiveStatusEffect> UAtomStatusEffectComponent::GetActiveStatusEffects() const
{
	return ActiveEffects;
}

bool UAtomStatusEffectComponent::HasStatusEffect(FGameplayTag StatusTag) const
{
	return ActiveEffects.ContainsByPredicate([&StatusTag](const FAtomActiveStatusEffect& Effect)
	{
		return Effect.StatusTag.MatchesTagExact(StatusTag);
	});
}

void UAtomStatusEffectComponent::OnStatusTagChanged(const FGameplayTag StatusTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		FAtomActiveStatusEffect* Existing = ActiveEffects.FindByPredicate([&StatusTag](const FAtomActiveStatusEffect& Effect)
		{
			return Effect.StatusTag.MatchesTagExact(StatusTag);
		});

		if (Existing)
		{
			Existing->StackCount = NewCount;
		}
		else
		{
			FAtomActiveStatusEffect NewEffect(StatusTag, NewCount, -1.f);
			ActiveEffects.Add(NewEffect);
			OnStatusEffectAdded.Broadcast(StatusTag, NewCount);
		}
	}
	else
	{
		int32 RemovedIndex = ActiveEffects.IndexOfByPredicate([&StatusTag](const FAtomActiveStatusEffect& Effect)
		{
			return Effect.StatusTag.MatchesTagExact(StatusTag);
		});

		if (RemovedIndex != INDEX_NONE)
		{
			ActiveEffects.RemoveAt(RemovedIndex);
			OnStatusEffectRemoved.Broadcast(StatusTag);
		}
	}
}
