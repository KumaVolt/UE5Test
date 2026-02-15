// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "OutlawItemDefinition.generated.h"

class UOutlawAbilitySet;
class UGameplayAbility;
class UTexture2D;

/** Rarity tier for inventory items. Determines sort order and UI presentation. */
UENUM(BlueprintType)
enum class EOutlawItemRarity : uint8
{
	Common = 0,
	Uncommon,
	Rare,
	Epic,
	Legendary
};

/** Broad category of an item. Used for filtering and sorting. */
UENUM(BlueprintType)
enum class EOutlawItemType : uint8
{
	Miscellaneous = 0,
	Weapon,
	Shield,
	Helmet,
	Chest,
	Gloves,
	Boots,
	Ring,
	Amulet,
	Consumable,
	Material,
	Quest
};

/**
 * Data asset that defines an inventory item's properties.
 * Create instances in the Content Browser to define swords, potions, armor, etc.
 */
UCLASS(BlueprintType, Const)
class OUTLAW_API UOutlawItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UOutlawItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Display name shown in UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	/** Rarity tier (affects sort order and UI color). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EOutlawItemRarity Rarity = EOutlawItemRarity::Common;

	/** Broad item category (Weapon, Boots, Consumable, etc.). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EOutlawItemType ItemType = EOutlawItemType::Miscellaneous;

	/** Item description shown in tooltips. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Description;

	/** Icon texture for UI display. Soft reference to avoid loading all icons at once. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Maximum number of items in a single stack. 1 = non-stackable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
	int32 MaxStackSize = 1;

	/** Weight per single item unit. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0.0"))
	float Weight = 0.0f;

	/** Width in grid cells (for grid-based / PoE-style inventories). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Grid", meta = (ClampMin = "1"))
	int32 GridWidth = 1;

	/** Height in grid cells (for grid-based / PoE-style inventories). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Grid", meta = (ClampMin = "1"))
	int32 GridHeight = 1;

	/** Tags describing the item (e.g. Item.Weapon.Sword, Item.Consumable.Potion). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (Categories = "Item"))
	FGameplayTagContainer ItemTags;

	/** Ability set granted when this item is equipped. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UOutlawAbilitySet> GrantedAbilitySet;

	/** Ability activated when the item is used (e.g. drink potion). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Use")
	TSubclassOf<UGameplayAbility> UseAbility;

	/** Whether this item can be placed in an equipment slot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	bool bCanBeEquipped = false;

	/** Which equipment slot this item occupies (e.g. Equipment.Slot.MainHand). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (EditCondition = "bCanBeEquipped", Categories = "Equipment.Slot"))
	FGameplayTag EquipmentSlotTag;
};
