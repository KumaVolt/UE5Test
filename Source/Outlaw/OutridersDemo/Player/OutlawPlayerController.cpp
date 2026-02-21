#include "Player/OutlawPlayerController.h"
#include "UI/AtomHUDLayout.h"
#include "UI/OutlawDemoHUD.h"
#include "UI/OutlawDemoOverlayScreen.h"
#include "UI/OutlawDemoInventoryScreen.h"
#include "UI/OutlawDemoEquipmentPopup.h"
#include "UI/OutlawDemoSkillTree.h"

AOutlawPlayerController::AOutlawPlayerController()
{
	HUDLayoutClass = UOutlawDemoHUD::StaticClass();
}

void AOutlawPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CreateHUD();
	CreateOverlays();
}

void AOutlawPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();

	CreateHUD();
	CreateOverlays();
}

void AOutlawPlayerController::CreateHUD()
{
	if (IsLocalController() && HUDLayoutClass && !HUDLayoutWidget)
	{
		HUDLayoutWidget = CreateWidget<UAtomHUDLayout>(this, HUDLayoutClass);
		if (HUDLayoutWidget)
		{
			HUDLayoutWidget->AddToViewport();
			HUDLayoutWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
}

void AOutlawPlayerController::CreateOverlays()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!InventoryScreen)
	{
		InventoryScreen = CreateWidget<UOutlawDemoInventoryScreen>(this);
		if (InventoryScreen)
		{
			InventoryScreen->AddToViewport(10);
			InventoryScreen->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (!EquipmentPopup)
	{
		EquipmentPopup = CreateWidget<UOutlawDemoEquipmentPopup>(this);
		if (EquipmentPopup)
		{
			EquipmentPopup->AddToViewport(10);
			EquipmentPopup->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (!SkillTreeScreen)
	{
		SkillTreeScreen = CreateWidget<UOutlawDemoSkillTree>(this);
		if (SkillTreeScreen)
		{
			SkillTreeScreen->AddToViewport(10);
			SkillTreeScreen->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void AOutlawPlayerController::ToggleOverlay(UOutlawDemoOverlayScreen* Screen)
{
	if (!Screen)
	{
		return;
	}

	// Closing the currently active overlay
	if (ActiveOverlay == Screen)
	{
		Screen->DeactivateWidget();
		Screen->SetVisibility(ESlateVisibility::Collapsed);
		ActiveOverlay = nullptr;
		return;
	}

	// Close any other active overlay first
	if (ActiveOverlay)
	{
		ActiveOverlay->DeactivateWidget();
		ActiveOverlay->SetVisibility(ESlateVisibility::Collapsed);
		ActiveOverlay = nullptr;
	}

	// Open the requested overlay
	Screen->SetVisibility(ESlateVisibility::Visible);
	Screen->ActivateWidget();
	ActiveOverlay = Screen;
}

void AOutlawPlayerController::ToggleInventoryScreen()
{
	ToggleOverlay(InventoryScreen);
}

void AOutlawPlayerController::ToggleEquipmentPopup()
{
	ToggleOverlay(EquipmentPopup);
}

void AOutlawPlayerController::ToggleSkillTree()
{
	ToggleOverlay(SkillTreeScreen);
}
