// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawAbilitySystemComponent.h"
#include "OutlawAbilitySet.h"
#include "OutlawGameplayAbility.h"

UOutlawAbilitySystemComponent::UOutlawAbilitySystemComponent()
{
	SetIsReplicated(true);
}

void UOutlawAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

FOutlawAbilitySetGrantedHandles UOutlawAbilitySystemComponent::GrantAbilitySet(const UOutlawAbilitySet* AbilitySet, UObject* SourceObject)
{
	FOutlawAbilitySetGrantedHandles Handles;

	if (AbilitySet)
	{
		AbilitySet->GiveToAbilitySystem(this, SourceObject, Handles);
	}

	return Handles;
}

void UOutlawAbilitySystemComponent::RevokeAbilitySet(FOutlawAbilitySetGrantedHandles& Handles)
{
	Handles.RevokeFromASC(this);
}

void UOutlawAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		InputPressedTags.Add(InputTag);
		InputHeldTags.AddTag(InputTag);
	}
}

void UOutlawAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		InputReleasedTags.Add(InputTag);
		InputHeldTags.RemoveTag(InputTag);
	}
}

void UOutlawAbilitySystemComponent::ProcessAbilityInput()
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
