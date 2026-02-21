#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OutlawDemoDeathScreen.generated.h"

/**
 * Pure C++ death screen overlay. Shows "YOU DIED" text with a respawn timer countdown.
 */
UCLASS()
class OUTLAW_API UOutlawDemoDeathScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	UOutlawDemoDeathScreen(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void SetRespawnDelay(float Delay) { RespawnDelay = Delay; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	TSharedPtr<STextBlock> DeathText;
	TSharedPtr<STextBlock> TimerText;
	float RespawnDelay = 3.f;
	float ElapsedTime = 0.f;
};
