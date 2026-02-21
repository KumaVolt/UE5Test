// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomInventoryComponent.h"
#include "AtomItemDefinition.h"
#include "AtomItemInstance.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystem/AtomAbilitySet.h"
#include "Weapon/AtomWeaponManagerComponent.h"
#include "Weapon/AtomShooterWeaponData.h"
#include "Weapon/AtomARPGWeaponData.h"
#include "Weapon/AtomAffixDefinition.h"
#include "Weapon/AtomSkillGemDefinition.h"
#include "Weapon/AtomWeaponModDefinition.h"
#include "Weapon/AtomWeaponTypes.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtomInventory, Log, All);

// ════════════════════════════════════════════════════════════════
// FAtomInventoryEntry — FFastArraySerializerItem callbacks
// ════════════════════════════════════════════════════════════════

void FAtomInventoryEntry::PreReplicatedRemove(const FAtomInventoryList& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent)
	{
		InArraySerializer.OwnerComponent->BroadcastInventoryChanged();
	}
}

void FAtomInventoryEntry::PostReplicatedAdd(const FAtomInventoryList& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent)
	{
		InArraySerializer.OwnerComponent->BroadcastInventoryChanged();
	}
}

void FAtomInventoryEntry::PostReplicatedChange(const FAtomInventoryList& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent)
	{
		InArraySerializer.OwnerComponent->BroadcastInventoryChanged();
	}
}

// ════════════════════════════════════════════════════════════════
// FAtomInventoryList — Helpers
// ════════════════════════════════════════════════════════════════

int32 FAtomInventoryList::AddEntry(UAtomItemDefinition* ItemDef, int32 StackCount, int32 InstanceId, int32 GridX, int32 GridY)
{
	FAtomInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.ItemDef = ItemDef;
	NewEntry.StackCount = StackCount;
	NewEntry.InstanceId = InstanceId;
	NewEntry.GridX = GridX;
	NewEntry.GridY = GridY;

	MarkItemDirty(NewEntry);
	return Entries.Num() - 1;
}

bool FAtomInventoryList::RemoveEntry(int32 InstanceId)
{
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (Entries[i].InstanceId == InstanceId)
		{
			Entries.RemoveAt(i);
			MarkArrayDirty();
			return true;
		}
	}
	return false;
}

FAtomInventoryEntry* FAtomInventoryList::FindEntry(int32 InstanceId)
{
	for (FAtomInventoryEntry& Entry : Entries)
	{
		if (Entry.InstanceId == InstanceId)
		{
			return &Entry;
		}
	}
	return nullptr;
}

const FAtomInventoryEntry* FAtomInventoryList::FindEntry(int32 InstanceId) const
{
	for (const FAtomInventoryEntry& Entry : Entries)
	{
		if (Entry.InstanceId == InstanceId)
		{
			return &Entry;
		}
	}
	return nullptr;
}

// ════════════════════════════════════════════════════════════════
// UAtomInventoryComponent
// ════════════════════════════════════════════════════════════════

UAtomInventoryComponent::UAtomInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
{
	SetIsReplicatedByDefault(true);
}

void UAtomInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsGridMode())
	{
		RebuildOccupancyGrid();
	}
}

void UAtomInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAtomInventoryComponent, InventoryList);
	DOREPLIFETIME(UAtomInventoryComponent, EquipmentSlots);
}

// ── Core Inventory ──────────────────────────────────────────────

int32 UAtomInventoryComponent::AddItem(UAtomItemDefinition* ItemDef, int32 Count)
{
	if (!ItemDef || Count <= 0)
	{
		return 0;
	}

	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogAtomInventory, Warning, TEXT("AddItem called on client. Item: %s"), *ItemDef->DisplayName.ToString());
		return 0;
	}

	int32 Remaining = Count;

	// First pass: fill existing stacks
	if (ItemDef->MaxStackSize > 1)
	{
		for (FAtomInventoryEntry& Entry : InventoryList.Entries)
		{
			if (Remaining <= 0)
			{
				break;
			}

			if (Entry.ItemDef == ItemDef && Entry.StackCount < ItemDef->MaxStackSize)
			{
				const int32 SpaceInStack = ItemDef->MaxStackSize - Entry.StackCount;
				const int32 ToAdd = FMath::Min(Remaining, SpaceInStack);

				// Check weight
				const float WeightToAdd = ToAdd * ItemDef->Weight;
				if (GetCurrentWeight() + WeightToAdd > MaxWeight)
				{
					const int32 CanFitByWeight = (MaxWeight > 0.0f && ItemDef->Weight > 0.0f)
						? FMath::FloorToInt32((MaxWeight - GetCurrentWeight()) / ItemDef->Weight)
						: ToAdd;
					const int32 ActualAdd = FMath::Min(ToAdd, FMath::Max(0, CanFitByWeight));
					if (ActualAdd <= 0)
					{
						break;
					}
					Entry.StackCount += ActualAdd;
					Remaining -= ActualAdd;
					InventoryList.MarkItemDirty(Entry);
					break;
				}

				Entry.StackCount += ToAdd;
				Remaining -= ToAdd;
				InventoryList.MarkItemDirty(Entry);
			}
		}
	}

	// Second pass: create new stacks
	while (Remaining > 0)
	{
		const int32 StackSize = FMath::Min(Remaining, ItemDef->MaxStackSize);

		// Check weight
		const float WeightToAdd = StackSize * ItemDef->Weight;
		if (ItemDef->Weight > 0.0f && GetCurrentWeight() + WeightToAdd > MaxWeight)
		{
			const int32 CanFitByWeight = FMath::FloorToInt32((MaxWeight - GetCurrentWeight()) / ItemDef->Weight);
			if (CanFitByWeight <= 0)
			{
				break;
			}

			const int32 ActualStack = FMath::Min(StackSize, CanFitByWeight);

			if (IsGridMode())
			{
				int32 PlaceX, PlaceY;
				if (!FindFreeSpace(ItemDef, PlaceX, PlaceY))
				{
					break;
				}
				const int32 NewId = GenerateInstanceId();
				const int32 EntryIdx = InventoryList.AddEntry(ItemDef, ActualStack, NewId, PlaceX, PlaceY);
				SetOccupancy(PlaceX, PlaceY, ItemDef->GridWidth, ItemDef->GridHeight, NewId);
				if (ItemDef->IsWeapon())
				{
					InventoryList.Entries[EntryIdx].ItemInstance = CreateItemInstance(ItemDef, NewId);
				}
			}
			else
			{
				if (GetRemainingSlots() <= 0)
				{
					break;
				}
				const int32 NewId = GenerateInstanceId();
				const int32 EntryIdx = InventoryList.AddEntry(ItemDef, ActualStack, NewId);
				if (ItemDef->IsWeapon())
				{
					InventoryList.Entries[EntryIdx].ItemInstance = CreateItemInstance(ItemDef, NewId);
				}
			}

			Remaining -= ActualStack;
			break;
		}

		if (IsGridMode())
		{
			int32 PlaceX, PlaceY;
			if (!FindFreeSpace(ItemDef, PlaceX, PlaceY))
			{
				break;
			}
			const int32 NewId = GenerateInstanceId();
			const int32 EntryIdx = InventoryList.AddEntry(ItemDef, StackSize, NewId, PlaceX, PlaceY);
			SetOccupancy(PlaceX, PlaceY, ItemDef->GridWidth, ItemDef->GridHeight, NewId);
			if (ItemDef->IsWeapon())
			{
				InventoryList.Entries[EntryIdx].ItemInstance = CreateItemInstance(ItemDef, NewId);
			}
		}
		else
		{
			if (GetRemainingSlots() <= 0)
			{
				break;
			}
			const int32 NewId = GenerateInstanceId();
			const int32 EntryIdx = InventoryList.AddEntry(ItemDef, StackSize, NewId);
			if (ItemDef->IsWeapon())
			{
				InventoryList.Entries[EntryIdx].ItemInstance = CreateItemInstance(ItemDef, NewId);
			}
		}

		Remaining -= StackSize;
	}

	const int32 Added = Count - Remaining;
	if (Added > 0)
	{
		BroadcastInventoryChanged();
	}
	return Added;
}

bool UAtomInventoryComponent::RemoveItem(int32 InstanceId, int32 Count)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	FAtomInventoryEntry* Entry = InventoryList.FindEntry(InstanceId);
	if (!Entry)
	{
		return false;
	}

	if (Count >= Entry->StackCount)
	{
		// Clear grid occupancy before removing
		if (IsGridMode() && Entry->ItemDef && Entry->GridX != INDEX_NONE)
		{
			ClearOccupancy(Entry->GridX, Entry->GridY, Entry->ItemDef->GridWidth, Entry->ItemDef->GridHeight);
		}
		InventoryList.RemoveEntry(InstanceId);
	}
	else
	{
		Entry->StackCount -= Count;
		InventoryList.MarkItemDirty(*Entry);
	}

	BroadcastInventoryChanged();
	return true;
}

int32 UAtomInventoryComponent::RemoveItemByDef(UAtomItemDefinition* ItemDef, int32 Count)
{
	if (!ItemDef || Count <= 0 || !GetOwner()->HasAuthority())
	{
		return 0;
	}

	int32 Remaining = Count;

	// Iterate in reverse so removals don't invalidate indices
	for (int32 i = InventoryList.Entries.Num() - 1; i >= 0 && Remaining > 0; --i)
	{
		FAtomInventoryEntry& Entry = InventoryList.Entries[i];
		if (Entry.ItemDef != ItemDef)
		{
			continue;
		}

		if (Remaining >= Entry.StackCount)
		{
			Remaining -= Entry.StackCount;
			if (IsGridMode() && Entry.GridX != INDEX_NONE)
			{
				ClearOccupancy(Entry.GridX, Entry.GridY, Entry.ItemDef->GridWidth, Entry.ItemDef->GridHeight);
			}
			InventoryList.Entries.RemoveAt(i);
			InventoryList.MarkArrayDirty();
		}
		else
		{
			Entry.StackCount -= Remaining;
			Remaining = 0;
			InventoryList.MarkItemDirty(Entry);
		}
	}

	const int32 Removed = Count - Remaining;
	if (Removed > 0)
	{
		BroadcastInventoryChanged();
	}
	return Removed;
}

TArray<FAtomInventoryEntry> UAtomInventoryComponent::FindItemsByTag(FGameplayTag Tag) const
{
	TArray<FAtomInventoryEntry> Result;
	for (const FAtomInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef && Entry.ItemDef->ItemTags.HasTag(Tag))
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

int32 UAtomInventoryComponent::GetItemCount(const UAtomItemDefinition* ItemDef) const
{
	int32 Total = 0;
	for (const FAtomInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef == ItemDef)
		{
			Total += Entry.StackCount;
		}
	}
	return Total;
}

bool UAtomInventoryComponent::HasItem(const UAtomItemDefinition* ItemDef, int32 Count) const
{
	return GetItemCount(ItemDef) >= Count;
}

float UAtomInventoryComponent::GetCurrentWeight() const
{
	float Total = 0.0f;
	for (const FAtomInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef)
		{
			Total += Entry.ItemDef->Weight * Entry.StackCount;
		}
	}
	return Total;
}

int32 UAtomInventoryComponent::GetRemainingSlots() const
{
	if (IsGridMode())
	{
		int32 FreeCells = 0;
		for (int32 Cell : OccupancyGrid)
		{
			if (Cell == INDEX_NONE)
			{
				++FreeCells;
			}
		}
		return FreeCells;
	}
	return MaxSlots - InventoryList.Entries.Num();
}

TArray<FAtomInventoryEntry> UAtomInventoryComponent::FindItemsForSlot(FGameplayTag SlotTag) const
{
	TArray<FAtomInventoryEntry> Result;
	for (const FAtomInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef && Entry.ItemDef->bCanBeEquipped && Entry.ItemDef->EquipmentSlotTag == SlotTag)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

TArray<FAtomInventoryEntry> UAtomInventoryComponent::FindItemsByType(EAtomItemType ItemType) const
{
	TArray<FAtomInventoryEntry> Result;
	for (const FAtomInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef && Entry.ItemDef->ItemType == ItemType)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

TArray<FAtomInventoryEntry> UAtomInventoryComponent::FindItemsByRarity(EAtomItemRarity Rarity) const
{
	TArray<FAtomInventoryEntry> Result;
	for (const FAtomInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef && Entry.ItemDef->Rarity == Rarity)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

// ── Sorting ─────────────────────────────────────────────────────

void UAtomInventoryComponent::SortInventory(EAtomInventorySortMode SortMode, bool bDescending)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	InventoryList.Entries.Sort([SortMode, bDescending](const FAtomInventoryEntry& A, const FAtomInventoryEntry& B)
	{
		// Null-def entries sink to the bottom
		if (!A.ItemDef) return false;
		if (!B.ItemDef) return true;

		int32 Result = 0;

		switch (SortMode)
		{
		case EAtomInventorySortMode::ByName:
			Result = A.ItemDef->DisplayName.CompareTo(B.ItemDef->DisplayName);
			break;

		case EAtomInventorySortMode::ByRarity:
			Result = static_cast<int32>(A.ItemDef->Rarity) - static_cast<int32>(B.ItemDef->Rarity);
			break;

		case EAtomInventorySortMode::ByType:
			Result = static_cast<int32>(A.ItemDef->ItemType) - static_cast<int32>(B.ItemDef->ItemType);
			// Secondary sort by name within same type
			if (Result == 0)
			{
				Result = A.ItemDef->DisplayName.CompareTo(B.ItemDef->DisplayName);
			}
			break;

		case EAtomInventorySortMode::ByWeight:
			{
				const float WeightDiff = (A.ItemDef->Weight * A.StackCount) - (B.ItemDef->Weight * B.StackCount);
				Result = (WeightDiff > 0.0f) ? 1 : (WeightDiff < 0.0f) ? -1 : 0;
			}
			break;
		}

		return bDescending ? (Result > 0) : (Result < 0);
	});

	InventoryList.MarkArrayDirty();
	BroadcastInventoryChanged();
}

// ── Equipment ───────────────────────────────────────────────────

bool UAtomInventoryComponent::EquipItem(int32 InstanceId)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	FAtomInventoryEntry* Entry = InventoryList.FindEntry(InstanceId);
	if (!Entry || !Entry->ItemDef)
	{
		UE_LOG(LogAtomInventory, Warning, TEXT("EquipItem: Invalid InstanceId %d"), InstanceId);
		return false;
	}

	const UAtomItemDefinition* ItemDef = Entry->ItemDef;
	if (!ItemDef->bCanBeEquipped)
	{
		UE_LOG(LogAtomInventory, Warning, TEXT("EquipItem: Item '%s' cannot be equipped."), *ItemDef->DisplayName.ToString());
		return false;
	}

	FAtomEquipmentSlotInfo* Slot = FindEquipmentSlot(ItemDef->EquipmentSlotTag);
	if (!Slot)
	{
		UE_LOG(LogAtomInventory, Warning, TEXT("EquipItem: No equipment slot '%s' configured."), *ItemDef->EquipmentSlotTag.ToString());
		return false;
	}

	// Unequip current item in that slot if occupied
	if (Slot->EquippedItemInstanceId != INDEX_NONE)
	{
		UnequipItem(Slot->SlotTag);
	}

	Slot->EquippedItemInstanceId = InstanceId;

	// Grant ability set via ASC
	if (ItemDef->GrantedAbilitySet)
	{
		UAbilitySystemComponent* ASC = GetASC();
		if (ASC)
		{
			ItemDef->GrantedAbilitySet->GiveToAbilitySystem(ASC, GetOwner(), Slot->GrantedHandles);
		}
	}

	// Notify weapon manager if this is a weapon with an instance
	if (Entry->ItemInstance)
	{
		if (UAtomWeaponManagerComponent* WeaponMgr = GetWeaponManager())
		{
			WeaponMgr->OnWeaponEquipped(Entry->ItemInstance, Slot->SlotTag);
		}
	}

	OnItemEquipped.Broadcast(ItemDef, Slot->SlotTag);
	BroadcastInventoryChanged();
	return true;
}

bool UAtomInventoryComponent::UnequipItem(FGameplayTag SlotTag)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	FAtomEquipmentSlotInfo* Slot = FindEquipmentSlot(SlotTag);
	if (!Slot || Slot->EquippedItemInstanceId == INDEX_NONE)
	{
		return false;
	}

	const FAtomInventoryEntry* Entry = InventoryList.FindEntry(Slot->EquippedItemInstanceId);
	const UAtomItemDefinition* ItemDef = Entry ? Entry->ItemDef : nullptr;

	// Notify weapon manager before revoking
	if (Entry && Entry->ItemInstance)
	{
		if (UAtomWeaponManagerComponent* WeaponMgr = GetWeaponManager())
		{
			WeaponMgr->OnWeaponUnequipped(Entry->ItemInstance, SlotTag);
		}
	}

	// Revoke ability set
	UAbilitySystemComponent* ASC = GetASC();
	if (ASC)
	{
		Slot->GrantedHandles.RevokeFromASC(ASC);
	}

	Slot->EquippedItemInstanceId = INDEX_NONE;

	if (ItemDef)
	{
		OnItemUnequipped.Broadcast(ItemDef, SlotTag);
	}
	BroadcastInventoryChanged();
	return true;
}

UAtomItemDefinition* UAtomInventoryComponent::GetEquippedItem(FGameplayTag SlotTag) const
{
	const FAtomEquipmentSlotInfo* Slot = FindEquipmentSlot(SlotTag);
	if (!Slot || Slot->EquippedItemInstanceId == INDEX_NONE)
	{
		return nullptr;
	}

	const FAtomInventoryEntry* Entry = InventoryList.FindEntry(Slot->EquippedItemInstanceId);
	return Entry ? const_cast<UAtomItemDefinition*>(Entry->ItemDef.Get()) : nullptr;
}

bool UAtomInventoryComponent::IsSlotOccupied(FGameplayTag SlotTag) const
{
	const FAtomEquipmentSlotInfo* Slot = FindEquipmentSlot(SlotTag);
	return Slot && Slot->EquippedItemInstanceId != INDEX_NONE;
}

// ── Use ─────────────────────────────────────────────────────────

bool UAtomInventoryComponent::UseItem(int32 InstanceId)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	FAtomInventoryEntry* Entry = InventoryList.FindEntry(InstanceId);
	if (!Entry || !Entry->ItemDef)
	{
		return false;
	}

	const UAtomItemDefinition* ItemDef = Entry->ItemDef;
	if (!ItemDef->UseAbility)
	{
		UE_LOG(LogAtomInventory, Warning, TEXT("UseItem: Item '%s' has no UseAbility."), *ItemDef->DisplayName.ToString());
		return false;
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		UE_LOG(LogAtomInventory, Warning, TEXT("UseItem: No ASC found on owning actor."));
		return false;
	}

	// Grant and try to activate the use ability
	FGameplayAbilitySpec AbilitySpec(ItemDef->UseAbility, 1, INDEX_NONE, GetOwner());
	const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AbilitySpec);

	if (ASC->TryActivateAbility(Handle))
	{
		// Consume one from the stack
		RemoveItem(InstanceId, 1);
		OnItemUsed.Broadcast(ItemDef);

		// Clean up the transient ability after it ends
		ASC->SetRemoveAbilityOnEnd(Handle);
		return true;
	}

	// Activation failed — remove the transient ability
	ASC->ClearAbility(Handle);
	return false;
}

// ── Save/Load ───────────────────────────────────────────────────

FAtomInventorySaveData UAtomInventoryComponent::SaveInventory() const
{
	FAtomInventorySaveData SaveData;

	for (const FAtomInventoryEntry& Entry : InventoryList.Entries)
	{
		if (!Entry.ItemDef)
		{
			continue;
		}

		FAtomInventoryItemSaveEntry SaveEntry;
		SaveEntry.ItemDefPath = FSoftObjectPath(Entry.ItemDef);
		SaveEntry.StackCount = Entry.StackCount;
		SaveEntry.GridX = Entry.GridX;
		SaveEntry.GridY = Entry.GridY;

		// Check if this item is currently equipped
		for (const FAtomEquipmentSlotInfo& Slot : EquipmentSlots)
		{
			if (Slot.EquippedItemInstanceId == Entry.InstanceId)
			{
				SaveEntry.EquippedSlotTag = Slot.SlotTag;
				break;
			}
		}

		// Save weapon instance state
		if (Entry.ItemInstance)
		{
			UAtomItemInstance* Inst = Entry.ItemInstance;
			SaveEntry.CurrentAmmo = Inst->CurrentAmmo;
			SaveEntry.Quality = Inst->Quality;

			// Save affixes
			for (const FAtomItemAffix& Affix : Inst->Affixes)
			{
				FAtomSavedAffix SavedAffix;
				SavedAffix.AffixDefPath = FSoftObjectPath(Affix.AffixDef);
				SavedAffix.RolledValue = Affix.RolledValue;
				SavedAffix.Slot = static_cast<uint8>(Affix.Slot);
				SaveEntry.SavedAffixes.Add(SavedAffix);
			}

			// Save socketed gems
			for (const FAtomSocketSlot& Socket : Inst->SocketSlots)
			{
				SaveEntry.SavedSocketedGems.Add(Socket.SocketedGem ? FSoftObjectPath(Socket.SocketedGem) : FSoftObjectPath());
			}

			// Save mods
			if (Inst->InstalledModTier1)
			{
				SaveEntry.SavedModTier1 = FSoftObjectPath(Inst->InstalledModTier1);
			}
			if (Inst->InstalledModTier2)
			{
				SaveEntry.SavedModTier2 = FSoftObjectPath(Inst->InstalledModTier2);
			}
		}

		SaveData.Items.Add(SaveEntry);
	}

	return SaveData;
}

void UAtomInventoryComponent::LoadInventory(const FAtomInventorySaveData& Data)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// Unequip all slots first
	for (FAtomEquipmentSlotInfo& Slot : EquipmentSlots)
	{
		if (Slot.EquippedItemInstanceId != INDEX_NONE)
		{
			UnequipItem(Slot.SlotTag);
		}
	}

	// Clear current inventory
	InventoryList.Entries.Reset();
	InventoryList.MarkArrayDirty();

	if (IsGridMode())
	{
		OccupancyGrid.Init(INDEX_NONE, InventoryGridWidth * InventoryGridHeight);
	}

	// Restore items
	TArray<TPair<int32, FGameplayTag>> ItemsToEquip;

	for (const FAtomInventoryItemSaveEntry& SaveEntry : Data.Items)
	{
		UAtomItemDefinition* ItemDef = Cast<UAtomItemDefinition>(SaveEntry.ItemDefPath.TryLoad());
		if (!ItemDef)
		{
			UE_LOG(LogAtomInventory, Warning, TEXT("LoadInventory: Failed to load item at path '%s'"), *SaveEntry.ItemDefPath.ToString());
			continue;
		}

		const int32 NewInstanceId = GenerateInstanceId();
		const int32 EntryIdx = InventoryList.AddEntry(ItemDef, SaveEntry.StackCount, NewInstanceId, SaveEntry.GridX, SaveEntry.GridY);

		if (IsGridMode() && SaveEntry.GridX != INDEX_NONE)
		{
			SetOccupancy(SaveEntry.GridX, SaveEntry.GridY, ItemDef->GridWidth, ItemDef->GridHeight, NewInstanceId);
		}

		// Restore weapon instance if this is a weapon
		if (ItemDef->IsWeapon())
		{
			UAtomItemInstance* Inst = CreateItemInstance(ItemDef, NewInstanceId);
			InventoryList.Entries[EntryIdx].ItemInstance = Inst;

			Inst->CurrentAmmo = SaveEntry.CurrentAmmo;
			Inst->Quality = SaveEntry.Quality;

			// Restore affixes
			Inst->Affixes.Reset();
			for (const FAtomSavedAffix& SavedAffix : SaveEntry.SavedAffixes)
			{
				UAtomAffixDefinition* AffixDef = Cast<UAtomAffixDefinition>(SavedAffix.AffixDefPath.TryLoad());
				if (AffixDef)
				{
					FAtomItemAffix Affix;
					Affix.AffixDef = AffixDef;
					Affix.RolledValue = SavedAffix.RolledValue;
					Affix.Slot = static_cast<EAtomAffixSlot>(SavedAffix.Slot);
					Inst->Affixes.Add(Affix);
				}
			}

			// Restore socketed gems
			for (int32 i = 0; i < SaveEntry.SavedSocketedGems.Num() && i < Inst->SocketSlots.Num(); ++i)
			{
				if (SaveEntry.SavedSocketedGems[i].IsValid())
				{
					UAtomSkillGemDefinition* GemDef = Cast<UAtomSkillGemDefinition>(SaveEntry.SavedSocketedGems[i].TryLoad());
					if (GemDef)
					{
						Inst->SocketSlots[i].SocketedGem = GemDef;
					}
				}
			}

			// Restore mods
			if (SaveEntry.SavedModTier1.IsValid())
			{
				Inst->InstalledModTier1 = Cast<UAtomWeaponModDefinition>(SaveEntry.SavedModTier1.TryLoad());
			}
			if (SaveEntry.SavedModTier2.IsValid())
			{
				Inst->InstalledModTier2 = Cast<UAtomWeaponModDefinition>(SaveEntry.SavedModTier2.TryLoad());
			}
		}

		if (SaveEntry.EquippedSlotTag.IsValid())
		{
			ItemsToEquip.Add(TPair<int32, FGameplayTag>(NewInstanceId, SaveEntry.EquippedSlotTag));
		}
	}

	// Re-equip items that were equipped at save time
	for (const auto& Pair : ItemsToEquip)
	{
		EquipItem(Pair.Key);
	}

	BroadcastInventoryChanged();
}

// ── Item Instance API ────────────────────────────────────────────

UAtomItemInstance* UAtomInventoryComponent::GetItemInstance(FGameplayTag SlotTag) const
{
	const FAtomEquipmentSlotInfo* Slot = FindEquipmentSlot(SlotTag);
	if (!Slot || Slot->EquippedItemInstanceId == INDEX_NONE)
	{
		return nullptr;
	}

	const FAtomInventoryEntry* Entry = InventoryList.FindEntry(Slot->EquippedItemInstanceId);
	return Entry ? Entry->ItemInstance : nullptr;
}

UAtomItemInstance* UAtomInventoryComponent::GetItemInstanceById(int32 InstanceId) const
{
	const FAtomInventoryEntry* Entry = InventoryList.FindEntry(InstanceId);
	return Entry ? Entry->ItemInstance : nullptr;
}

// ── Private Helpers ─────────────────────────────────────────────

UAtomItemInstance* UAtomInventoryComponent::CreateItemInstance(UAtomItemDefinition* ItemDef, int32 InstanceId)
{
	if (!ItemDef || !ItemDef->IsWeapon())
	{
		return nullptr;
	}

	UAtomItemInstance* Instance = NewObject<UAtomItemInstance>(GetOwner());
	Instance->ItemDef = ItemDef;
	Instance->InstanceId = InstanceId;

	// Initialize shooter state
	if (ItemDef->ShooterWeaponData)
	{
		Instance->CurrentAmmo = ItemDef->ShooterWeaponData->MagazineSize;
	}

	// Initialize ARPG state — copy default socket layout
	if (ItemDef->ARPGWeaponData)
	{
		Instance->SocketSlots = ItemDef->ARPGWeaponData->DefaultSocketLayout;
	}

	return Instance;
}

UAtomWeaponManagerComponent* UAtomInventoryComponent::GetWeaponManager() const
{
	AActor* Owner = GetOwner();
	return Owner ? Owner->FindComponentByClass<UAtomWeaponManagerComponent>() : nullptr;
}

UAbilitySystemComponent* UAtomInventoryComponent::GetASC() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// Try the owner actor first (works for characters that implement IAbilitySystemInterface)
	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		return ASI->GetAbilitySystemComponent();
	}

	// Try the pawn's player state (common in multiplayer setups like this project)
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

FAtomEquipmentSlotInfo* UAtomInventoryComponent::FindEquipmentSlot(FGameplayTag SlotTag)
{
	for (FAtomEquipmentSlotInfo& Slot : EquipmentSlots)
	{
		if (Slot.SlotTag == SlotTag)
		{
			return &Slot;
		}
	}
	return nullptr;
}

const FAtomEquipmentSlotInfo* UAtomInventoryComponent::FindEquipmentSlot(FGameplayTag SlotTag) const
{
	for (const FAtomEquipmentSlotInfo& Slot : EquipmentSlots)
	{
		if (Slot.SlotTag == SlotTag)
		{
			return &Slot;
		}
	}
	return nullptr;
}

void UAtomInventoryComponent::BroadcastInventoryChanged()
{
	// On clients, rebuild the occupancy grid from replicated entries
	if (IsGridMode() && GetOwner() && !GetOwner()->HasAuthority())
	{
		RebuildOccupancyGrid();
	}

	OnInventoryChanged.Broadcast();
}

int32 UAtomInventoryComponent::GenerateInstanceId()
{
	return NextInstanceId++;
}

// ── Grid Mode ───────────────────────────────────────────────────

bool UAtomInventoryComponent::IsGridMode() const
{
	return InventoryGridWidth > 0 && InventoryGridHeight > 0;
}

bool UAtomInventoryComponent::CanPlaceItemAt(const UAtomItemDefinition* ItemDef, int32 X, int32 Y) const
{
	return CanPlaceItemAtIgnoring(ItemDef, X, Y, INDEX_NONE);
}

bool UAtomInventoryComponent::CanPlaceItemAtIgnoring(const UAtomItemDefinition* ItemDef, int32 X, int32 Y, int32 IgnoreInstanceId) const
{
	if (!IsGridMode() || !ItemDef)
	{
		return false;
	}

	return IsRectFree(X, Y, ItemDef->GridWidth, ItemDef->GridHeight, IgnoreInstanceId);
}

int32 UAtomInventoryComponent::AddItemAtPosition(UAtomItemDefinition* ItemDef, int32 X, int32 Y, int32 Count)
{
	if (!ItemDef || Count <= 0 || !IsGridMode())
	{
		return 0;
	}

	if (!GetOwner()->HasAuthority())
	{
		return 0;
	}

	// Try stacking at this position first
	if (ItemDef->MaxStackSize > 1)
	{
		for (FAtomInventoryEntry& Entry : InventoryList.Entries)
		{
			if (Entry.ItemDef == ItemDef && Entry.GridX == X && Entry.GridY == Y && Entry.StackCount < ItemDef->MaxStackSize)
			{
				const int32 SpaceInStack = ItemDef->MaxStackSize - Entry.StackCount;
				const int32 ToAdd = FMath::Min(Count, SpaceInStack);

				if (ItemDef->Weight > 0.0f && GetCurrentWeight() + ToAdd * ItemDef->Weight > MaxWeight)
				{
					const int32 CanFit = FMath::FloorToInt32((MaxWeight - GetCurrentWeight()) / ItemDef->Weight);
					if (CanFit <= 0) return 0;
					Entry.StackCount += FMath::Min(ToAdd, CanFit);
					InventoryList.MarkItemDirty(Entry);
					BroadcastInventoryChanged();
					return FMath::Min(ToAdd, CanFit);
				}

				Entry.StackCount += ToAdd;
				InventoryList.MarkItemDirty(Entry);
				BroadcastInventoryChanged();
				return ToAdd;
			}
		}
	}

	// Need a new entry — check if space is free
	if (!CanPlaceItemAt(ItemDef, X, Y))
	{
		return 0;
	}

	const int32 StackSize = FMath::Min(Count, ItemDef->MaxStackSize);

	// Weight check
	if (ItemDef->Weight > 0.0f && GetCurrentWeight() + StackSize * ItemDef->Weight > MaxWeight)
	{
		const int32 CanFit = FMath::FloorToInt32((MaxWeight - GetCurrentWeight()) / ItemDef->Weight);
		if (CanFit <= 0) return 0;
		const int32 Actual = FMath::Min(StackSize, CanFit);
		const int32 NewId = GenerateInstanceId();
		InventoryList.AddEntry(ItemDef, Actual, NewId, X, Y);
		SetOccupancy(X, Y, ItemDef->GridWidth, ItemDef->GridHeight, NewId);
		BroadcastInventoryChanged();
		return Actual;
	}

	const int32 NewId = GenerateInstanceId();
	InventoryList.AddEntry(ItemDef, StackSize, NewId, X, Y);
	SetOccupancy(X, Y, ItemDef->GridWidth, ItemDef->GridHeight, NewId);
	BroadcastInventoryChanged();
	return StackSize;
}

bool UAtomInventoryComponent::MoveItem(int32 InstanceId, int32 NewX, int32 NewY)
{
	if (!IsGridMode() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	FAtomInventoryEntry* Entry = InventoryList.FindEntry(InstanceId);
	if (!Entry || !Entry->ItemDef)
	{
		return false;
	}

	// Check if the new position is free (ignoring the item being moved)
	if (!CanPlaceItemAtIgnoring(Entry->ItemDef, NewX, NewY, InstanceId))
	{
		return false;
	}

	// Clear old position
	if (Entry->GridX != INDEX_NONE)
	{
		ClearOccupancy(Entry->GridX, Entry->GridY, Entry->ItemDef->GridWidth, Entry->ItemDef->GridHeight);
	}

	// Set new position
	Entry->GridX = NewX;
	Entry->GridY = NewY;
	SetOccupancy(NewX, NewY, Entry->ItemDef->GridWidth, Entry->ItemDef->GridHeight, InstanceId);

	InventoryList.MarkItemDirty(*Entry);
	BroadcastInventoryChanged();
	return true;
}

bool UAtomInventoryComponent::FindFreeSpace(const UAtomItemDefinition* ItemDef, int32& OutX, int32& OutY) const
{
	if (!IsGridMode() || !ItemDef)
	{
		return false;
	}

	// Scan top-to-bottom, left-to-right
	for (int32 Y = 0; Y <= InventoryGridHeight - ItemDef->GridHeight; ++Y)
	{
		for (int32 X = 0; X <= InventoryGridWidth - ItemDef->GridWidth; ++X)
		{
			if (IsRectFree(X, Y, ItemDef->GridWidth, ItemDef->GridHeight))
			{
				OutX = X;
				OutY = Y;
				return true;
			}
		}
	}

	return false;
}

FAtomInventoryEntry UAtomInventoryComponent::GetItemAtGridPosition(int32 X, int32 Y) const
{
	if (!IsGridMode() || X < 0 || X >= InventoryGridWidth || Y < 0 || Y >= InventoryGridHeight)
	{
		return FAtomInventoryEntry();
	}

	const int32 Id = OccupancyGrid[GridIndex(X, Y)];
	if (Id == INDEX_NONE)
	{
		return FAtomInventoryEntry();
	}

	const FAtomInventoryEntry* Entry = InventoryList.FindEntry(Id);
	return Entry ? *Entry : FAtomInventoryEntry();
}

// ── Grid Internals ──────────────────────────────────────────────

void UAtomInventoryComponent::RebuildOccupancyGrid()
{
	if (!IsGridMode())
	{
		return;
	}

	OccupancyGrid.Init(INDEX_NONE, InventoryGridWidth * InventoryGridHeight);

	for (const FAtomInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef && Entry.GridX != INDEX_NONE)
		{
			SetOccupancy(Entry.GridX, Entry.GridY, Entry.ItemDef->GridWidth, Entry.ItemDef->GridHeight, Entry.InstanceId);
		}
	}
}

void UAtomInventoryComponent::SetOccupancy(int32 GridX, int32 GridY, int32 W, int32 H, int32 InstanceId)
{
	for (int32 Y = GridY; Y < GridY + H && Y < InventoryGridHeight; ++Y)
	{
		for (int32 X = GridX; X < GridX + W && X < InventoryGridWidth; ++X)
		{
			OccupancyGrid[GridIndex(X, Y)] = InstanceId;
		}
	}
}

void UAtomInventoryComponent::ClearOccupancy(int32 GridX, int32 GridY, int32 W, int32 H)
{
	for (int32 Y = GridY; Y < GridY + H && Y < InventoryGridHeight; ++Y)
	{
		for (int32 X = GridX; X < GridX + W && X < InventoryGridWidth; ++X)
		{
			OccupancyGrid[GridIndex(X, Y)] = INDEX_NONE;
		}
	}
}

bool UAtomInventoryComponent::IsRectFree(int32 X, int32 Y, int32 W, int32 H, int32 IgnoreInstanceId) const
{
	if (X < 0 || Y < 0 || X + W > InventoryGridWidth || Y + H > InventoryGridHeight)
	{
		return false;
	}

	for (int32 CY = Y; CY < Y + H; ++CY)
	{
		for (int32 CX = X; CX < X + W; ++CX)
		{
			const int32 Cell = OccupancyGrid[GridIndex(CX, CY)];
			if (Cell != INDEX_NONE && Cell != IgnoreInstanceId)
			{
				return false;
			}
		}
	}

	return true;
}

int32 UAtomInventoryComponent::GridIndex(int32 X, int32 Y) const
{
	return Y * InventoryGridWidth + X;
}
