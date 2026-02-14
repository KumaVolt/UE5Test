// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OutlawPlayerController.generated.h"

class UOutlawHUDLayout;

UCLASS()
class OUTLAW_API AOutlawPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UOutlawHUDLayout> HUDLayoutClass;

	virtual void BeginPlayingState() override;

private:
	UPROPERTY()
	TObjectPtr<UOutlawHUDLayout> HUDLayoutWidget;
};
