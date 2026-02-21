#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OutlawPlayerController.generated.h"

class UAtomHUDLayout;
class UOutlawDemoOverlayScreen;
class UOutlawDemoInventoryScreen;
class UOutlawDemoEquipmentPopup;
class UOutlawDemoSkillTree;

UCLASS()
class OUTLAW_API AOutlawPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AOutlawPlayerController();

	UAtomHUDLayout* GetHUDLayout() const { return HUDLayoutWidget; }

	void ToggleInventoryScreen();
	void ToggleEquipmentPopup();
	void ToggleSkillTree();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UAtomHUDLayout> HUDLayoutClass;

	virtual void BeginPlay() override;
	virtual void BeginPlayingState() override;

private:
	void CreateHUD();
	void CreateOverlays();
	void ToggleOverlay(UOutlawDemoOverlayScreen* Screen);

	UPROPERTY()
	TObjectPtr<UAtomHUDLayout> HUDLayoutWidget;

	UPROPERTY()
	TObjectPtr<UOutlawDemoInventoryScreen> InventoryScreen;

	UPROPERTY()
	TObjectPtr<UOutlawDemoEquipmentPopup> EquipmentPopup;

	UPROPERTY()
	TObjectPtr<UOutlawDemoSkillTree> SkillTreeScreen;

	UPROPERTY()
	TObjectPtr<UOutlawDemoOverlayScreen> ActiveOverlay;
};
