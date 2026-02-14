// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/OutlawPlayerController.h"
#include "UI/OutlawHUDLayout.h"

void AOutlawPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();

	if (IsLocalController() && HUDLayoutClass && !HUDLayoutWidget)
	{
		HUDLayoutWidget = CreateWidget<UOutlawHUDLayout>(this, HUDLayoutClass);
		HUDLayoutWidget->AddToViewport();
	}
}
