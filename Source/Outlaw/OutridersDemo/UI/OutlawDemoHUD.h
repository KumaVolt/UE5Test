#pragma once

#include "CoreMinimal.h"
#include "UI/AtomHUDLayout.h"
#include "OutlawDemoHUD.generated.h"

class SOutlawDemoHUDSlate;

/**
 * Concrete HUD layout for the demo. Pure C++ — builds its own Slate widget tree.
 * Shows: Health bar, Stamina bar, Ammo counter, XP bar, Level, Crosshair.
 */
UCLASS()
class OUTLAW_API UOutlawDemoHUD : public UAtomHUDLayout
{
	GENERATED_BODY()

public:
	UOutlawDemoHUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	TSharedPtr<SOutlawDemoHUDSlate> SlateHUD;
};
