// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "AtomDamageNumberWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class OUTLAW_API UAtomDamageNumberWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Damage Number")
	void InitDamageNumber(float Amount, bool bCrit, FVector WorldLocation);

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Damage Number")
	void OnDamageNumberInit(float Amount, bool bIsCrit);
	virtual void OnDamageNumberInit_Implementation(float Amount, bool bIsCrit) {}
};
