// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/OutlawAbilityTypes.h"
#include "GameFramework/Character.h"
#include "OutlawCharacterBase.generated.h"

class UOutlawAbilitySystemComponent;
class UOutlawAbilitySet;

UCLASS()
class OUTLAW_API AOutlawCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AOutlawCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	void GrantDefaultAbilitySet();
	void RevokeDefaultAbilitySet();

	UPROPERTY()
	TObjectPtr<UOutlawAbilitySystemComponent> AbilitySystemComponent;

	/** The default ability set granted at spawn. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<UOutlawAbilitySet> DefaultAbilitySet;

	/** Handles for the currently granted default ability set. */
	FOutlawAbilitySetGrantedHandles DefaultAbilitySetHandles;
};
