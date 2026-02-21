// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/AtomHUDLayout.h"

UAtomHUDLayout::UAtomHUDLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoActivate = true;
}

TOptional<FUIInputConfig> UAtomHUDLayout::GetDesiredInputConfig() const
{
	return FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::CaptureDuringMouseDown);
}
