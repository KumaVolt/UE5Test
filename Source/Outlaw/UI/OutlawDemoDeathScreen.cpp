#include "UI/OutlawDemoDeathScreen.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"

UOutlawDemoDeathScreen::UOutlawDemoDeathScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UOutlawDemoDeathScreen::RebuildWidget()
{
	ElapsedTime = 0.f;

	return SNew(SOverlay)
		// Dark overlay
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.ColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.6f))
		]
		// Text centered
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SAssignNew(DeathText, STextBlock)
				.Text(FText::FromString(TEXT("YOU DIED")))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.1f, 0.1f)))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 20)
			[
				SAssignNew(TimerText, STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("Respawning in %.1f..."), RespawnDelay)))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
			]
		];
}

void UOutlawDemoDeathScreen::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ElapsedTime += InDeltaTime;
	float Remaining = FMath::Max(0.f, RespawnDelay - ElapsedTime);

	if (TimerText.IsValid())
	{
		TimerText->SetText(FText::FromString(FString::Printf(TEXT("Respawning in %.1f..."), Remaining)));
	}
}
