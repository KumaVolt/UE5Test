#include "UI/OutlawDemoDamageNumber.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"

UOutlawDemoDamageNumber::UOutlawDemoDamageNumber(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UOutlawDemoDamageNumber::RebuildWidget()
{
	return SNew(SBox)
		.WidthOverride(100.f)
		.HeightOverride(30.f)
		[
			SAssignNew(DamageText, STextBlock)
			.Text(FText::FromString(TEXT("0")))
			.ColorAndOpacity(FSlateColor(FLinearColor::White))
			.Justification(ETextJustify::Center)
		];
}

void UOutlawDemoDamageNumber::OnDamageNumberInit_Implementation(float Amount, bool bIsCrit)
{
	DamageAmount = Amount;
	bIsCritical = bIsCrit;
	ElapsedTime = 0.f;

	if (DamageText.IsValid())
	{
		FString Text = bIsCrit
			? FString::Printf(TEXT("CRIT %d!"), FMath::RoundToInt(Amount))
			: FString::Printf(TEXT("%d"), FMath::RoundToInt(Amount));

		FLinearColor Color = bIsCrit
			? FLinearColor(1.f, 0.8f, 0.f)   // Gold for crits
			: FLinearColor::White;

		DamageText->SetText(FText::FromString(Text));
		DamageText->SetColorAndOpacity(FSlateColor(Color));
	}
}

void UOutlawDemoDamageNumber::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ElapsedTime += InDeltaTime;

	if (ElapsedTime >= LifeTime)
	{
		RemoveFromParent();
		return;
	}

	// Float upward and fade out
	float Alpha = 1.f - (ElapsedTime / LifeTime);
	if (DamageText.IsValid())
	{
		FLinearColor Color = bIsCritical
			? FLinearColor(1.f, 0.8f, 0.f, Alpha)
			: FLinearColor(1.f, 1.f, 1.f, Alpha);
		DamageText->SetColorAndOpacity(FSlateColor(Color));
	}

	// Move widget upward via render transform
	SetRenderTranslation(FVector2D(0.f, -50.f * (ElapsedTime / LifeTime)));
}
