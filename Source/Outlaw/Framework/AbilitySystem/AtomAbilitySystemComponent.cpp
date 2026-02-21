// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomAbilitySystemComponent.h"
#include "AtomAbilitySet.h"
#include "AtomGameplayAbility.h"

UAtomAbilitySystemComponent::UAtomAbilitySystemComponent()
{
	SetIsReplicated(true);
}

void UAtomAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

FAtomAbilitySetGrantedHandles UAtomAbilitySystemComponent::GrantAbilitySet(const UAtomAbilitySet* AbilitySet, UObject* SourceObject)
{
	FAtomAbilitySetGrantedHandles Handles;

	if (AbilitySet)
	{
		AbilitySet->GiveToAbilitySystem(this, SourceObject, Handles);
	}

	return Handles;
}

void UAtomAbilitySystemComponent::RevokeAbilitySet(FAtomAbilitySetGrantedHandles& Handles)
{
	Handles.RevokeFromASC(this);
}

void UAtomAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		InputPressedTags.Add(InputTag);
		InputHeldTags.AddTag(InputTag);
	}
}

void UAtomAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		InputReleasedTags.Add(InputTag);
		InputHeldTags.RemoveTag(InputTag);
	}
}

void UAtomAbilitySystemComponent::ProcessAbilityInput()
{
	// Process pressed tags - try to activate matching abilities
	for (const FGameplayTag& Tag : InputPressedTags)
	{
		for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
		{
			if (Spec.Ability && Spec.GetDynamicSpecSourceTags().HasTagExact(Tag))
			{
				Spec.InputPressed = true;

				if (Spec.IsActive())
				{
					AbilitySpecInputPressed(Spec);
				}
				else
				{
					TryActivateAbility(Spec.Handle);
				}
			}
		}
	}

	// Process released tags
	for (const FGameplayTag& Tag : InputReleasedTags)
	{
		for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
		{
			if (Spec.Ability && Spec.GetDynamicSpecSourceTags().HasTagExact(Tag))
			{
				Spec.InputPressed = false;

				if (Spec.IsActive())
				{
					AbilitySpecInputReleased(Spec);
				}
			}
		}
	}

	InputPressedTags.Reset();
	InputReleasedTags.Reset();
}
