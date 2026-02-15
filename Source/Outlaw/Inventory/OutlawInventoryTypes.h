// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "AbilitySystem/OutlawAbilityTypes.h"
#include "OutlawInventoryTypes.generated.h"

class UOutlawItemDefinition;
class UOutlawItemInstance;
class UOutlawInventoryComponent;
class UOutlawWeaponModDefinition;
class UOutlawSkillGemDefinition;
class UOutlawAffixDefinition;

// ────────────────────────────────────────────────────────────────
// FOutlawInventoryEntry — A single inventory slot (FFastArraySerializer item)
// ────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FOutlawInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FOutlawInventoryEntry()
		: ItemDef(nullptr)
		, StackCount(0)
		, InstanceId(INDEX_NONE)
		, GridX(INDEX_NONE)
		, GridY(INDEX_NONE)
	{
	}

	/** The item definition for this entry. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UOutlawItemDefinition> ItemDef;

	/** How many items are in this stack. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 StackCount;

	/** Unique instance identifier for network identification. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 InstanceId;

	/** Grid X position (top-left cell). INDEX_NONE when not using grid mode. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Grid")
	int32 GridX;

	/** Grid Y position (top-left cell). INDEX_NONE when not using grid mode. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Grid")
	int32 GridY;

	/** Per-item mutable state (ammo, affixes, gems, etc.). Only set for weapons. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UOutlawItemInstance> ItemInstance;

	// FFastArraySerializerItem callbacks
	void PreReplicatedRemove(const struct FOutlawInventoryList& InArraySerializer);
	void PostReplicatedAdd(const struct FOutlawInventoryList& InArraySerializer);
	void PostReplicatedChange(const struct FOutlawInventoryList& InArraySerializer);
};

// ────────────────────────────────────────────────────────────────
// FOutlawInventoryList — Replicated array of inventory entries
// ────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FOutlawInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	FOutlawInventoryList()
		: OwnerComponent(nullptr)
	{
	}

	explicit FOutlawInventoryList(UOutlawInventoryComponent* InOwner)
		: OwnerComponent(InOwner)
	{
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FOutlawInventoryEntry, FOutlawInventoryList>(Entries, DeltaParms, *this);
	}

	/** Add an entry and mark the array dirty. Returns the new entry's index. GridX/GridY default to INDEX_NONE (flat mode). */
	int32 AddEntry(UOutlawItemDefinition* ItemDef, int32 StackCount, int32 InstanceId, int32 GridX = INDEX_NONE, int32 GridY = INDEX_NONE);

	/** Remove an entry by instance ID. Returns true if found and removed. */
	bool RemoveEntry(int32 InstanceId);

	/** Find an entry by instance ID. Returns nullptr if not found. */
	FOutlawInventoryEntry* FindEntry(int32 InstanceId);
	const FOutlawInventoryEntry* FindEntry(int32 InstanceId) const;

	/** All inventory entries. */
	UPROPERTY()
	TArray<FOutlawInventoryEntry> Entries;

	/** Back-pointer to the owning component. */
	UPROPERTY(NotReplicated)
	TObjectPtr<UOutlawInventoryComponent> OwnerComponent;
};

/** Enable NetDeltaSerialize for FOutlawInventoryList. */
template<>
struct TStructOpsTypeTraits<FOutlawInventoryList> : public TStructOpsTypeTraitsBase2<FOutlawInventoryList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

// ────────────────────────────────────────────────────────────────
// FOutlawEquipmentSlotInfo — Tracks what's equipped in each slot
// ────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FOutlawEquipmentSlotInfo
{
	GENERATED_BODY()

	FOutlawEquipmentSlotInfo()
		: EquippedItemInstanceId(INDEX_NONE)
	{
	}

	/** Which equipment slot this represents (e.g. Equipment.Slot.MainHand). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (Categories = "Equipment.Slot"))
	FGameplayTag SlotTag;

	/** Instance ID of the equipped item, INDEX_NONE if empty. */
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	int32 EquippedItemInstanceId;

	/** Handles for abilities granted by the equipped item. Not replicated — server only. */
	UPROPERTY(NotReplicated)
	FOutlawAbilitySetGrantedHandles GrantedHandles;
};

// ────────────────────────────────────────────────────────────────
// FOutlawSavedAffix — Serialized affix for save/load
// ────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FOutlawSavedAffix
{
	GENERATED_BODY()

	/** Path to the affix definition asset. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	FSoftObjectPath AffixDefPath;

	/** The rolled value. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	float RolledValue = 0.0f;

	/** Prefix or suffix. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	uint8 Slot = 0;
};

// ────────────────────────────────────────────────────────────────
// FOutlawInventoryItemSaveEntry — Single item save record
// ────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FOutlawInventoryItemSaveEntry
{
	GENERATED_BODY()

	/** Soft path to the item definition asset. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	FSoftObjectPath ItemDefPath;

	/** Stack count of this entry. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	int32 StackCount = 0;

	/** Equipment slot tag if this item was equipped, empty otherwise. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	FGameplayTag EquippedSlotTag;

	/** Grid position X (for grid-mode inventories). INDEX_NONE if flat mode. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	int32 GridX = INDEX_NONE;

	/** Grid position Y (for grid-mode inventories). INDEX_NONE if flat mode. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	int32 GridY = INDEX_NONE;

	// ── Weapon Instance Save Fields ─────────────────────────────

	/** Current ammo in magazine (shooter weapons). */
	UPROPERTY(BlueprintReadWrite, Category = "Save|Weapon")
	int32 CurrentAmmo = 0;

	/** Quality value (ARPG weapons). */
	UPROPERTY(BlueprintReadWrite, Category = "Save|Weapon")
	int32 Quality = 0;

	/** Saved affix definitions and rolled values. */
	UPROPERTY(BlueprintReadWrite, Category = "Save|Weapon")
	TArray<FOutlawSavedAffix> SavedAffixes;

	/** Saved socketed gem paths per socket index. */
	UPROPERTY(BlueprintReadWrite, Category = "Save|Weapon")
	TArray<FSoftObjectPath> SavedSocketedGems;

	/** Saved Tier 1 mod path. */
	UPROPERTY(BlueprintReadWrite, Category = "Save|Weapon")
	FSoftObjectPath SavedModTier1;

	/** Saved Tier 2 mod path. */
	UPROPERTY(BlueprintReadWrite, Category = "Save|Weapon")
	FSoftObjectPath SavedModTier2;
};

// ────────────────────────────────────────────────────────────────
// FOutlawInventorySaveData — Complete inventory snapshot for save/load
// ────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FOutlawInventorySaveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Save")
	TArray<FOutlawInventoryItemSaveEntry> Items;
};
