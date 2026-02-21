#include "UI/OutlawDemoSkillTree.h"
#include "Progression/AtomProgressionComponent.h"
#include "Progression/AtomClassDefinition.h"
#include "Progression/AtomSkillTreeNodeDefinition.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Rendering/DrawElements.h"

// ── Visual data for a single node ────────────────────────────────

struct FSkillNodeVisual
{
	FGameplayTag NodeTag;
	FText DisplayName;
	float TreeX = 0.f;
	float TreeY = 0.f;
	int32 MaxRank = 1;
	int32 AllocatedRank = 0;
	bool bCanAllocate = false;
	TArray<FGameplayTag> PrerequisiteTags;
};

// ── Canvas Widget (SLeafWidget) ──────────────────────────────────

class SOutlawDemoSkillTreeCanvas : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SOutlawDemoSkillTreeCanvas) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		SetVisibility(EVisibility::Visible);
		PanOffset = FVector2D::ZeroVector;
		ZoomLevel = 1.0f;
		bIsDragging = false;
	}

	void SetNodes(const TArray<FSkillNodeVisual>& InNodes)
	{
		Nodes = InNodes;
	}

	void SetProgressionComponent(UAtomProgressionComponent* InComp)
	{
		ProgComp = InComp;
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(800.f, 400.f);
	}

	virtual bool SupportsKeyboardFocus() const override { return true; }

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		FVector2D LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());

		// Hit-test nodes
		int32 HitIndex = HitTestNode(LocalPos, MyGeometry);

		if (HitIndex != INDEX_NONE && ProgComp.IsValid())
		{
			if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				ProgComp->AllocateSkillNode(Nodes[HitIndex].NodeTag);
				return FReply::Handled();
			}
			else if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
			{
				ProgComp->DeallocateSkillNode(Nodes[HitIndex].NodeTag);
				return FReply::Handled();
			}
		}

		// Start drag (pan)
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			bIsDragging = true;
			DragStart = LocalPos;
			PanStart = PanOffset;
			return FReply::Handled().CaptureMouse(SharedThis(this));
		}

		return FReply::Unhandled();
	}

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (bIsDragging && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			bIsDragging = false;
			return FReply::Handled().ReleaseMouseCapture();
		}
		return FReply::Unhandled();
	}

	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (bIsDragging)
		{
			FVector2D CurrentPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
			PanOffset = PanStart + (CurrentPos - DragStart);
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}

	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		float Delta = MouseEvent.GetWheelDelta() * 0.1f;
		float OldZoom = ZoomLevel;
		ZoomLevel = FMath::Clamp(ZoomLevel + Delta, 0.5f, 2.5f);

		// Zoom toward cursor
		FVector2D LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		FVector2D Center = MyGeometry.GetLocalSize() * 0.5f;
		FVector2D CursorFromCenter = LocalPos - Center - PanOffset;
		PanOffset -= CursorFromCenter * (ZoomLevel / OldZoom - 1.f);

		return FReply::Handled();
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("GenericWhiteBox");
		FVector2D Center = AllottedGeometry.GetLocalSize() * 0.5f;
		const float GridScale = 80.f;

		// Draw prerequisite lines
		for (const FSkillNodeVisual& Node : Nodes)
		{
			FVector2D NodeScreen = GridToScreen(Node.TreeX, Node.TreeY, Center, GridScale);

			for (const FGameplayTag& PrereqTag : Node.PrerequisiteTags)
			{
				const FSkillNodeVisual* PrereqNode = FindNodeByTag(PrereqTag);
				if (!PrereqNode)
				{
					continue;
				}

				FVector2D PrereqScreen = GridToScreen(PrereqNode->TreeX, PrereqNode->TreeY, Center, GridScale);
				bool bBothAllocated = (Node.AllocatedRank > 0) && (PrereqNode->AllocatedRank > 0);
				FLinearColor LineColor = bBothAllocated
					? FLinearColor(0.2f, 0.85f, 0.2f, 0.8f)
					: FLinearColor(0.3f, 0.3f, 0.3f, 0.6f);

				TArray<FVector2D> Points;
				Points.Add(PrereqScreen);
				Points.Add(NodeScreen);

				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId,
					AllottedGeometry.ToPaintGeometry(),
					Points,
					ESlateDrawEffect::None,
					LineColor,
					true,
					2.f
				);
			}
		}

		LayerId++;

		// Draw nodes
		const float NodeRadius = 24.f;
		FSlateFontInfo NodeFont = FCoreStyle::GetDefaultFontStyle("Bold", 10);
		FSlateFontInfo RankFont = FCoreStyle::GetDefaultFontStyle("Bold", 11);

		for (const FSkillNodeVisual& Node : Nodes)
		{
			FVector2D NodeScreen = GridToScreen(Node.TreeX, Node.TreeY, Center, GridScale);

			// Node color
			FLinearColor NodeColor;
			if (Node.AllocatedRank >= Node.MaxRank)
			{
				NodeColor = FLinearColor(0.15f, 0.7f, 0.15f, 0.9f); // Fully allocated (green)
			}
			else if (Node.AllocatedRank > 0)
			{
				NodeColor = FLinearColor(0.2f, 0.6f, 0.2f, 0.85f); // Partially allocated
			}
			else if (Node.bCanAllocate)
			{
				NodeColor = FLinearColor(0.15f, 0.6f, 0.85f, 0.85f); // Available (cyan)
			}
			else
			{
				NodeColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.75f); // Locked (gray)
			}

			// Draw circle as rounded box
			FVector2D BoxTopLeft(NodeScreen.X - NodeRadius, NodeScreen.Y - NodeRadius);
			FVector2D BoxSize(NodeRadius * 2.f, NodeRadius * 2.f);

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(FVector2f(BoxSize.X, BoxSize.Y),
					FSlateLayoutTransform(FVector2f(BoxTopLeft.X, BoxTopLeft.Y))),
				WhiteBrush,
				ESlateDrawEffect::None,
				NodeColor
			);

			// Border
			float BorderThickness = 2.f;
			FLinearColor BorderColor = Node.bCanAllocate
				? FLinearColor(0.4f, 0.8f, 1.f, 0.6f)
				: FLinearColor(0.4f, 0.4f, 0.4f, 0.4f);

			// Top
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1,
				AllottedGeometry.ToPaintGeometry(FVector2f(BoxSize.X, BorderThickness),
					FSlateLayoutTransform(FVector2f(BoxTopLeft.X, BoxTopLeft.Y))),
				WhiteBrush, ESlateDrawEffect::None, BorderColor);
			// Bottom
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1,
				AllottedGeometry.ToPaintGeometry(FVector2f(BoxSize.X, BorderThickness),
					FSlateLayoutTransform(FVector2f(BoxTopLeft.X, BoxTopLeft.Y + BoxSize.Y - BorderThickness))),
				WhiteBrush, ESlateDrawEffect::None, BorderColor);
			// Left
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1,
				AllottedGeometry.ToPaintGeometry(FVector2f(BorderThickness, BoxSize.Y),
					FSlateLayoutTransform(FVector2f(BoxTopLeft.X, BoxTopLeft.Y))),
				WhiteBrush, ESlateDrawEffect::None, BorderColor);
			// Right
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1,
				AllottedGeometry.ToPaintGeometry(FVector2f(BorderThickness, BoxSize.Y),
					FSlateLayoutTransform(FVector2f(BoxTopLeft.X + BoxSize.X - BorderThickness, BoxTopLeft.Y))),
				WhiteBrush, ESlateDrawEffect::None, BorderColor);

			// Rank text inside node (for multi-rank)
			if (Node.MaxRank > 1)
			{
				FString RankText = FString::Printf(TEXT("%d/%d"), Node.AllocatedRank, Node.MaxRank);
				FSlateDrawElement::MakeText(
					OutDrawElements,
					LayerId + 2,
					AllottedGeometry.ToPaintGeometry(FVector2f(NodeRadius * 2.f, 16.f),
						FSlateLayoutTransform(FVector2f(NodeScreen.X - NodeRadius, NodeScreen.Y - 8.f))),
					RankText,
					RankFont,
					ESlateDrawEffect::None,
					FLinearColor::White
				);
			}

			// Node name below
			FString NameStr = Node.DisplayName.ToString();
			FSlateDrawElement::MakeText(
				OutDrawElements,
				LayerId + 2,
				AllottedGeometry.ToPaintGeometry(FVector2f(120.f, 16.f),
					FSlateLayoutTransform(FVector2f(NodeScreen.X - 60.f, NodeScreen.Y + NodeRadius + 4.f))),
				NameStr,
				NodeFont,
				ESlateDrawEffect::None,
				FLinearColor(0.8f, 0.8f, 0.8f)
			);
		}

		return LayerId + 3;
	}

private:
	FVector2D GridToScreen(float GridX, float GridY, const FVector2D& Center, float GridScale) const
	{
		return Center + FVector2D(GridX, GridY) * GridScale * ZoomLevel + PanOffset;
	}

	int32 HitTestNode(const FVector2D& LocalPos, const FGeometry& Geometry) const
	{
		FVector2D Center = Geometry.GetLocalSize() * 0.5f;
		const float GridScale = 80.f;
		const float NodeRadius = 24.f;
		const float HitRadiusSq = NodeRadius * NodeRadius;

		for (int32 i = 0; i < Nodes.Num(); ++i)
		{
			FVector2D NodeScreen = GridToScreen(Nodes[i].TreeX, Nodes[i].TreeY, Center, GridScale);
			if (FVector2D::DistSquared(LocalPos, NodeScreen) <= HitRadiusSq)
			{
				return i;
			}
		}
		return INDEX_NONE;
	}

	const FSkillNodeVisual* FindNodeByTag(const FGameplayTag& Tag) const
	{
		for (const FSkillNodeVisual& N : Nodes)
		{
			if (N.NodeTag == Tag)
			{
				return &N;
			}
		}
		return nullptr;
	}

	TArray<FSkillNodeVisual> Nodes;
	TWeakObjectPtr<UAtomProgressionComponent> ProgComp;
	FVector2D PanOffset;
	FVector2D PanStart;
	FVector2D DragStart;
	float ZoomLevel;
	bool bIsDragging;
};

// ── Outer Shell Widget ───────────────────────────────────────────

class SOutlawDemoSkillTreeSlate : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SOutlawDemoSkillTreeSlate) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		FSlateFontInfo HeaderFont = FCoreStyle::GetDefaultFontStyle("Bold", 14);
		FSlateFontInfo FooterFont = FCoreStyle::GetDefaultFontStyle("Regular", 10);

		ChildSlot
		[
			SNew(SOverlay)

			// Dark backdrop
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.ColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.8f))
			]

			// Content
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)

				// Header
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(16.f, 10.f, 16.f, 4.f)
				[
					SAssignNew(HeaderLabel, STextBlock)
					.Text(FText::FromString(TEXT("SKILL TREE")))
					.Font(HeaderFont)
					.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.7f, 0.3f)))
				]

				// Canvas
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SAssignNew(Canvas, SOutlawDemoSkillTreeCanvas)
				]

				// Footer
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(16.f, 4.f, 16.f, 10.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("LClick: Allocate  |  RClick: Deallocate  |  Drag: Pan  |  Scroll: Zoom")))
					.Font(FooterFont)
					.ColorAndOpacity(FSlateColor(FLinearColor(0.45f, 0.45f, 0.45f)))
				]
			]
		];
	}

	void Update(UAtomProgressionComponent* ProgComp)
	{
		if (!ProgComp || !Canvas.IsValid())
		{
			return;
		}

		Canvas->SetProgressionComponent(ProgComp);

		UAtomClassDefinition* ActiveClass = ProgComp->GetActiveClassDefinition();
		if (!ActiveClass)
		{
			return;
		}

		// Build visual nodes
		TArray<FSkillNodeVisual> VisualNodes;
		for (UAtomSkillTreeNodeDefinition* NodeDef : ActiveClass->SkillTreeNodes)
		{
			if (!NodeDef)
			{
				continue;
			}

			FSkillNodeVisual V;
			V.NodeTag = NodeDef->NodeTag;
			V.DisplayName = NodeDef->DisplayName;
			V.TreeX = NodeDef->TreePositionX;
			V.TreeY = NodeDef->TreePositionY;
			V.MaxRank = NodeDef->MaxRank;
			V.AllocatedRank = ProgComp->GetAllocatedRank(NodeDef->NodeTag);
			V.bCanAllocate = ProgComp->CanAllocateNode(NodeDef->NodeTag);

			for (const FAtomSkillNodePrerequisite& Prereq : NodeDef->Prerequisites)
			{
				V.PrerequisiteTags.Add(Prereq.RequiredNodeTag);
			}

			VisualNodes.Add(V);
		}

		Canvas->SetNodes(VisualNodes);

		// Update header
		if (HeaderLabel.IsValid())
		{
			FString ClassName = ActiveClass->DisplayName.ToString();
			HeaderLabel->SetText(FText::FromString(FString::Printf(
				TEXT("SKILL TREE  |  Level: %d  |  Points: %d  |  Class: %s"),
				ProgComp->GetCurrentLevel(),
				ProgComp->GetAvailableSkillPoints(),
				*ClassName)));
		}
	}

private:
	TSharedPtr<SOutlawDemoSkillTreeCanvas> Canvas;
	TSharedPtr<STextBlock> HeaderLabel;
};

// ── UOutlawDemoSkillTree ─────────────────────────────────────────

UOutlawDemoSkillTree::UOutlawDemoSkillTree(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UOutlawDemoSkillTree::RebuildWidget()
{
	SlateWidget = SNew(SOutlawDemoSkillTreeSlate);
	return SlateWidget.ToSharedRef();
}

void UOutlawDemoSkillTree::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
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

	UAtomProgressionComponent* ProgComp = Pawn->FindComponentByClass<UAtomProgressionComponent>();
	SlateWidget->Update(ProgComp);
}
