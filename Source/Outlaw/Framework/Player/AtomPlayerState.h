// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "OutlawPlayerState.generated.h"

class UOutlawAbilitySystemComponent;

UCLASS()
class OUTLAW_API AOutlawPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AOutlawPlayerState();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY()
	TObjectPtr<UOutlawAbilitySystemComponent> AbilitySystemComponent;
};
