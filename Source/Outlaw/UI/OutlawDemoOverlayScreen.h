#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "OutlawDemoOverlayScreen.generated.h"

/**
 * Base class for full-screen overlay screens (inventory, equipment, skill tree).
 * Returns Menu input mode so the mouse cursor appears. Game continues underneath.
 */
UCLASS(Abstract)
class OUTLAW_API UOutlawDemoOverlayScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UOutlawDemoOverlayScreen(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
};
