#include "UI/OutlawDemoHUD.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/OutlawAttributeSet.h"
#include "Weapon/OutlawWeaponManagerComponent.h"
#include "Weapon/OutlawShooterWeaponData.h"
#include "Inventory/OutlawItemInstance.h"
#include "Inventory/OutlawItemDefinition.h"
#include "Progression/OutlawProgressionComponent.h"
#include "Characters/OutlawEnemyCharacter.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Rendering/DrawElements.h"

// ── Enemy Health Bar Data ───────────────────────────────────────

struct FEnemyHealthBarData
{
	FVector2D ScreenPos;
	float HealthPercent;
};

// ── Custom Leaf Widget: Enemy Health Overlay ────────────────────

class SOutlawEnemyHealthOverlay : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SOutlawEnemyHealthOverlay) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		SetVisibility(EVisibility::HitTestInvisible);
	}

	void SetHealthBars(const TArray<FEnemyHealthBarData>& InBars)
	{
		HealthBars = InBars;
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D::ZeroVector;
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const float BarWidth = 80.f;
		const float BarHeight = 8.f;
		const float BarOffsetY = -20.f;

		for (const FEnemyHealthBarData& Bar : HealthBars)
		{
			FVector2D TopLeft(Bar.ScreenPos.X - BarWidth * 0.5f, Bar.ScreenPos.Y + BarOffsetY);

			// Background
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(FVector2f(BarWidth, BarHeight), FSlateLayoutTransform(FVector2f(TopLeft.X, TopLeft.Y))),
				FCoreStyle::Get().GetBrush("GenericWhiteBox"),
				ESlateDrawEffect::None,
				FLinearColor(0.05f, 0.05f, 0.05f, 0.85f)
			);

			// Fill
			float FillWidth = BarWidth * FMath::Clamp(Bar.HealthPercent, 0.f, 1.f);
			if (FillWidth > 0.f)
			{
				// Color: green > yellow > red
				FLinearColor FillColor;
				if (Bar.HealthPercent > 0.6f)
				{
					FillColor = FLinearColor(0.2f, 0.85f, 0.2f);
				}
				else if (Bar.HealthPercent > 0.3f)
				{
					float T = (Bar.HealthPercent - 0.3f) / 0.3f;
					FillColor = FMath::Lerp(FLinearColor(0.9f, 0.2f, 0.1f), FLinearColor(0.9f, 0.85f, 0.1f), T);
				}
				else
				{
					FillColor = FLinearColor(0.9f, 0.2f, 0.1f);
				}

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 1,
					AllottedGeometry.ToPaintGeometry(FVector2f(FillWidth, BarHeight), FSlateLayoutTransform(FVector2f(TopLeft.X, TopLeft.Y))),
					FCoreStyle::Get().GetBrush("GenericWhiteBox"),
					ESlateDrawEffect::None,
					FillColor
				);
			}
		}

		return LayerId + 2;
	}

private:
	TArray<FEnemyHealthBarData> HealthBars;
};

// ── Slate HUD Widget ──────────────────────────────────────────────

class SOutlawDemoHUDSlate : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SOutlawDemoHUDSlate) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		FSlateFontInfo LargeFont = FCoreStyle::GetDefaultFontStyle("Bold", 18);
		FSlateFontInfo MediumFont = FCoreStyle::GetDefaultFontStyle("Bold", 14);
		FSlateFontInfo SmallFont = FCoreStyle::GetDefaultFontStyle("Regular", 11);
		FSlateFontInfo AmmoFont = FCoreStyle::GetDefaultFontStyle("Bold", 22);

		FLinearColor PanelBG(0.02f, 0.02f, 0.04f, 0.75f);
		FLinearColor PanelBGLight(0.05f, 0.05f, 0.08f, 0.65f);

		ChildSlot
		[
			SNew(SConstraintCanvas)

			// ── Health Bar — bottom-left ──────────────────────
			+ SConstraintCanvas::Slot()
			.Anchors(FAnchors(0.f, 1.f, 0.f, 1.f))
			.Offset(FMargin(20.f, -110.f, 300.f, 50.f))
			.AutoSize(false)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor(PanelBG)
				.Padding(FMargin(10.f, 6.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SAssignNew(HealthBar, SProgressBar)
							.Percent(1.f)
							.FillColorAndOpacity(FLinearColor(0.2f, 0.85f, 0.2f))
							.BackgroundImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
							.Style(&FCoreStyle::Get().GetWidgetStyle<FProgressBarStyle>("ProgressBar"))
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SAssignNew(HealthLabel, STextBlock)
							.Text(FText::FromString(TEXT("HP 100 / 100")))
							.Font(MediumFont)
							.ColorAndOpacity(FSlateColor(FLinearColor::White))
						]
					]
				]
			]

			// ── Stamina Bar — below health ────────────────────
			+ SConstraintCanvas::Slot()
			.Anchors(FAnchors(0.f, 1.f, 0.f, 1.f))
			.Offset(FMargin(20.f, -55.f, 300.f, 35.f))
			.AutoSize(false)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor(PanelBG)
				.Padding(FMargin(10.f, 4.f))
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SAssignNew(StaminaBar, SProgressBar)
						.Percent(1.f)
						.FillColorAndOpacity(FLinearColor(0.85f, 0.65f, 0.1f))
						.BackgroundImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
						.Style(&FCoreStyle::Get().GetWidgetStyle<FProgressBarStyle>("ProgressBar"))
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(StaminaLabel, STextBlock)
						.Text(FText::FromString(TEXT("SP 200 / 200")))
						.Font(SmallFont)
						.ColorAndOpacity(FSlateColor(FLinearColor::White))
					]
				]
			]

			// ── Ammo Counter — bottom-right ───────────────────
			+ SConstraintCanvas::Slot()
			.Anchors(FAnchors(1.f, 1.f, 1.f, 1.f))
			.Offset(FMargin(-220.f, -80.f, 200.f, 60.f))
			.AutoSize(false)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor(PanelBG)
				.Padding(FMargin(16.f, 8.f))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SAssignNew(AmmoLabel, STextBlock)
					.Text(FText::FromString(TEXT("30 | 30")))
					.Font(AmmoFont)
					.ColorAndOpacity(FSlateColor(FLinearColor::White))
					.Justification(ETextJustify::Center)
				]
			]

			// ── Level + XP Bar — top-left ─────────────────────
			+ SConstraintCanvas::Slot()
			.Anchors(FAnchors(0.f, 0.f, 0.f, 0.f))
			.Offset(FMargin(20.f, 15.f, 180.f, 40.f))
			.AutoSize(false)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor(PanelBGLight)
				.Padding(FMargin(8.f, 4.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(LevelLabel, STextBlock)
						.Text(FText::FromString(TEXT("Lv.1")))
						.Font(MediumFont)
						.ColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.75f, 1.f)))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
					[
						SNew(SBox)
						.HeightOverride(6.f)
						[
							SAssignNew(XPBar, SProgressBar)
							.Percent(0.f)
							.FillColorAndOpacity(FLinearColor(0.3f, 0.65f, 1.f))
						]
					]
				]
			]

			// ── Crosshair — center ────────────────────────────
			+ SConstraintCanvas::Slot()
			.Anchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f))
			.Offset(FMargin(-10.f, -10.f, 20.f, 20.f))
			.AutoSize(false)
			[
				SAssignNew(CrosshairText, STextBlock)
				.Text(FText::FromString(TEXT("+")))
				.Font(MediumFont)
				.ColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.8f)))
				.Justification(ETextJustify::Center)
			]

			// ── Enemy Health Overlay — full screen ─────────────
			+ SConstraintCanvas::Slot()
			.Anchors(FAnchors(0.f, 0.f, 1.f, 1.f))
			.Offset(FMargin(0.f))
			.AutoSize(false)
			[
				SAssignNew(EnemyHealthOverlay, SOutlawEnemyHealthOverlay)
			]

		];
	}

	void UpdateHealth(float Current, float Max)
	{
		if (!HealthBar.IsValid() || !HealthLabel.IsValid() || Max <= 0.f)
		{
			return;
		}

		float Pct = FMath::Clamp(Current / Max, 0.f, 1.f);
		HealthBar->SetPercent(Pct);
		HealthLabel->SetText(FText::FromString(FString::Printf(TEXT("HP %d / %d"), FMath::RoundToInt(Current), FMath::RoundToInt(Max))));

		// Color interpolation: green > yellow > red
		FLinearColor BarColor;
		if (Pct > 0.6f)
		{
			BarColor = FLinearColor(0.2f, 0.85f, 0.2f);
		}
		else if (Pct > 0.3f)
		{
			float T = (Pct - 0.3f) / 0.3f;
			BarColor = FMath::Lerp(FLinearColor(0.9f, 0.2f, 0.1f), FLinearColor(0.9f, 0.85f, 0.1f), T);
		}
		else
		{
			BarColor = FLinearColor(0.9f, 0.2f, 0.1f);
		}
		HealthBar->SetFillColorAndOpacity(BarColor);
	}

	void UpdateStamina(float Current, float Max)
	{
		if (StaminaBar.IsValid() && StaminaLabel.IsValid() && Max > 0.f)
		{
			StaminaBar->SetPercent(Current / Max);
			StaminaLabel->SetText(FText::FromString(FString::Printf(TEXT("SP %d / %d"), FMath::RoundToInt(Current), FMath::RoundToInt(Max))));
		}
	}

	void UpdateAmmo(int32 Current, int32 Max)
	{
		if (AmmoLabel.IsValid())
		{
			AmmoLabel->SetText(FText::FromString(FString::Printf(TEXT("%d | %d"), Current, Max)));
		}
	}

	void UpdateLevel(int32 Level, float XPProgress)
	{
		if (LevelLabel.IsValid() && XPBar.IsValid())
		{
			LevelLabel->SetText(FText::FromString(FString::Printf(TEXT("Lv.%d"), Level)));
			XPBar->SetPercent(XPProgress);
		}
	}

	void UpdateEnemyHealthBars(const TArray<FEnemyHealthBarData>& Bars)
	{
		if (EnemyHealthOverlay.IsValid())
		{
			EnemyHealthOverlay->SetHealthBars(Bars);
		}
	}

private:
	TSharedPtr<SProgressBar> HealthBar;
	TSharedPtr<STextBlock> HealthLabel;
	TSharedPtr<SProgressBar> StaminaBar;
	TSharedPtr<STextBlock> StaminaLabel;
	TSharedPtr<STextBlock> AmmoLabel;
	TSharedPtr<STextBlock> LevelLabel;
	TSharedPtr<SProgressBar> XPBar;
	TSharedPtr<STextBlock> CrosshairText;
	TSharedPtr<SOutlawEnemyHealthOverlay> EnemyHealthOverlay;
};

// ── UOutlawDemoHUD ────────────────────────────────────────────────

UOutlawDemoHUD::UOutlawDemoHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UOutlawDemoHUD::RebuildWidget()
{
	SlateHUD = SNew(SOutlawDemoHUDSlate);
	return SlateHUD.ToSharedRef();
}

void UOutlawDemoHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!SlateHUD.IsValid())
	{
		return;
	}

	APawn* Pawn = GetOwningPlayerPawn();
	if (!Pawn)
	{
		return;
	}

	// Read Health/Stamina from ASC
	if (IAbilitySystemInterface* ASCOwner = Cast<IAbilitySystemInterface>(Pawn->GetPlayerState()))
	{
		if (UAbilitySystemComponent* ASC = ASCOwner->GetAbilitySystemComponent())
		{
			bool bFound = false;
			float Health = ASC->GetGameplayAttributeValue(UOutlawAttributeSet::GetHealthAttribute(), bFound);
			float MaxHealth = ASC->GetGameplayAttributeValue(UOutlawAttributeSet::GetMaxHealthAttribute(), bFound);
			float Stamina = ASC->GetGameplayAttributeValue(UOutlawAttributeSet::GetStaminaAttribute(), bFound);
			float MaxStamina = ASC->GetGameplayAttributeValue(UOutlawAttributeSet::GetMaxStaminaAttribute(), bFound);

			SlateHUD->UpdateHealth(Health, MaxHealth);
			SlateHUD->UpdateStamina(Stamina, MaxStamina);
		}
	}

	// Read Ammo from WeaponManager
	if (UOutlawWeaponManagerComponent* WeaponMgr = Pawn->FindComponentByClass<UOutlawWeaponManagerComponent>())
	{
		UOutlawItemInstance* ActiveWeapon = WeaponMgr->GetActiveWeapon();
		if (ActiveWeapon)
		{
			int32 CurrentAmmo = ActiveWeapon->CurrentAmmo;
			int32 MaxAmmo = 0;
			if (ActiveWeapon->ItemDef && ActiveWeapon->ItemDef->ShooterWeaponData)
			{
				MaxAmmo = ActiveWeapon->ItemDef->ShooterWeaponData->MagazineSize;
			}
			SlateHUD->UpdateAmmo(CurrentAmmo, MaxAmmo);
		}
		else
		{
			SlateHUD->UpdateAmmo(0, 0);
		}
	}

	// Read Level/XP from ProgressionComponent
	if (UOutlawProgressionComponent* Prog = Pawn->FindComponentByClass<UOutlawProgressionComponent>())
	{
		SlateHUD->UpdateLevel(Prog->GetCurrentLevel(), Prog->GetXPProgress());
	}

	// ── Enemy Health Bars ─────────────────────────────────────
	APlayerController* PC = Pawn->GetController<APlayerController>();
	UWorld* World = Pawn->GetWorld();
	TArray<FEnemyHealthBarData> EnemyBars;

	if (PC && World)
	{
		const FVector PlayerLoc = Pawn->GetActorLocation();
		const float MaxDistance = 3000.f;

		for (TActorIterator<AOutlawEnemyCharacter> It(World); It; ++It)
		{
			AOutlawEnemyCharacter* Enemy = *It;
			if (!Enemy || Enemy->IsPendingKillPending())
			{
				continue;
			}

			float Dist = FVector::Dist(PlayerLoc, Enemy->GetActorLocation());
			if (Dist > MaxDistance)
			{
				continue;
			}

			// Enemy ASC lives on the character itself
			UAbilitySystemComponent* EnemyASC = Enemy->GetAbilitySystemComponent();
			if (!EnemyASC)
			{
				continue;
			}

			bool bFound = false;
			float EHealth = EnemyASC->GetGameplayAttributeValue(UOutlawAttributeSet::GetHealthAttribute(), bFound);
			float EMaxHealth = EnemyASC->GetGameplayAttributeValue(UOutlawAttributeSet::GetMaxHealthAttribute(), bFound);
			if (EMaxHealth <= 0.f || EHealth <= 0.f)
			{
				continue;
			}

			// Project enemy position to screen (offset above head)
			FVector WorldPos = Enemy->GetActorLocation() + FVector(0.f, 0.f, 120.f);
			FVector2D ScreenPos;
			if (PC->ProjectWorldLocationToScreen(WorldPos, ScreenPos, true))
			{
				FEnemyHealthBarData Data;
				Data.ScreenPos = ScreenPos;
				Data.HealthPercent = FMath::Clamp(EHealth / EMaxHealth, 0.f, 1.f);
				EnemyBars.Add(Data);
			}
		}
	}
	SlateHUD->UpdateEnemyHealthBars(EnemyBars);
}
