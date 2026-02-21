#pragma once

#include "CoreMinimal.h"
#include "UI/OutlawDemoOverlayScreen.h"
#include "OutlawDemoSkillTree.generated.h"

class SOutlawDemoSkillTreeSlate;

/**
 * PoE2-style skill tree overlay.
 * Custom SLeafWidget canvas with pan/zoom, node rendering, prerequisite lines.
 * Left-click to allocate, right-click to deallocate.
 */
UCLASS()
class OUTLAW_API UOutlawDemoSkillTree : public UOutlawDemoOverlayScreen
{
	GENERATED_BODY()

public:
	UOutlawDemoSkillTree(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	TSharedPtr<SOutlawDemoSkillTreeSlate> SlateWidget;
};
