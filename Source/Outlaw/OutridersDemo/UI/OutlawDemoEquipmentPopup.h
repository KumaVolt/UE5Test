#pragma once

#include "CoreMinimal.h"
#include "UI/OutlawDemoOverlayScreen.h"
#include "OutlawDemoEquipmentPopup.generated.h"

class SOutlawDemoEquipmentSlate;

/**
 * Outriders-style compact equipment popup.
 * Centered panel showing equipped loadout + backpack items for quick swap.
 */
UCLASS()
class OUTLAW_API UOutlawDemoEquipmentPopup : public UOutlawDemoOverlayScreen
{
	GENERATED_BODY()

public:
	UOutlawDemoEquipmentPopup(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	TSharedPtr<SOutlawDemoEquipmentSlate> SlateWidget;
};
