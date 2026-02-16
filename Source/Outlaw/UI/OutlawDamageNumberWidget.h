// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "OutlawDamageNumberWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class OUTLAW_API UOutlawDamageNumberWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Damage Number")
	void InitDamageNumber(float Amount, bool bCrit, FVector WorldLocation);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Damage Number")
	void OnDamageNumberInit(float Amount, bool bIsCrit);
};
