#include "UI/OutlawDemoInventoryScreen.h"
#include "Inventory/OutlawInventoryComponent.h"
#include "Inventory/OutlawInventoryTypes.h"
#include "Inventory/OutlawItemDefinition.h"
#include "Progression/OutlawProgressionComponent.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SSpacer.h"

// ── Rarity Color Helper ──────────────────────────────────────────

static FLinearColor GetRarityColor(EOutlawItemRarity Rarity)
{
	switch (Rarity)
	{
	case EOutlawItemRarity::Common:    return FLinearColor(0.8f, 0.8f, 0.8f);
	case EOutlawItemRarity::Uncommon:  return FLinearColor(0.2f, 0.9f, 0.2f);
	case EOutlawItemRarity::Rare:      return FLinearColor(0.3f, 0.5f, 1.0f);
	case EOutlawItemRarity::Epic:      return FLinearColor(0.7f, 0.3f, 0.9f);
	case EOutlawItemRarity::Legendary: return FLinearColor(1.0f, 0.65f, 0.0f);
	default:                           return FLinearColor::White;
	}
}

// ── Slate Widget ─────────────────────────────────────────────────

class SOutlawDemoInventorySlate : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SOutlawDemoInventorySlate) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		HeaderFont = FCoreStyle::GetDefaultFontStyle("Bold", 16);
		SlotFont = FCoreStyle::GetDefaultFontStyle("Bold", 13);
		ItemFont = FCoreStyle::GetDefaultFontStyle("Regular", 12);
		InfoFont = FCoreStyle::GetDefaultFontStyle("Regular", 11);

		FLinearColor PanelBG(0.03f, 0.03f, 0.06f, 0.92f);
		FLinearColor SectionBG(0.06f, 0.06f, 0.1f, 0.85f);

		ChildSlot
		[
			SNew(SOverlay)

			// Dark backdrop
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.ColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.7f))
			]

			// Content
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(900.f)
				.HeightOverride(550.f)
				[
					SNew(SHorizontalBox)

					// ── Left: Equipment ──────────────────
					+ SHorizontalBox::Slot()
					.Padding(8.f)
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(300.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
							.BorderBackgroundColor(PanelBG)
							.Padding(FMargin(12.f, 10.f))
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
								[
									SNew(STextBlock)
									.Text(FText::FromString(TEXT("EQUIPMENT")))
									.Font(HeaderFont)
									.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.7f, 0.3f)))
								]

								+ SVerticalBox::Slot().AutoHeight()
								[
									SAssignNew(EquipmentBox, SVerticalBox)
								]

								+ SVerticalBox::Slot().FillHeight(1.f)
								[
									SNew(SSpacer)
								]

								+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
								[
									SAssignNew(InfoLabel, STextBlock)
									.Text(FText::FromString(TEXT("Level: 1  |  Skill Points: 0")))
									.Font(InfoFont)
									.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
								]
							]
						]
					]

					// ── Right: Inventory List ────────────
					+ SHorizontalBox::Slot()
					.Padding(8.f)
					.FillWidth(1.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
						.BorderBackgroundColor(PanelBG)
						.Padding(FMargin(12.f, 10.f))
						[
							SNew(SVerticalBox)

							+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("INVENTORY")))
								.Font(HeaderFont)
								.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
							]

							+ SVerticalBox::Slot().FillHeight(1.f)
							[
								SAssignNew(InventoryScrollBox, SScrollBox)
							]
						]
					]
				]
			]
		];
	}

	void Update(UOutlawInventoryComponent* InvComp, UOutlawProgressionComponent* ProgComp)
	{
		CachedInvComp = InvComp;
		if (!InvComp)
		{
			return;
		}

		const TArray<FOutlawInventoryEntry>& Entries = InvComp->GetEntries();
		int32 CurrentCount = Entries.Num();
		// Simple change detection
		bool bNeedsRebuild = (CurrentCount != LastItemCount);
		if (!bNeedsRebuild)
		{
			// Check if equipped items changed
			for (const FOutlawEquipmentSlotInfo& Slot : InvComp->EquipmentSlots)
			{
				int32 Equipped = Slot.EquippedItemInstanceId;
				if (!LastEquipped.Contains(Slot.SlotTag) || LastEquipped[Slot.SlotTag] != Equipped)
				{
					bNeedsRebuild = true;
					break;
				}
			}
		}

		if (!bNeedsRebuild)
		{
			return;
		}

		LastItemCount = CurrentCount;
		LastEquipped.Empty();
		for (const FOutlawEquipmentSlotInfo& Slot : InvComp->EquipmentSlots)
		{
			LastEquipped.Add(Slot.SlotTag, Slot.EquippedItemInstanceId);
		}

		RebuildEquipmentPanel(InvComp);
		RebuildInventoryList(InvComp);

		if (ProgComp && InfoLabel.IsValid())
		{
			InfoLabel->SetText(FText::FromString(FString::Printf(
				TEXT("Level: %d  |  Skill Points: %d"),
				ProgComp->GetCurrentLevel(), ProgComp->GetAvailableSkillPoints())));
		}
	}

private:
	void RebuildEquipmentPanel(UOutlawInventoryComponent* InvComp)
	{
		if (!EquipmentBox.IsValid())
		{
			return;
		}
		EquipmentBox->ClearChildren();

		TArray<TPair<FString, FGameplayTag>> SlotNames;
		SlotNames.Add(MakeTuple(FString(TEXT("Primary 1")), FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Primary1"))));
		SlotNames.Add(MakeTuple(FString(TEXT("Primary 2")), FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Primary2"))));
		SlotNames.Add(MakeTuple(FString(TEXT("Sidearm")), FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Sidearm"))));

		for (const auto& SlotPair : SlotNames)
		{
			FGameplayTag SlotTag = SlotPair.Value;
			UOutlawItemDefinition* Equipped = InvComp->GetEquippedItem(SlotTag);

			FString ButtonText;
			FLinearColor TextColor;
			if (Equipped)
			{
				ButtonText = FString::Printf(TEXT("[%s]  %s"), *SlotPair.Key, *Equipped->DisplayName.ToString());
				TextColor = GetRarityColor(Equipped->Rarity);
			}
			else
			{
				ButtonText = FString::Printf(TEXT("[%s]  -- empty --"), *SlotPair.Key);
				TextColor = FLinearColor(0.35f, 0.35f, 0.35f);
			}

			EquipmentBox->AddSlot()
			.AutoHeight()
			.Padding(FMargin(0.f, 2.f))
			[
				SNew(SButton)
				.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
				.ContentPadding(FMargin(8.f, 6.f))
				.OnClicked_Lambda([this, SlotTag]() -> FReply
				{
					if (CachedInvComp.IsValid())
					{
						CachedInvComp->UnequipItem(SlotTag);
						LastItemCount = -1; // Force rebuild
					}
					return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Text(FText::FromString(ButtonText))
					.Font(SlotFont)
					.ColorAndOpacity(FSlateColor(TextColor))
				]
			];
		}
	}

	void RebuildInventoryList(UOutlawInventoryComponent* InvComp)
	{
		if (!InventoryScrollBox.IsValid())
		{
			return;
		}
		InventoryScrollBox->ClearChildren();

		const TArray<FOutlawInventoryEntry>& Entries = InvComp->GetEntries();

		if (Entries.Num() == 0)
		{
			InventoryScrollBox->AddSlot()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("  (empty)")))
				.Font(FCoreStyle::GetDefaultFontStyle("Italic", 11))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.4f, 0.4f)))
			];
			return;
		}

		for (const FOutlawInventoryEntry& Entry : Entries)
		{
			if (!Entry.ItemDef)
			{
				continue;
			}

			FLinearColor RarityColor = GetRarityColor(Entry.ItemDef->Rarity);
			FString ItemName = Entry.ItemDef->DisplayName.IsEmpty()
				? Entry.ItemDef->GetName()
				: Entry.ItemDef->DisplayName.ToString();
			FString DisplayText = (Entry.StackCount > 1)
				? FString::Printf(TEXT("%s x%d"), *ItemName, Entry.StackCount)
				: ItemName;

			int32 InstanceId = Entry.InstanceId;

			InventoryScrollBox->AddSlot()
			.Padding(FMargin(0.f, 1.f))
			[
				SNew(SButton)
				.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
				.ContentPadding(FMargin(4.f, 3.f))
				.OnClicked_Lambda([this, InstanceId]() -> FReply
				{
					if (CachedInvComp.IsValid())
					{
						CachedInvComp->EquipItem(InstanceId);
						LastItemCount = -1; // Force rebuild
					}
					return FReply::Handled();
				})
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(2.f, 0.f, 8.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(8.f)
						.HeightOverride(8.f)
						[
							SNew(SImage)
							.Image(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
							.ColorAndOpacity(RarityColor)
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(DisplayText))
						.Font(ItemFont)
						.ColorAndOpacity(FSlateColor(RarityColor))
					]
				]
			];
		}
	}

	TWeakObjectPtr<UOutlawInventoryComponent> CachedInvComp;
	TSharedPtr<SVerticalBox> EquipmentBox;
	TSharedPtr<SScrollBox> InventoryScrollBox;
	TSharedPtr<STextBlock> InfoLabel;
	TMap<FGameplayTag, int32> LastEquipped;
	int32 LastItemCount = -1;

	FSlateFontInfo HeaderFont;
	FSlateFontInfo SlotFont;
	FSlateFontInfo ItemFont;
	FSlateFontInfo InfoFont;
};

// ── UOutlawDemoInventoryScreen ───────────────────────────────────

UOutlawDemoInventoryScreen::UOutlawDemoInventoryScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UOutlawDemoInventoryScreen::RebuildWidget()
{
	SlateWidget = SNew(SOutlawDemoInventorySlate);
	return SlateWidget.ToSharedRef();
}

void UOutlawDemoInventoryScreen::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!SlateWidget.IsValid())
	{
		return;
	}

	APawn* Pawn = GetOwningPlayerPawn();
	if (!Pawn)
	{
		return;
	}

	UOutlawInventoryComponent* InvComp = Pawn->FindComponentByClass<UOutlawInventoryComponent>();
	UOutlawProgressionComponent* ProgComp = Pawn->FindComponentByClass<UOutlawProgressionComponent>();
	SlateWidget->Update(InvComp, ProgComp);
}
