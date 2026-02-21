// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/AtomAbilityTypes.h"
#include "GameFramework/Character.h"
#include "AtomCharacterBase.generated.h"

class UAtomAbilitySystemComponent;
class UAtomAbilitySet;

UCLASS()
class OUTLAW_API AAtomCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAtomCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	void GrantDefaultAbilitySet();
	void RevokeDefaultAbilitySet();

	UPROPERTY()
	TObjectPtr<UAtomAbilitySystemComponent> AbilitySystemComponent;

	/** The default ability set granted at spawn. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<UAtomAbilitySet> DefaultAbilitySet;

	/** Handles for the currently granted default ability set. */
	FAtomAbilitySetGrantedHandles DefaultAbilitySetHandles;
};
