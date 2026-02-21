// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "AtomHUDLayout.generated.h"

UCLASS(Abstract, Blueprintable)
class OUTLAW_API UAtomHUDLayout : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UAtomHUDLayout(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
};
