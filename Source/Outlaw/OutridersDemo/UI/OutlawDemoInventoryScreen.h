#pragma once

#include "CoreMinimal.h"
#include "UI/OutlawDemoOverlayScreen.h"
#include "OutlawDemoInventoryScreen.generated.h"

class SOutlawDemoInventorySlate;

/**
 * Destiny-style full-screen inventory overlay.
 * Equipment slots on the left, scrollable inventory list on the right.
 * Click inventory items to equip, click equipment slots to unequip.
 */
UCLASS()
class OUTLAW_API UOutlawDemoInventoryScreen : public UOutlawDemoOverlayScreen
{
	GENERATED_BODY()

public:
	UOutlawDemoInventoryScreen(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	TSharedPtr<SOutlawDemoInventorySlate> SlateWidget;
};
