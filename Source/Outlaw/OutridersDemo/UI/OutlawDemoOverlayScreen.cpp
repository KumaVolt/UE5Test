#include "UI/OutlawDemoOverlayScreen.h"

UOutlawDemoOverlayScreen::UOutlawDemoOverlayScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoActivate = false;
}

TOptional<FUIInputConfig> UOutlawDemoOverlayScreen::GetDesiredInputConfig() const
{
	FUIInputConfig Config(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	return Config;
}
