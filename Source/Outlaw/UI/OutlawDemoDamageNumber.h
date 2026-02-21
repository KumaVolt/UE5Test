#pragma once

#include "CoreMinimal.h"
#include "UI/OutlawDamageNumberWidget.h"
#include "OutlawDemoDamageNumber.generated.h"

/**
 * Concrete damage number widget for the demo.
 * Pure C++ — shows damage text that floats up and fades out.
 */
UCLASS()
class OUTLAW_API UOutlawDemoDamageNumber : public UOutlawDamageNumberWidget
{
	GENERATED_BODY()

public:
	UOutlawDemoDamageNumber(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void OnDamageNumberInit_Implementation(float Amount, bool bIsCrit) override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	TSharedPtr<STextBlock> DamageText;
	float LifeTime = 1.5f;
	float ElapsedTime = 0.f;
	float DamageAmount = 0.f;
	bool bIsCritical = false;
};
