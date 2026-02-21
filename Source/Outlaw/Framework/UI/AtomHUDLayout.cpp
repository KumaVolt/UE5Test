// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/OutlawHUDLayout.h"

UOutlawHUDLayout::UOutlawHUDLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoActivate = true;
}

TOptional<FUIInputConfig> UOutlawHUDLayout::GetDesiredInputConfig() const
{
	return FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::CaptureDuringMouseDown);
}
