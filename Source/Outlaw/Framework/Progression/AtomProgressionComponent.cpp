// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawProgressionComponent.h"
#include "OutlawLevelingConfig.h"
#include "OutlawClassDefinition.h"
#include "OutlawSkillTreeNodeDefinition.h"
#include "AbilitySystem/OutlawAbilitySet.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogOutlawProgression, Log, All);

UOutlawProgressionComponent::UOutlawProgressionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UOutlawProgressionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UOutlawProgressionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UOutlawProgressionComponent, CurrentLevel);
	DOREPLIFETIME(UOutlawProgressionComponent, CurrentXP);
	DOREPLIFETIME(UOutlawProgressionComponent, AvailableSkillPoints);
	DOREPLIFETIME(UOutlawProgressionComponent, SelectedClassTag);
	DOREPLIFETIME(UOutlawProgressionComponent, SelectedAscendancyTag);
	DOREPLIFETIME(UOutlawProgressionComponent, AllocatedNodes);
}

// ── Leveling API ────────────────────────────────────────────────

void UOutlawProgressionComponent::AwardXP(int32 Amount)
{
	if (!GetOwner()->HasAuthority() || Amount <= 0)
	{
		return;
	}

	const UOutlawLevelingConfig* Config = GetEffectiveLevelingConfig();
	if (!Config)
	{
		UE_LOG(LogOutlawProgression, Warning, TEXT("AwardXP: No leveling config available."));
		return;
	}

	CurrentXP += Amount;
	OnXPChanged.Broadcast(CurrentXP, Amount);

	// Process level-ups
	const int32 MaxLevel = Config->GetMaxLevel();
	while (CurrentLevel < MaxLevel)
	{
		const int32 XPForNextLevel = Config->GetXPForLevel(CurrentLevel + 1);
		if (CurrentXP < XPForNextLevel)
		{
			break;
		}

		CurrentLevel++;
		const int32 PointsAwarded = Config->GetSkillPointsForLevel(CurrentLevel);
		AvailableSkillPoints += PointsAwarded;

		OnPlayerLeveledUp.Broadcast(CurrentLevel, PointsAwarded);

		// Process auto-unlock nodes for the new level
		ProcessAutoUnlockNodes();
	}

	// Recalculate stat bases after potential level-ups
	RecalculateStatBases();
}

int32 UOutlawProgressionComponent::GetXPToNextLevel() const
{
	const UOutlawLevelingConfig* Config = GetEffectiveLevelingConfig();
	if (!Config)
	{
		return 0;
	}

	const int32 MaxLevel = Config->GetMaxLevel();
	if (CurrentLevel >= MaxLevel)
	{
		return 0;
	}

	const int32 XPForNext = Config->GetXPForLevel(CurrentLevel + 1);
	return FMath::Max(0, XPForNext - CurrentXP);
}

float UOutlawProgressionComponent::GetXPProgress() const
{
	const UOutlawLevelingConfig* Config = GetEffectiveLevelingConfig();
	if (!Config)
	{
		return 0.0f;
	}

	const int32 MaxLevel = Config->GetMaxLevel();
	if (CurrentLevel >= MaxLevel)
	{
		return 1.0f;
	}

	const int32 XPForCurrent = Config->GetXPForLevel(CurrentLevel);
	const int32 XPForNext = Config->GetXPForLevel(CurrentLevel + 1);
	const int32 LevelXPRange = XPForNext - XPForCurrent;

	if (LevelXPRange <= 0)
	{
		return 1.0f;
	}

	return FMath::Clamp(static_cast<float>(CurrentXP - XPForCurrent) / static_cast<float>(LevelXPRange), 0.0f, 1.0f);
}

// ── Class API ───────────────────────────────────────────────────

void UOutlawProgressionComponent::SelectClass(FGameplayTag ClassTag)
{
	if (!GetOwner()->HasAuthority() || !ClassTag.IsValid())
	{
		return;
	}

	// Find the class definition
	UOutlawClassDefinition* NewClass = nullptr;
	for (const TObjectPtr<UOutlawClassDefinition>& ClassDef : AvailableClasses)
	{
		if (ClassDef && ClassDef->ClassTag == ClassTag)
		{
			NewClass = ClassDef;
			break;
		}
	}

	if (!NewClass)
	{
		UE_LOG(LogOutlawProgression, Warning, TEXT("SelectClass: Class tag '%s' not found in AvailableClasses."), *ClassTag.ToString());
		return;
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		UE_LOG(LogOutlawProgression, Warning, TEXT("SelectClass: No ASC available."));
		return;
	}

	// Revoke old class if re-selecting
	if (SelectedClassTag.IsValid())
	{
		ClassAbilityHandles.RevokeFromASC(ASC);

		// Revoke ascendancy too
		if (SelectedAscendancyTag.IsValid())
		{
			AscendancyAbilityHandles.RevokeFromASC(ASC);
			SelectedAscendancyTag = FGameplayTag();
		}

		// Respec all nodes since the old tree is no longer valid
		RespecAllNodes();
	}

	SelectedClassTag = ClassTag;

	// Grant class ability set
	if (NewClass->ClassAbilitySet)
	{
		NewClass->ClassAbilitySet->GiveToAbilitySystem(ASC, GetOwner(), ClassAbilityHandles);
	}

	// Process auto-unlock nodes for current level
	ProcessAutoUnlockNodes();

	// Recalculate stats with new class growth table
	RecalculateStatBases();

	OnClassChanged.Broadcast(ClassTag);
}

void UOutlawProgressionComponent::SelectAscendancy(FGameplayTag AscendancyTag)
{
	if (!GetOwner()->HasAuthority() || !AscendancyTag.IsValid())
	{
		return;
	}

	// Must have a base class selected first
	UOutlawClassDefinition* BaseClass = GetSelectedClass();
	if (!BaseClass)
	{
		UE_LOG(LogOutlawProgression, Warning, TEXT("SelectAscendancy: No base class selected."));
		return;
	}

	// Check level requirement
	if (CurrentLevel < BaseClass->AscendancyRequiredLevel)
	{
		UE_LOG(LogOutlawProgression, Warning, TEXT("SelectAscendancy: Level %d < required %d."), CurrentLevel, BaseClass->AscendancyRequiredLevel);
		return;
	}

	// Find the ascendancy in the base class's available ascendancies
	UOutlawClassDefinition* NewAscendancy = nullptr;
	for (const TObjectPtr<UOutlawClassDefinition>& AscDef : BaseClass->AvailableAscendancies)
	{
		if (AscDef && AscDef->ClassTag == AscendancyTag)
		{
			NewAscendancy = AscDef;
			break;
		}
	}

	if (!NewAscendancy)
	{
		UE_LOG(LogOutlawProgression, Warning, TEXT("SelectAscendancy: Ascendancy '%s' not found in base class '%s'."),
			*AscendancyTag.ToString(), *BaseClass->ClassTag.ToString());
		return;
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	// Revoke old ascendancy if re-selecting
	if (SelectedAscendancyTag.IsValid())
	{
		AscendancyAbilityHandles.RevokeFromASC(ASC);

		// Respec ascendancy nodes (nodes from old ascendancy tree)
		UOutlawClassDefinition* OldAscendancy = GetSelectedAscendancy();
		if (OldAscendancy)
		{
			for (const TObjectPtr<UOutlawSkillTreeNodeDefinition>& Node : OldAscendancy->SkillTreeNodes)
			{
				if (Node && GetAllocatedRank(Node->NodeTag) > 0)
				{
					RevokeNodeAbilities(Node->NodeTag);
					// Find and remove the allocation entry
					for (int32 i = AllocatedNodes.Num() - 1; i >= 0; --i)
					{
						if (AllocatedNodes[i].NodeTag == Node->NodeTag)
						{
							AvailableSkillPoints += Node->PointCostPerRank * AllocatedNodes[i].AllocatedRank;
							AllocatedNodes.RemoveAt(i);
							break;
						}
					}
				}
			}
		}
	}

	SelectedAscendancyTag = AscendancyTag;

	// Grant ascendancy ability set
	if (NewAscendancy->ClassAbilitySet)
	{
		NewAscendancy->ClassAbilitySet->GiveToAbilitySystem(ASC, GetOwner(), AscendancyAbilityHandles);
	}

	// Process auto-unlock nodes for ascendancy tree
	ProcessAutoUnlockNodes();

	RecalculateStatBases();

	OnAscendancySelected.Broadcast(AscendancyTag);
}

UOutlawClassDefinition* UOutlawProgressionComponent::GetSelectedClass() const
{
	if (!SelectedClassTag.IsValid())
	{
		return nullptr;
	}

	for (const TObjectPtr<UOutlawClassDefinition>& ClassDef : AvailableClasses)
	{
		if (ClassDef && ClassDef->ClassTag == SelectedClassTag)
		{
			return ClassDef;
		}
	}
	return nullptr;
}

UOutlawClassDefinition* UOutlawProgressionComponent::GetSelectedAscendancy() const
{
	if (!SelectedAscendancyTag.IsValid())
	{
		return nullptr;
	}

	UOutlawClassDefinition* BaseClass = GetSelectedClass();
	if (!BaseClass)
	{
		return nullptr;
	}

	for (const TObjectPtr<UOutlawClassDefinition>& AscDef : BaseClass->AvailableAscendancies)
	{
		if (AscDef && AscDef->ClassTag == SelectedAscendancyTag)
		{
			return AscDef;
		}
	}
	return nullptr;
}

UOutlawClassDefinition* UOutlawProgressionComponent::GetActiveClassDefinition() const
{
	UOutlawClassDefinition* Ascendancy = GetSelectedAscendancy();
	return Ascendancy ? Ascendancy : GetSelectedClass();
}

// ── Skill Tree API ──────────────────────────────────────────────

void UOutlawProgressionComponent::AllocateSkillNode(FGameplayTag NodeTag)
{
	if (!GetOwner()->HasAuthority() || !NodeTag.IsValid())
	{
		return;
	}

	if (!CanAllocateNode(NodeTag))
	{
		UE_LOG(LogOutlawProgression, Warning, TEXT("AllocateSkillNode: Cannot allocate '%s'."), *NodeTag.ToString());
		return;
	}

	UOutlawSkillTreeNodeDefinition* NodeDef = FindNodeDefinition(NodeTag);
	if (!NodeDef)
	{
		return;
	}

	FOutlawAllocatedSkillNode& Entry = FindOrAddAllocatedNode(NodeTag);
	Entry.AllocatedRank++;

	// Deduct skill points (auto-unlock nodes are free)
	if (NodeDef->UnlockType == EOutlawSkillNodeUnlockType::Manual)
	{
		AvailableSkillPoints -= NodeDef->PointCostPerRank;
	}

	// Grant abilities for the new rank
	GrantNodeAbilities(NodeTag, Entry.AllocatedRank);

	// Recalculate stats to include node stat bonuses
	RecalculateStatBases();

	OnSkillNodeAllocated.Broadcast(NodeTag, Entry.AllocatedRank);
}

void UOutlawProgressionComponent::DeallocateSkillNode(FGameplayTag NodeTag)
{
	if (!GetOwner()->HasAuthority() || !NodeTag.IsValid())
	{
		return;
	}

	const int32 CurrentRank = GetAllocatedRank(NodeTag);
	if (CurrentRank <= 0)
	{
		UE_LOG(LogOutlawProgression, Warning, TEXT("DeallocateSkillNode: '%s' is not allocated."), *NodeTag.ToString());
		return;
	}

	UOutlawSkillTreeNodeDefinition* NodeDef = FindNodeDefinition(NodeTag);
	if (!NodeDef)
	{
		return;
	}

	// Cannot deallocate auto-unlock nodes
	if (NodeDef->UnlockType == EOutlawSkillNodeUnlockType::AutoOnLevel)
	{
		UE_LOG(LogOutlawProgression, Warning, TEXT("DeallocateSkillNode: Cannot deallocate auto-unlock node '%s'."), *NodeTag.ToString());
		return;
	}

	// Check if any other node depends on this rank
	if (IsNodeRequiredByOthers(NodeTag, CurrentRank))
	{
		UE_LOG(LogOutlawProgression, Warning, TEXT("DeallocateSkillNode: '%s' at rank %d is required by other nodes."), *NodeTag.ToString(), CurrentRank);
		return;
	}

	// Revoke current abilities
	RevokeNodeAbilities(NodeTag);

	// Decrement rank
	for (FOutlawAllocatedSkillNode& Entry : AllocatedNodes)
	{
		if (Entry.NodeTag == NodeTag)
		{
			Entry.AllocatedRank--;

			// Refund points
			AvailableSkillPoints += NodeDef->PointCostPerRank;

			const int32 NewRank = Entry.AllocatedRank;

			// Re-grant abilities for the lower rank if still allocated
			if (NewRank > 0)
			{
				GrantNodeAbilities(NodeTag, NewRank);
			}
			else
			{
				// Remove the entry entirely if rank dropped to 0
				AllocatedNodes.RemoveAll([&NodeTag](const FOutlawAllocatedSkillNode& N) { return N.NodeTag == NodeTag; });
			}

			RecalculateStatBases();
			OnSkillNodeDeallocated.Broadcast(NodeTag, NewRank);
			return;
		}
	}
}

void UOutlawProgressionComponent::RespecAllNodes()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetASC();

	for (const FOutlawAllocatedSkillNode& Entry : AllocatedNodes)
	{
		UOutlawSkillTreeNodeDefinition* NodeDef = FindNodeDefinition(Entry.NodeTag);

		// Revoke abilities
		if (ASC)
		{
			FOutlawAbilitySetGrantedHandles* Handles = NodeAbilityHandles.Find(Entry.NodeTag);
			if (Handles)
			{
				Handles->RevokeFromASC(ASC);
			}
		}

		// Refund points (only manual nodes cost points)
		if (NodeDef && NodeDef->UnlockType == EOutlawSkillNodeUnlockType::Manual)
		{
			AvailableSkillPoints += NodeDef->PointCostPerRank * Entry.AllocatedRank;
		}
	}

	AllocatedNodes.Empty();
	NodeAbilityHandles.Empty();

	// Re-process auto-unlock nodes since they should remain active
	ProcessAutoUnlockNodes();

	RecalculateStatBases();
}

int32 UOutlawProgressionComponent::GetAllocatedRank(FGameplayTag NodeTag) const
{
	for (const FOutlawAllocatedSkillNode& Entry : AllocatedNodes)
	{
		if (Entry.NodeTag == NodeTag)
		{
			return Entry.AllocatedRank;
		}
	}
	return 0;
}

bool UOutlawProgressionComponent::CanAllocateNode(FGameplayTag NodeTag) const
{
	if (!NodeTag.IsValid())
	{
		return false;
	}

	UOutlawSkillTreeNodeDefinition* NodeDef = FindNodeDefinition(NodeTag);
	if (!NodeDef)
	{
		return false;
	}

	const int32 CurrentRank = GetAllocatedRank(NodeTag);

	// Already at max rank?
	if (CurrentRank >= NodeDef->MaxRank)
	{
		return false;
	}

	// Level requirement
	if (CurrentLevel < NodeDef->RequiredLevel)
	{
		return false;
	}

	// Skill points (manual nodes only)
	if (NodeDef->UnlockType == EOutlawSkillNodeUnlockType::Manual && AvailableSkillPoints < NodeDef->PointCostPerRank)
	{
		return false;
	}

	// Prerequisites
	for (const FOutlawSkillNodePrerequisite& Prereq : NodeDef->Prerequisites)
	{
		if (GetAllocatedRank(Prereq.RequiredNodeTag) < Prereq.RequiredRank)
		{
			return false;
		}
	}

	return true;
}

TArray<FGameplayTag> UOutlawProgressionComponent::GetAllocableNodes() const
{
	TArray<FGameplayTag> Result;

	// Gather nodes from both base class and ascendancy
	TArray<UOutlawSkillTreeNodeDefinition*> AllNodes;

	UOutlawClassDefinition* BaseClass = GetSelectedClass();
	if (BaseClass)
	{
		for (const TObjectPtr<UOutlawSkillTreeNodeDefinition>& Node : BaseClass->SkillTreeNodes)
		{
			if (Node)
			{
				AllNodes.Add(Node);
			}
		}
	}

	UOutlawClassDefinition* Ascendancy = GetSelectedAscendancy();
	if (Ascendancy)
	{
		for (const TObjectPtr<UOutlawSkillTreeNodeDefinition>& Node : Ascendancy->SkillTreeNodes)
		{
			if (Node)
			{
				AllNodes.Add(Node);
			}
		}
	}

	for (const UOutlawSkillTreeNodeDefinition* Node : AllNodes)
	{
		if (CanAllocateNode(Node->NodeTag))
		{
			Result.Add(Node->NodeTag);
		}
	}

	return Result;
}

// ── Save/Load API ───────────────────────────────────────────────

FOutlawProgressionSaveData UOutlawProgressionComponent::SaveProgression() const
{
	FOutlawProgressionSaveData Data;
	Data.CurrentLevel = CurrentLevel;
	Data.CurrentXP = CurrentXP;
	Data.AvailableSkillPoints = AvailableSkillPoints;
	Data.SelectedClassTag = SelectedClassTag;
	Data.SelectedAscendancyTag = SelectedAscendancyTag;
	Data.AllocatedNodes = AllocatedNodes;
	return Data;
}

void UOutlawProgressionComponent::LoadProgression(const FOutlawProgressionSaveData& Data)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		UE_LOG(LogOutlawProgression, Warning, TEXT("LoadProgression: No ASC available."));
		return;
	}

	// Clear existing state
	// Revoke all node abilities
	for (auto& Pair : NodeAbilityHandles)
	{
		Pair.Value.RevokeFromASC(ASC);
	}
	NodeAbilityHandles.Empty();
	AllocatedNodes.Empty();

	// Revoke class abilities
	ClassAbilityHandles.RevokeFromASC(ASC);
	AscendancyAbilityHandles.RevokeFromASC(ASC);

	// Restore basic state
	CurrentLevel = Data.CurrentLevel;
	CurrentXP = Data.CurrentXP;
	AvailableSkillPoints = Data.AvailableSkillPoints;
	SelectedClassTag = FGameplayTag();
	SelectedAscendancyTag = FGameplayTag();

	// Re-select class (grants class abilities)
	if (Data.SelectedClassTag.IsValid())
	{
		SelectClass(Data.SelectedClassTag);
	}

	// Re-select ascendancy
	if (Data.SelectedAscendancyTag.IsValid())
	{
		SelectAscendancy(Data.SelectedAscendancyTag);
	}

	// Restore skill point count before re-allocating
	// (SelectClass may have triggered auto-unlocks and respec; we need the exact saved count
	//  minus the points that will be spent re-allocating)
	// First, calculate total points that auto-unlock nodes would have consumed — they're free, so 0.
	// We restore the exact saved skill points, then re-allocate manual nodes.
	AvailableSkillPoints = Data.AvailableSkillPoints;

	// Respec any auto-allocated nodes from SelectClass so we can rebuild from saved state
	for (int32 i = AllocatedNodes.Num() - 1; i >= 0; --i)
	{
		RevokeNodeAbilities(AllocatedNodes[i].NodeTag);
	}
	AllocatedNodes.Empty();
	NodeAbilityHandles.Empty();

	// Re-allocate saved nodes in order
	for (const FOutlawAllocatedSkillNode& SavedNode : Data.AllocatedNodes)
	{
		UOutlawSkillTreeNodeDefinition* NodeDef = FindNodeDefinition(SavedNode.NodeTag);
		if (!NodeDef)
		{
			continue;
		}

		FOutlawAllocatedSkillNode& Entry = FindOrAddAllocatedNode(SavedNode.NodeTag);
		for (int32 Rank = 1; Rank <= SavedNode.AllocatedRank; ++Rank)
		{
			Entry.AllocatedRank = Rank;
			GrantNodeAbilities(SavedNode.NodeTag, Rank);
		}
	}

	// Final recalc
	RecalculateStatBases();
}

// ── Private Helpers ─────────────────────────────────────────────

UAbilitySystemComponent* UOutlawProgressionComponent::GetASC() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		return ASI->GetAbilitySystemComponent();
	}

	if (const APawn* Pawn = Cast<APawn>(Owner))
	{
		if (const APlayerState* PS = Pawn->GetPlayerState())
		{
			if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PS))
			{
				return ASI->GetAbilitySystemComponent();
			}
		}
	}

	return nullptr;
}

UOutlawLevelingConfig* UOutlawProgressionComponent::GetEffectiveLevelingConfig() const
{
	UOutlawClassDefinition* ActiveClass = GetSelectedClass();
	if (ActiveClass && ActiveClass->LevelingConfig)
	{
		return ActiveClass->LevelingConfig;
	}
	return DefaultLevelingConfig;
}

void UOutlawProgressionComponent::RecalculateStatBases()
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	// Collect all stat contributions: class growth + ascendancy growth + node bonuses
	TMap<FGameplayAttribute, float> AttributeTotals;

	// Class stat growth (base class)
	UOutlawClassDefinition* BaseClass = GetSelectedClass();
	if (BaseClass)
	{
		for (const FOutlawStatGrowthEntry& Entry : BaseClass->StatGrowthTable)
		{
			if (Entry.Attribute.IsValid())
			{
				AttributeTotals.FindOrAdd(Entry.Attribute) += Entry.ValuePerLevel * static_cast<float>(CurrentLevel);
			}
		}
	}

	// Ascendancy stat growth
	UOutlawClassDefinition* Ascendancy = GetSelectedAscendancy();
	if (Ascendancy)
	{
		for (const FOutlawStatGrowthEntry& Entry : Ascendancy->StatGrowthTable)
		{
			if (Entry.Attribute.IsValid())
			{
				AttributeTotals.FindOrAdd(Entry.Attribute) += Entry.ValuePerLevel * static_cast<float>(CurrentLevel);
			}
		}
	}

	// Node stat bonuses
	for (const FOutlawAllocatedSkillNode& AllocNode : AllocatedNodes)
	{
		UOutlawSkillTreeNodeDefinition* NodeDef = FindNodeDefinition(AllocNode.NodeTag);
		if (!NodeDef)
		{
			continue;
		}

		for (const FOutlawStatGrowthEntry& Bonus : NodeDef->StatBonusesPerRank)
		{
			if (Bonus.Attribute.IsValid())
			{
				AttributeTotals.FindOrAdd(Bonus.Attribute) += Bonus.ValuePerLevel * static_cast<float>(AllocNode.AllocatedRank);
			}
		}
	}

	// Apply all calculated base values to the ASC
	for (const auto& Pair : AttributeTotals)
	{
		ASC->SetNumericAttributeBase(Pair.Key, Pair.Value);
	}
}

void UOutlawProgressionComponent::ProcessAutoUnlockNodes()
{
	// Gather all nodes from both base class and ascendancy
	TArray<UOutlawSkillTreeNodeDefinition*> AllNodes;

	UOutlawClassDefinition* BaseClass = GetSelectedClass();
	if (BaseClass)
	{
		for (const TObjectPtr<UOutlawSkillTreeNodeDefinition>& Node : BaseClass->SkillTreeNodes)
		{
			if (Node)
			{
				AllNodes.Add(Node);
			}
		}
	}

	UOutlawClassDefinition* Ascendancy = GetSelectedAscendancy();
	if (Ascendancy)
	{
		for (const TObjectPtr<UOutlawSkillTreeNodeDefinition>& Node : Ascendancy->SkillTreeNodes)
		{
			if (Node)
			{
				AllNodes.Add(Node);
			}
		}
	}

	for (UOutlawSkillTreeNodeDefinition* Node : AllNodes)
	{
		if (Node->UnlockType != EOutlawSkillNodeUnlockType::AutoOnLevel)
		{
			continue;
		}

		if (Node->AutoUnlockLevel <= 0 || CurrentLevel < Node->AutoUnlockLevel)
		{
			continue;
		}

		// Already allocated?
		const int32 CurrentRank = GetAllocatedRank(Node->NodeTag);
		if (CurrentRank >= Node->MaxRank)
		{
			continue;
		}

		// Auto-allocate to max rank
		for (int32 Rank = CurrentRank + 1; Rank <= Node->MaxRank; ++Rank)
		{
			FOutlawAllocatedSkillNode& Entry = FindOrAddAllocatedNode(Node->NodeTag);
			Entry.AllocatedRank = Rank;
			GrantNodeAbilities(Node->NodeTag, Rank);
			OnSkillNodeAllocated.Broadcast(Node->NodeTag, Rank);
		}
	}
}

void UOutlawProgressionComponent::GrantNodeAbilities(FGameplayTag NodeTag, int32 Rank)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	UOutlawSkillTreeNodeDefinition* NodeDef = FindNodeDefinition(NodeTag);
	if (!NodeDef)
	{
		return;
	}

	// Revoke previous rank's abilities (we'll re-grant the correct set for the new rank)
	FOutlawAbilitySetGrantedHandles* ExistingHandles = NodeAbilityHandles.Find(NodeTag);
	if (ExistingHandles)
	{
		ExistingHandles->RevokeFromASC(ASC);
		NodeAbilityHandles.Remove(NodeTag);
	}

	UOutlawAbilitySet* AbilitySet = NodeDef->GetAbilitySetForRank(Rank);
	if (AbilitySet)
	{
		FOutlawAbilitySetGrantedHandles& NewHandles = NodeAbilityHandles.Add(NodeTag);
		AbilitySet->GiveToAbilitySystem(ASC, GetOwner(), NewHandles);
	}
}

void UOutlawProgressionComponent::RevokeNodeAbilities(FGameplayTag NodeTag)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	FOutlawAbilitySetGrantedHandles* Handles = NodeAbilityHandles.Find(NodeTag);
	if (Handles)
	{
		Handles->RevokeFromASC(ASC);
		NodeAbilityHandles.Remove(NodeTag);
	}
}

UOutlawSkillTreeNodeDefinition* UOutlawProgressionComponent::FindNodeDefinition(FGameplayTag NodeTag) const
{
	// Search base class tree
	UOutlawClassDefinition* BaseClass = GetSelectedClass();
	if (BaseClass)
	{
		UOutlawSkillTreeNodeDefinition* Node = BaseClass->FindSkillNode(NodeTag);
		if (Node)
		{
			return Node;
		}
	}

	// Search ascendancy tree
	UOutlawClassDefinition* Ascendancy = GetSelectedAscendancy();
	if (Ascendancy)
	{
		UOutlawSkillTreeNodeDefinition* Node = Ascendancy->FindSkillNode(NodeTag);
		if (Node)
		{
			return Node;
		}
	}

	return nullptr;
}

FOutlawAllocatedSkillNode& UOutlawProgressionComponent::FindOrAddAllocatedNode(FGameplayTag NodeTag)
{
	for (FOutlawAllocatedSkillNode& Entry : AllocatedNodes)
	{
		if (Entry.NodeTag == NodeTag)
		{
			return Entry;
		}
	}

	FOutlawAllocatedSkillNode NewEntry;
	NewEntry.NodeTag = NodeTag;
	NewEntry.AllocatedRank = 0;
	AllocatedNodes.Add(NewEntry);
	return AllocatedNodes.Last();
}

bool UOutlawProgressionComponent::IsNodeRequiredByOthers(FGameplayTag NodeTag, int32 AtRank) const
{
	// Check all allocated nodes to see if any has a prerequisite on this node at this rank
	for (const FOutlawAllocatedSkillNode& AllocNode : AllocatedNodes)
	{
		if (AllocNode.NodeTag == NodeTag || AllocNode.AllocatedRank <= 0)
		{
			continue;
		}

		UOutlawSkillTreeNodeDefinition* NodeDef = FindNodeDefinition(AllocNode.NodeTag);
		if (!NodeDef)
		{
			continue;
		}

		for (const FOutlawSkillNodePrerequisite& Prereq : NodeDef->Prerequisites)
		{
			if (Prereq.RequiredNodeTag == NodeTag && Prereq.RequiredRank >= AtRank)
			{
				return true;
			}
		}
	}

	return false;
}
