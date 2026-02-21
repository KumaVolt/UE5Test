#include "UI/OutlawDemoEquipmentPopup.h"
#include "Inventory/OutlawInventoryComponent.h"
#include "Inventory/OutlawInventoryTypes.h"
#include "Inventory/OutlawItemDefinition.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SSeparator.h"

// ── Rarity Color Helper ──────────────────────────────────────────

namespace EquipPopupColors
{
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
}

// ── Slate Widget ─────────────────────────────────────────────────

class SOutlawDemoEquipmentSlate : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SOutlawDemoEquipmentSlate) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		HeaderFont = FCoreStyle::GetDefaultFontStyle("Bold", 15);
		SubHeaderFont = FCoreStyle::GetDefaultFontStyle("Bold", 12);
		SlotFont = FCoreStyle::GetDefaultFontStyle("Bold", 12);
		ItemFont = FCoreStyle::GetDefaultFontStyle("Regular", 11);

		FLinearColor PanelBG(0.03f, 0.03f, 0.06f, 0.95f);

		ChildSlot
		[
			SNew(SOverlay)

			// Dim backdrop
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.ColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.5f))
			]

			// Centered popup
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(400.f)
				.HeightOverride(450.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
					.BorderBackgroundColor(PanelBG)
					.Padding(FMargin(14.f, 10.f))
					[
						SNew(SVerticalBox)

						// Header
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("LOADOUT")))
							.Font(HeaderFont)
							.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.7f, 0.3f)))
						]

						// Equipment section
						+ SVerticalBox::Slot().AutoHeight()
						[
							SAssignNew(EquipmentBox, SVerticalBox)
						]

						// Separator
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 8)
						[
							SNew(SSeparator)
							.ColorAndOpacity(FLinearColor(0.2f, 0.2f, 0.2f))
						]

						// Backpack header
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("BACKPACK")))
							.Font(SubHeaderFont)
							.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
						]

						// Backpack list
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SAssignNew(BackpackScrollBox, SScrollBox)
						]
					]
				]
			]
		];
	}

	void Update(UOutlawInventoryComponent* InvComp)
	{
		CachedInvComp = InvComp;
		if (!InvComp)
		{
			return;
		}

		const TArray<FOutlawInventoryEntry>& Entries = InvComp->GetEntries();
		int32 CurrentCount = Entries.Num();

		bool bNeedsRebuild = (CurrentCount != LastItemCount);
		if (!bNeedsRebuild)
		{
			for (const FOutlawEquipmentSlotInfo& Slot : InvComp->EquipmentSlots)
			{
				if (!LastEquipped.Contains(Slot.SlotTag) || LastEquipped[Slot.SlotTag] != Slot.EquippedItemInstanceId)
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

		RebuildEquipment(InvComp);
		RebuildBackpack(InvComp);
	}

private:
	void RebuildEquipment(UOutlawInventoryComponent* InvComp)
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

			FString SlotLabel = SlotPair.Key;
			FString ItemText;
			FLinearColor ItemColor;

			if (Equipped)
			{
				ItemText = Equipped->DisplayName.ToString();
				ItemColor = EquipPopupColors::GetRarityColor(Equipped->Rarity);
			}
			else
			{
				ItemText = TEXT("-- empty --");
				ItemColor = FLinearColor(0.3f, 0.3f, 0.3f);
			}

			EquipmentBox->AddSlot()
			.AutoHeight()
			.Padding(FMargin(0.f, 2.f))
			[
				SNew(SButton)
				.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
				.ContentPadding(FMargin(6.f, 4.f))
				.OnClicked_Lambda([this, SlotTag]() -> FReply
				{
					if (CachedInvComp.IsValid())
					{
						CachedInvComp->UnequipItem(SlotTag);
						LastItemCount = -1;
					}
					return FReply::Handled();
				})
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
					[
						SNew(SBox)
						.WidthOverride(80.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(SlotLabel))
							.Font(SlotFont)
							.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(ItemText))
						.Font(ItemFont)
						.ColorAndOpacity(FSlateColor(ItemColor))
					]
				]
			];
		}
	}

	void RebuildBackpack(UOutlawInventoryComponent* InvComp)
	{
		if (!BackpackScrollBox.IsValid())
		{
			return;
		}
		BackpackScrollBox->ClearChildren();

		const TArray<FOutlawInventoryEntry>& Entries = InvComp->GetEntries();

		if (Entries.Num() == 0)
		{
			BackpackScrollBox->AddSlot()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("  (empty)")))
				.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
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

			FLinearColor RarityColor = EquipPopupColors::GetRarityColor(Entry.ItemDef->Rarity);
			FString ItemName = Entry.ItemDef->DisplayName.IsEmpty()
				? Entry.ItemDef->GetName()
				: Entry.ItemDef->DisplayName.ToString();
			FString DisplayText = (Entry.StackCount > 1)
				? FString::Printf(TEXT("%s x%d"), *ItemName, Entry.StackCount)
				: ItemName;

			int32 InstanceId = Entry.InstanceId;

			BackpackScrollBox->AddSlot()
			.Padding(FMargin(0.f, 1.f))
			[
				SNew(SButton)
				.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
				.ContentPadding(FMargin(4.f, 2.f))
				.OnClicked_Lambda([this, InstanceId]() -> FReply
				{
					if (CachedInvComp.IsValid())
					{
						CachedInvComp->EquipItem(InstanceId);
						LastItemCount = -1;
					}
					return FReply::Handled();
				})
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(2.f, 0.f, 6.f, 0.f)
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
	TSharedPtr<SScrollBox> BackpackScrollBox;
	TMap<FGameplayTag, int32> LastEquipped;
	int32 LastItemCount = -1;

	FSlateFontInfo HeaderFont;
	FSlateFontInfo SubHeaderFont;
	FSlateFontInfo SlotFont;
	FSlateFontInfo ItemFont;
};

// ── UOutlawDemoEquipmentPopup ────────────────────────────────────

UOutlawDemoEquipmentPopup::UOutlawDemoEquipmentPopup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UOutlawDemoEquipmentPopup::RebuildWidget()
{
	SlateWidget = SNew(SOutlawDemoEquipmentSlate);
	return SlateWidget.ToSharedRef();
}

void UOutlawDemoEquipmentPopup::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
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
	SlateWidget->Update(InvComp);
}
