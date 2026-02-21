// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AtomInventoryTypes.h"
#include "AtomItemDefinition.h"
#include "AtomInventoryComponent.generated.h"

class UAbilitySystemComponent;
class UAtomItemInstance;
class UAtomWeaponManagerComponent;

/** Criteria for sorting inventory entries. */
UENUM(BlueprintType)
enum class EAtomInventorySortMode : uint8
{
	ByName,
	ByRarity,
	ByType,
	ByWeight
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemEquipped, const UAtomItemDefinition*, ItemDef, FGameplayTag, SlotTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUnequipped, const UAtomItemDefinition*, ItemDef, FGameplayTag, SlotTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUsed, const UAtomItemDefinition*, ItemDef);

/**
 * Inventory component that can be dropped onto any actor Blueprint.
 * Supports stacking, weight limits, equipment slots with GAS integration, item use, and save/load.
 * Server-authoritative with FFastArraySerializer replication for inventory entries.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UAtomInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtomInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ── Core Inventory API ──────────────────────────────────────

	/**
	 * Add items to the inventory. Stacks with existing entries when possible.
	 * @param ItemDef   The item to add.
	 * @param Count     Number of items to add.
	 * @return Number of items actually added (may be less if weight/slot limit reached).
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItem(UAtomItemDefinition* ItemDef, int32 Count = 1);

	/**
	 * Remove items from a specific inventory entry.
	 * @param InstanceId  The instance ID of the entry.
	 * @param Count       Number to remove. If >= StackCount, the entire entry is removed.
	 * @return True if the entry was found and items were removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(int32 InstanceId, int32 Count = 1);

	/**
	 * Remove items by definition, drawing from any matching stacks.
	 * @param ItemDef  The item type to remove.
	 * @param Count    Number of items to remove.
	 * @return Number of items actually removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveItemByDef(UAtomItemDefinition* ItemDef, int32 Count);

	/** Find all inventory entries that have items matching a gameplay tag. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FAtomInventoryEntry> FindItemsByTag(FGameplayTag Tag) const;

	/** Get total count of a specific item definition across all stacks. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(const UAtomItemDefinition* ItemDef) const;

	/** Check if the inventory contains at least Count of the given item. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasItem(const UAtomItemDefinition* ItemDef, int32 Count = 1) const;

	/** Get total weight of all items in the inventory. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	float GetCurrentWeight() const;

	/** Get number of available inventory slots. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetRemainingSlots() const;

	/** Get all inventory entries that can be equipped in the given slot. Useful for slot-based UI (Destiny/Outriders style). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FAtomInventoryEntry> FindItemsForSlot(FGameplayTag SlotTag) const;

	/** Get all inventory entries matching a specific item type (Boots, Helmet, Weapon, etc.). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FAtomInventoryEntry> FindItemsByType(EAtomItemType ItemType) const;

	/** Get all inventory entries matching a specific rarity. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FAtomInventoryEntry> FindItemsByRarity(EAtomItemRarity Rarity) const;

	// ── Sorting API ─────────────────────────────────────────────

	/**
	 * Sort the inventory using the given mode.
	 * @param SortMode  How to sort (by name, rarity, type, or weight).
	 * @param bDescending  If true, sort highest-first (e.g. Legendary before Common).
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Sort")
	void SortInventory(EAtomInventorySortMode SortMode, bool bDescending = false);

	// ── Equipment API ───────────────────────────────────────────

	/**
	 * Equip an item from the inventory into its designated equipment slot.
	 * Grants the item's ability set to the ASC.
	 * @param InstanceId  The instance ID of the inventory entry to equip.
	 * @return True if the item was successfully equipped.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool EquipItem(int32 InstanceId);

	/**
	 * Unequip the item from the given equipment slot.
	 * Revokes the item's ability set from the ASC.
	 * @param SlotTag  The equipment slot to unequip.
	 * @return True if the slot was occupied and the item was unequipped.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool UnequipItem(FGameplayTag SlotTag);

	/** Get the item definition currently equipped in the given slot. Returns null if empty. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	UAtomItemDefinition* GetEquippedItem(FGameplayTag SlotTag) const;

	/** Check if an equipment slot is occupied. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool IsSlotOccupied(FGameplayTag SlotTag) const;

	// ── Use API ─────────────────────────────────────────────────

	/**
	 * Use an item: activates its UseAbility via ASC and consumes one from the stack.
	 * @param InstanceId  The instance ID of the entry to use.
	 * @return True if the item was used successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Use")
	bool UseItem(int32 InstanceId);

	// ── Item Instance API ───────────────────────────────────────

	/**
	 * Get the item instance for a weapon equipped in the given slot.
	 * @param SlotTag  The equipment slot tag.
	 * @return The item instance, or nullptr if no weapon equipped or no instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Weapon")
	UAtomItemInstance* GetItemInstance(FGameplayTag SlotTag) const;

	/**
	 * Get the item instance by instance ID.
	 * @param InstanceId  The unique instance ID.
	 * @return The item instance, or nullptr if not found or not a weapon.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Weapon")
	UAtomItemInstance* GetItemInstanceById(int32 InstanceId) const;

	// ── Save/Load ───────────────────────────────────────────────

	/** Serialize the full inventory state for saving. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
	FAtomInventorySaveData SaveInventory() const;

	/** Load inventory from previously saved data. Clears current inventory first. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
	void LoadInventory(const FAtomInventorySaveData& Data);

	// ── Delegates ───────────────────────────────────────────────

	/** Fires when any inventory entry is added, removed, or has its stack count changed. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	/** Fires when an item is equipped into a slot. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Equipment")
	FOnItemEquipped OnItemEquipped;

	/** Fires when an item is unequipped from a slot. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Equipment")
	FOnItemUnequipped OnItemUnequipped;

	/** Fires when an item is consumed via UseItem. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Use")
	FOnItemUsed OnItemUsed;

	// ── Grid API (PoE-style spatial inventory) ─────────────────

	/**
	 * Check if an item can be placed at the given grid position.
	 * Only meaningful in grid mode (InventoryGridWidth > 0).
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	bool CanPlaceItemAt(const UAtomItemDefinition* ItemDef, int32 X, int32 Y) const;

	/**
	 * Check if an item can be placed at a position, ignoring a specific entry (for move operations).
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	bool CanPlaceItemAtIgnoring(const UAtomItemDefinition* ItemDef, int32 X, int32 Y, int32 IgnoreInstanceId) const;

	/**
	 * Add an item at a specific grid position.
	 * @return Number of items actually added (0 if position blocked or out of bounds).
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	int32 AddItemAtPosition(UAtomItemDefinition* ItemDef, int32 X, int32 Y, int32 Count = 1);

	/**
	 * Move an existing inventory entry to a new grid position.
	 * @return True if the move succeeded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	bool MoveItem(int32 InstanceId, int32 NewX, int32 NewY);

	/**
	 * Find the first free grid position that can fit the given item.
	 * Scans left-to-right, top-to-bottom.
	 * @param OutX  Receives the X position.
	 * @param OutY  Receives the Y position.
	 * @return True if a free position was found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	bool FindFreeSpace(const UAtomItemDefinition* ItemDef, int32& OutX, int32& OutY) const;

	/** Get the item at a specific grid cell (returns the entry occupying that cell, if any). */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	FAtomInventoryEntry GetItemAtGridPosition(int32 X, int32 Y) const;

	/** Get all inventory entries (read-only). */
	const TArray<FAtomInventoryEntry>& GetEntries() const { return InventoryList.Entries; }

	/** Whether the component is operating in grid mode. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	bool IsGridMode() const;

	// ── Configuration ───────────────────────────────────────────

	/** Maximum number of inventory slots (flat mode only, ignored in grid mode). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Config")
	int32 MaxSlots = 20;

	/** Maximum total weight the inventory can carry. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Config")
	float MaxWeight = 100.0f;

	/**
	 * Grid width in cells. Set > 0 to enable PoE-style grid inventory.
	 * When 0, uses flat slot-based mode (Destiny/Outriders style).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Config|Grid")
	int32 InventoryGridWidth = 0;

	/** Grid height in cells. Only used when InventoryGridWidth > 0. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Config|Grid")
	int32 InventoryGridHeight = 0;

	/** Available equipment slots. Configure in the Blueprint defaults. */
	UPROPERTY(EditDefaultsOnly, Replicated, BlueprintReadOnly, Category = "Inventory|Equipment")
	TArray<FAtomEquipmentSlotInfo> EquipmentSlots;

private:
	/** Create an item instance for a weapon item definition. */
	UAtomItemInstance* CreateItemInstance(UAtomItemDefinition* ItemDef, int32 InstanceId);

	/** Notify the weapon manager when a weapon is equipped/unequipped. */
	UAtomWeaponManagerComponent* GetWeaponManager() const;

	/** Resolve the ASC from the owning actor via IAbilitySystemInterface. */
	UAbilitySystemComponent* GetASC() const;

	/** Find a mutable equipment slot by tag. */
	FAtomEquipmentSlotInfo* FindEquipmentSlot(FGameplayTag SlotTag);
	const FAtomEquipmentSlotInfo* FindEquipmentSlot(FGameplayTag SlotTag) const;

	/** Broadcast inventory changed on both server and clients. */
	void BroadcastInventoryChanged();

	/** Generate the next unique instance ID. Server-only. */
	int32 GenerateInstanceId();

	// ── Grid internals ──────────────────────────────────────────

	/** Rebuild the occupancy grid from current entries. Call after load or replication changes in grid mode. */
	void RebuildOccupancyGrid();

	/** Stamp/clear an item's footprint on the occupancy grid. */
	void SetOccupancy(int32 GridX, int32 GridY, int32 W, int32 H, int32 InstanceId);
	void ClearOccupancy(int32 GridX, int32 GridY, int32 W, int32 H);

	/** Check if a rectangle is free, optionally ignoring one instance ID. */
	bool IsRectFree(int32 X, int32 Y, int32 W, int32 H, int32 IgnoreInstanceId = INDEX_NONE) const;

	/** Convert 2D grid coordinate to 1D index. */
	int32 GridIndex(int32 X, int32 Y) const;

	/**
	 * Occupancy grid: flat array of size GridWidth*GridHeight.
	 * Each cell stores the InstanceId of the item occupying it, or INDEX_NONE if empty.
	 * Not replicated — rebuilt from entries on clients.
	 */
	TArray<int32> OccupancyGrid;

	/** The replicated inventory list. */
	UPROPERTY(Replicated)
	FAtomInventoryList InventoryList;

	/** Server-side counter for generating unique instance IDs. */
	int32 NextInstanceId = 0;

	// FAtomInventoryList needs access to BroadcastInventoryChanged
	friend struct FAtomInventoryEntry;
	friend struct FAtomInventoryList;
};
