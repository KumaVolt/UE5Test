// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "OutlawHUDLayout.generated.h"

UCLASS(Abstract, Blueprintable)
class OUTLAW_API UOutlawHUDLayout : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UOutlawHUDLayout(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
};
