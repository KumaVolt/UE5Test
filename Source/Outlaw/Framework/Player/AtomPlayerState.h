// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AtomPlayerState.generated.h"

class UAtomAbilitySystemComponent;

UCLASS()
class OUTLAW_API AAtomPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AAtomPlayerState();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY()
	TObjectPtr<UAtomAbilitySystemComponent> AbilitySystemComponent;
};
