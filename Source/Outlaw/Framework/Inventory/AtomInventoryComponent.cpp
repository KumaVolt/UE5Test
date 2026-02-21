// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawInventoryComponent.h"
#include "OutlawItemDefinition.h"
#include "OutlawItemInstance.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystem/OutlawAbilitySet.h"
#include "Weapon/OutlawWeaponManagerComponent.h"
#include "Weapon/OutlawShooterWeaponData.h"
#include "Weapon/OutlawARPGWeaponData.h"
#include "Weapon/OutlawAffixDefinition.h"
#include "Weapon/OutlawSkillGemDefinition.h"
#include "Weapon/OutlawWeaponModDefinition.h"
#include "Weapon/OutlawWeaponTypes.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogOutlawInventory, Log, All);

// ════════════════════════════════════════════════════════════════
// FOutlawInventoryEntry — FFastArraySerializerItem callbacks
// ════════════════════════════════════════════════════════════════

void FOutlawInventoryEntry::PreReplicatedRemove(const FOutlawInventoryList& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent)
	{
		InArraySerializer.OwnerComponent->BroadcastInventoryChanged();
	}
}

void FOutlawInventoryEntry::PostReplicatedAdd(const FOutlawInventoryList& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent)
	{
		InArraySerializer.OwnerComponent->BroadcastInventoryChanged();
	}
}

void FOutlawInventoryEntry::PostReplicatedChange(const FOutlawInventoryList& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent)
	{
		InArraySerializer.OwnerComponent->BroadcastInventoryChanged();
	}
}

// ════════════════════════════════════════════════════════════════
// FOutlawInventoryList — Helpers
// ════════════════════════════════════════════════════════════════

int32 FOutlawInventoryList::AddEntry(UOutlawItemDefinition* ItemDef, int32 StackCount, int32 InstanceId, int32 GridX, int32 GridY)
{
	FOutlawInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.ItemDef = ItemDef;
	NewEntry.StackCount = StackCount;
	NewEntry.InstanceId = InstanceId;
	NewEntry.GridX = GridX;
	NewEntry.GridY = GridY;

	MarkItemDirty(NewEntry);
	return Entries.Num() - 1;
}

bool FOutlawInventoryList::RemoveEntry(int32 InstanceId)
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

FOutlawInventoryEntry* FOutlawInventoryList::FindEntry(int32 InstanceId)
{
	for (FOutlawInventoryEntry& Entry : Entries)
	{
		if (Entry.InstanceId == InstanceId)
		{
			return &Entry;
		}
	}
	return nullptr;
}

const FOutlawInventoryEntry* FOutlawInventoryList::FindEntry(int32 InstanceId) const
{
	for (const FOutlawInventoryEntry& Entry : Entries)
	{
		if (Entry.InstanceId == InstanceId)
		{
			return &Entry;
		}
	}
	return nullptr;
}

// ════════════════════════════════════════════════════════════════
// UOutlawInventoryComponent
// ════════════════════════════════════════════════════════════════

UOutlawInventoryComponent::UOutlawInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
{
	SetIsReplicatedByDefault(true);
}

void UOutlawInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsGridMode())
	{
		RebuildOccupancyGrid();
	}
}

void UOutlawInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UOutlawInventoryComponent, InventoryList);
	DOREPLIFETIME(UOutlawInventoryComponent, EquipmentSlots);
}

// ── Core Inventory ──────────────────────────────────────────────

int32 UOutlawInventoryComponent::AddItem(UOutlawItemDefinition* ItemDef, int32 Count)
{
	if (!ItemDef || Count <= 0)
	{
		return 0;
	}

	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogOutlawInventory, Warning, TEXT("AddItem called on client. Item: %s"), *ItemDef->DisplayName.ToString());
		return 0;
	}

	int32 Remaining = Count;

	// First pass: fill existing stacks
	if (ItemDef->MaxStackSize > 1)
	{
		for (FOutlawInventoryEntry& Entry : InventoryList.Entries)
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

bool UOutlawInventoryComponent::RemoveItem(int32 InstanceId, int32 Count)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	FOutlawInventoryEntry* Entry = InventoryList.FindEntry(InstanceId);
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

int32 UOutlawInventoryComponent::RemoveItemByDef(UOutlawItemDefinition* ItemDef, int32 Count)
{
	if (!ItemDef || Count <= 0 || !GetOwner()->HasAuthority())
	{
		return 0;
	}

	int32 Remaining = Count;

	// Iterate in reverse so removals don't invalidate indices
	for (int32 i = InventoryList.Entries.Num() - 1; i >= 0 && Remaining > 0; --i)
	{
		FOutlawInventoryEntry& Entry = InventoryList.Entries[i];
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

TArray<FOutlawInventoryEntry> UOutlawInventoryComponent::FindItemsByTag(FGameplayTag Tag) const
{
	TArray<FOutlawInventoryEntry> Result;
	for (const FOutlawInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef && Entry.ItemDef->ItemTags.HasTag(Tag))
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

int32 UOutlawInventoryComponent::GetItemCount(const UOutlawItemDefinition* ItemDef) const
{
	int32 Total = 0;
	for (const FOutlawInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef == ItemDef)
		{
			Total += Entry.StackCount;
		}
	}
	return Total;
}

bool UOutlawInventoryComponent::HasItem(const UOutlawItemDefinition* ItemDef, int32 Count) const
{
	return GetItemCount(ItemDef) >= Count;
}

float UOutlawInventoryComponent::GetCurrentWeight() const
{
	float Total = 0.0f;
	for (const FOutlawInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef)
		{
			Total += Entry.ItemDef->Weight * Entry.StackCount;
		}
	}
	return Total;
}

int32 UOutlawInventoryComponent::GetRemainingSlots() const
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

TArray<FOutlawInventoryEntry> UOutlawInventoryComponent::FindItemsForSlot(FGameplayTag SlotTag) const
{
	TArray<FOutlawInventoryEntry> Result;
	for (const FOutlawInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef && Entry.ItemDef->bCanBeEquipped && Entry.ItemDef->EquipmentSlotTag == SlotTag)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

TArray<FOutlawInventoryEntry> UOutlawInventoryComponent::FindItemsByType(EOutlawItemType ItemType) const
{
	TArray<FOutlawInventoryEntry> Result;
	for (const FOutlawInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef && Entry.ItemDef->ItemType == ItemType)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

TArray<FOutlawInventoryEntry> UOutlawInventoryComponent::FindItemsByRarity(EOutlawItemRarity Rarity) const
{
	TArray<FOutlawInventoryEntry> Result;
	for (const FOutlawInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef && Entry.ItemDef->Rarity == Rarity)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

// ── Sorting ─────────────────────────────────────────────────────

void UOutlawInventoryComponent::SortInventory(EOutlawInventorySortMode SortMode, bool bDescending)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	InventoryList.Entries.Sort([SortMode, bDescending](const FOutlawInventoryEntry& A, const FOutlawInventoryEntry& B)
	{
		// Null-def entries sink to the bottom
		if (!A.ItemDef) return false;
		if (!B.ItemDef) return true;

		int32 Result = 0;

		switch (SortMode)
		{
		case EOutlawInventorySortMode::ByName:
			Result = A.ItemDef->DisplayName.CompareTo(B.ItemDef->DisplayName);
			break;

		case EOutlawInventorySortMode::ByRarity:
			Result = static_cast<int32>(A.ItemDef->Rarity) - static_cast<int32>(B.ItemDef->Rarity);
			break;

		case EOutlawInventorySortMode::ByType:
			Result = static_cast<int32>(A.ItemDef->ItemType) - static_cast<int32>(B.ItemDef->ItemType);
			// Secondary sort by name within same type
			if (Result == 0)
			{
				Result = A.ItemDef->DisplayName.CompareTo(B.ItemDef->DisplayName);
			}
			break;

		case EOutlawInventorySortMode::ByWeight:
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

bool UOutlawInventoryComponent::EquipItem(int32 InstanceId)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	FOutlawInventoryEntry* Entry = InventoryList.FindEntry(InstanceId);
	if (!Entry || !Entry->ItemDef)
	{
		UE_LOG(LogOutlawInventory, Warning, TEXT("EquipItem: Invalid InstanceId %d"), InstanceId);
		return false;
	}

	const UOutlawItemDefinition* ItemDef = Entry->ItemDef;
	if (!ItemDef->bCanBeEquipped)
	{
		UE_LOG(LogOutlawInventory, Warning, TEXT("EquipItem: Item '%s' cannot be equipped."), *ItemDef->DisplayName.ToString());
		return false;
	}

	FOutlawEquipmentSlotInfo* Slot = FindEquipmentSlot(ItemDef->EquipmentSlotTag);
	if (!Slot)
	{
		UE_LOG(LogOutlawInventory, Warning, TEXT("EquipItem: No equipment slot '%s' configured."), *ItemDef->EquipmentSlotTag.ToString());
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
		if (UOutlawWeaponManagerComponent* WeaponMgr = GetWeaponManager())
		{
			WeaponMgr->OnWeaponEquipped(Entry->ItemInstance, Slot->SlotTag);
		}
	}

	OnItemEquipped.Broadcast(ItemDef, Slot->SlotTag);
	BroadcastInventoryChanged();
	return true;
}

bool UOutlawInventoryComponent::UnequipItem(FGameplayTag SlotTag)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	FOutlawEquipmentSlotInfo* Slot = FindEquipmentSlot(SlotTag);
	if (!Slot || Slot->EquippedItemInstanceId == INDEX_NONE)
	{
		return false;
	}

	const FOutlawInventoryEntry* Entry = InventoryList.FindEntry(Slot->EquippedItemInstanceId);
	const UOutlawItemDefinition* ItemDef = Entry ? Entry->ItemDef : nullptr;

	// Notify weapon manager before revoking
	if (Entry && Entry->ItemInstance)
	{
		if (UOutlawWeaponManagerComponent* WeaponMgr = GetWeaponManager())
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

UOutlawItemDefinition* UOutlawInventoryComponent::GetEquippedItem(FGameplayTag SlotTag) const
{
	const FOutlawEquipmentSlotInfo* Slot = FindEquipmentSlot(SlotTag);
	if (!Slot || Slot->EquippedItemInstanceId == INDEX_NONE)
	{
		return nullptr;
	}

	const FOutlawInventoryEntry* Entry = InventoryList.FindEntry(Slot->EquippedItemInstanceId);
	return Entry ? const_cast<UOutlawItemDefinition*>(Entry->ItemDef.Get()) : nullptr;
}

bool UOutlawInventoryComponent::IsSlotOccupied(FGameplayTag SlotTag) const
{
	const FOutlawEquipmentSlotInfo* Slot = FindEquipmentSlot(SlotTag);
	return Slot && Slot->EquippedItemInstanceId != INDEX_NONE;
}

// ── Use ─────────────────────────────────────────────────────────

bool UOutlawInventoryComponent::UseItem(int32 InstanceId)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	FOutlawInventoryEntry* Entry = InventoryList.FindEntry(InstanceId);
	if (!Entry || !Entry->ItemDef)
	{
		return false;
	}

	const UOutlawItemDefinition* ItemDef = Entry->ItemDef;
	if (!ItemDef->UseAbility)
	{
		UE_LOG(LogOutlawInventory, Warning, TEXT("UseItem: Item '%s' has no UseAbility."), *ItemDef->DisplayName.ToString());
		return false;
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		UE_LOG(LogOutlawInventory, Warning, TEXT("UseItem: No ASC found on owning actor."));
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

FOutlawInventorySaveData UOutlawInventoryComponent::SaveInventory() const
{
	FOutlawInventorySaveData SaveData;

	for (const FOutlawInventoryEntry& Entry : InventoryList.Entries)
	{
		if (!Entry.ItemDef)
		{
			continue;
		}

		FOutlawInventoryItemSaveEntry SaveEntry;
		SaveEntry.ItemDefPath = FSoftObjectPath(Entry.ItemDef);
		SaveEntry.StackCount = Entry.StackCount;
		SaveEntry.GridX = Entry.GridX;
		SaveEntry.GridY = Entry.GridY;

		// Check if this item is currently equipped
		for (const FOutlawEquipmentSlotInfo& Slot : EquipmentSlots)
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
			UOutlawItemInstance* Inst = Entry.ItemInstance;
			SaveEntry.CurrentAmmo = Inst->CurrentAmmo;
			SaveEntry.Quality = Inst->Quality;

			// Save affixes
			for (const FOutlawItemAffix& Affix : Inst->Affixes)
			{
				FOutlawSavedAffix SavedAffix;
				SavedAffix.AffixDefPath = FSoftObjectPath(Affix.AffixDef);
				SavedAffix.RolledValue = Affix.RolledValue;
				SavedAffix.Slot = static_cast<uint8>(Affix.Slot);
				SaveEntry.SavedAffixes.Add(SavedAffix);
			}

			// Save socketed gems
			for (const FOutlawSocketSlot& Socket : Inst->SocketSlots)
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

void UOutlawInventoryComponent::LoadInventory(const FOutlawInventorySaveData& Data)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// Unequip all slots first
	for (FOutlawEquipmentSlotInfo& Slot : EquipmentSlots)
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

	for (const FOutlawInventoryItemSaveEntry& SaveEntry : Data.Items)
	{
		UOutlawItemDefinition* ItemDef = Cast<UOutlawItemDefinition>(SaveEntry.ItemDefPath.TryLoad());
		if (!ItemDef)
		{
			UE_LOG(LogOutlawInventory, Warning, TEXT("LoadInventory: Failed to load item at path '%s'"), *SaveEntry.ItemDefPath.ToString());
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
			UOutlawItemInstance* Inst = CreateItemInstance(ItemDef, NewInstanceId);
			InventoryList.Entries[EntryIdx].ItemInstance = Inst;

			Inst->CurrentAmmo = SaveEntry.CurrentAmmo;
			Inst->Quality = SaveEntry.Quality;

			// Restore affixes
			Inst->Affixes.Reset();
			for (const FOutlawSavedAffix& SavedAffix : SaveEntry.SavedAffixes)
			{
				UOutlawAffixDefinition* AffixDef = Cast<UOutlawAffixDefinition>(SavedAffix.AffixDefPath.TryLoad());
				if (AffixDef)
				{
					FOutlawItemAffix Affix;
					Affix.AffixDef = AffixDef;
					Affix.RolledValue = SavedAffix.RolledValue;
					Affix.Slot = static_cast<EOutlawAffixSlot>(SavedAffix.Slot);
					Inst->Affixes.Add(Affix);
				}
			}

			// Restore socketed gems
			for (int32 i = 0; i < SaveEntry.SavedSocketedGems.Num() && i < Inst->SocketSlots.Num(); ++i)
			{
				if (SaveEntry.SavedSocketedGems[i].IsValid())
				{
					UOutlawSkillGemDefinition* GemDef = Cast<UOutlawSkillGemDefinition>(SaveEntry.SavedSocketedGems[i].TryLoad());
					if (GemDef)
					{
						Inst->SocketSlots[i].SocketedGem = GemDef;
					}
				}
			}

			// Restore mods
			if (SaveEntry.SavedModTier1.IsValid())
			{
				Inst->InstalledModTier1 = Cast<UOutlawWeaponModDefinition>(SaveEntry.SavedModTier1.TryLoad());
			}
			if (SaveEntry.SavedModTier2.IsValid())
			{
				Inst->InstalledModTier2 = Cast<UOutlawWeaponModDefinition>(SaveEntry.SavedModTier2.TryLoad());
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

UOutlawItemInstance* UOutlawInventoryComponent::GetItemInstance(FGameplayTag SlotTag) const
{
	const FOutlawEquipmentSlotInfo* Slot = FindEquipmentSlot(SlotTag);
	if (!Slot || Slot->EquippedItemInstanceId == INDEX_NONE)
	{
		return nullptr;
	}

	const FOutlawInventoryEntry* Entry = InventoryList.FindEntry(Slot->EquippedItemInstanceId);
	return Entry ? Entry->ItemInstance : nullptr;
}

UOutlawItemInstance* UOutlawInventoryComponent::GetItemInstanceById(int32 InstanceId) const
{
	const FOutlawInventoryEntry* Entry = InventoryList.FindEntry(InstanceId);
	return Entry ? Entry->ItemInstance : nullptr;
}

// ── Private Helpers ─────────────────────────────────────────────

UOutlawItemInstance* UOutlawInventoryComponent::CreateItemInstance(UOutlawItemDefinition* ItemDef, int32 InstanceId)
{
	if (!ItemDef || !ItemDef->IsWeapon())
	{
		return nullptr;
	}

	UOutlawItemInstance* Instance = NewObject<UOutlawItemInstance>(GetOwner());
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

UOutlawWeaponManagerComponent* UOutlawInventoryComponent::GetWeaponManager() const
{
	AActor* Owner = GetOwner();
	return Owner ? Owner->FindComponentByClass<UOutlawWeaponManagerComponent>() : nullptr;
}

UAbilitySystemComponent* UOutlawInventoryComponent::GetASC() const
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

FOutlawEquipmentSlotInfo* UOutlawInventoryComponent::FindEquipmentSlot(FGameplayTag SlotTag)
{
	for (FOutlawEquipmentSlotInfo& Slot : EquipmentSlots)
	{
		if (Slot.SlotTag == SlotTag)
		{
			return &Slot;
		}
	}
	return nullptr;
}

const FOutlawEquipmentSlotInfo* UOutlawInventoryComponent::FindEquipmentSlot(FGameplayTag SlotTag) const
{
	for (const FOutlawEquipmentSlotInfo& Slot : EquipmentSlots)
	{
		if (Slot.SlotTag == SlotTag)
		{
			return &Slot;
		}
	}
	return nullptr;
}

void UOutlawInventoryComponent::BroadcastInventoryChanged()
{
	// On clients, rebuild the occupancy grid from replicated entries
	if (IsGridMode() && GetOwner() && !GetOwner()->HasAuthority())
	{
		RebuildOccupancyGrid();
	}

	OnInventoryChanged.Broadcast();
}

int32 UOutlawInventoryComponent::GenerateInstanceId()
{
	return NextInstanceId++;
}

// ── Grid Mode ───────────────────────────────────────────────────

bool UOutlawInventoryComponent::IsGridMode() const
{
	return InventoryGridWidth > 0 && InventoryGridHeight > 0;
}

bool UOutlawInventoryComponent::CanPlaceItemAt(const UOutlawItemDefinition* ItemDef, int32 X, int32 Y) const
{
	return CanPlaceItemAtIgnoring(ItemDef, X, Y, INDEX_NONE);
}

bool UOutlawInventoryComponent::CanPlaceItemAtIgnoring(const UOutlawItemDefinition* ItemDef, int32 X, int32 Y, int32 IgnoreInstanceId) const
{
	if (!IsGridMode() || !ItemDef)
	{
		return false;
	}

	return IsRectFree(X, Y, ItemDef->GridWidth, ItemDef->GridHeight, IgnoreInstanceId);
}

int32 UOutlawInventoryComponent::AddItemAtPosition(UOutlawItemDefinition* ItemDef, int32 X, int32 Y, int32 Count)
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
		for (FOutlawInventoryEntry& Entry : InventoryList.Entries)
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

bool UOutlawInventoryComponent::MoveItem(int32 InstanceId, int32 NewX, int32 NewY)
{
	if (!IsGridMode() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	FOutlawInventoryEntry* Entry = InventoryList.FindEntry(InstanceId);
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

bool UOutlawInventoryComponent::FindFreeSpace(const UOutlawItemDefinition* ItemDef, int32& OutX, int32& OutY) const
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

FOutlawInventoryEntry UOutlawInventoryComponent::GetItemAtGridPosition(int32 X, int32 Y) const
{
	if (!IsGridMode() || X < 0 || X >= InventoryGridWidth || Y < 0 || Y >= InventoryGridHeight)
	{
		return FOutlawInventoryEntry();
	}

	const int32 Id = OccupancyGrid[GridIndex(X, Y)];
	if (Id == INDEX_NONE)
	{
		return FOutlawInventoryEntry();
	}

	const FOutlawInventoryEntry* Entry = InventoryList.FindEntry(Id);
	return Entry ? *Entry : FOutlawInventoryEntry();
}

// ── Grid Internals ──────────────────────────────────────────────

void UOutlawInventoryComponent::RebuildOccupancyGrid()
{
	if (!IsGridMode())
	{
		return;
	}

	OccupancyGrid.Init(INDEX_NONE, InventoryGridWidth * InventoryGridHeight);

	for (const FOutlawInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef && Entry.GridX != INDEX_NONE)
		{
			SetOccupancy(Entry.GridX, Entry.GridY, Entry.ItemDef->GridWidth, Entry.ItemDef->GridHeight, Entry.InstanceId);
		}
	}
}

void UOutlawInventoryComponent::SetOccupancy(int32 GridX, int32 GridY, int32 W, int32 H, int32 InstanceId)
{
	for (int32 Y = GridY; Y < GridY + H && Y < InventoryGridHeight; ++Y)
	{
		for (int32 X = GridX; X < GridX + W && X < InventoryGridWidth; ++X)
		{
			OccupancyGrid[GridIndex(X, Y)] = InstanceId;
		}
	}
}

void UOutlawInventoryComponent::ClearOccupancy(int32 GridX, int32 GridY, int32 W, int32 H)
{
	for (int32 Y = GridY; Y < GridY + H && Y < InventoryGridHeight; ++Y)
	{
		for (int32 X = GridX; X < GridX + W && X < InventoryGridWidth; ++X)
		{
			OccupancyGrid[GridIndex(X, Y)] = INDEX_NONE;
		}
	}
}

bool UOutlawInventoryComponent::IsRectFree(int32 X, int32 Y, int32 W, int32 H, int32 IgnoreInstanceId) const
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

int32 UOutlawInventoryComponent::GridIndex(int32 X, int32 Y) const
{
	return Y * InventoryGridWidth + X;
}
