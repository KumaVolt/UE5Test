// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "OutlawAbilityTypes.h"
#include "OutlawAbilitySystemComponent.generated.h"

class UOutlawAbilitySet;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UOutlawAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UOutlawAbilitySystemComponent();

	/**
	 * Grants an entire ability set and returns handles for later revocation.
	 * @param AbilitySet     The set to grant.
	 * @param SourceObject   The object responsible for granting (e.g. equipment).
	 * @return Handles that can be passed to RevokeAbilitySet.
	 */
	FOutlawAbilitySetGrantedHandles GrantAbilitySet(const UOutlawAbilitySet* AbilitySet, UObject* SourceObject);

	/** Revokes everything previously granted by GrantAbilitySet. */
	void RevokeAbilitySet(FOutlawAbilitySetGrantedHandles& Handles);

	/** Called when an input tag is pressed. Finds matching abilities and activates them. */
	void AbilityInputTagPressed(const FGameplayTag& InputTag);

	/** Called when an input tag is released. Finds matching abilities and deactivates them. */
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	/** Per-frame processing of pending input state. Call from character Tick or input processing. */
	void ProcessAbilityInput();

protected:
	virtual void BeginPlay() override;

private:
	/** Input tags pressed this frame, pending processing. */
	TArray<FGameplayTag> InputPressedTags;

	/** Input tags released this frame, pending processing. */
	TArray<FGameplayTag> InputReleasedTags;

	/** Input tags currently held down. */
	FGameplayTagContainer InputHeldTags;
};
