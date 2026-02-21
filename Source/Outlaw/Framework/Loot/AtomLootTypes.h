// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Inventory/OutlawItemDefinition.h"
#include "OutlawLootTypes.generated.h"

class UOutlawItemDefinition;

/**
 * Single entry in a loot table with weight, item level range, and quantity range.
 */
USTRUCT(BlueprintType)
struct FOutlawLootTableEntry
{
	GENERATED_BODY()

	/** The item to drop (soft reference for lazy loading). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TSoftObjectPtr<UOutlawItemDefinition> ItemDefinition;

	/** Weight for random selection (higher = more likely). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "0.0"))
	float Weight = 1.0f;

	/** Minimum item level for this drop (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "1"))
	int32 MinItemLevel = 1;

	/** Maximum item level for this drop (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "1"))
	int32 MaxItemLevel = 100;

	/** Minimum quantity to drop. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "1"))
	int32 MinQuantity = 1;

	/** Maximum quantity to drop. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "1"))
	int32 MaxQuantity = 1;
};

/**
 * Result of a loot roll, containing the selected item, quantity, item level, and rarity.
 */
USTRUCT(BlueprintType)
struct FOutlawLootDrop
{
	GENERATED_BODY()

	/** The item definition that was selected. */
	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	TObjectPtr<const UOutlawItemDefinition> ItemDefinition = nullptr;

	/** Number of items to drop. */
	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	int32 Quantity = 1;

	/** Item level for this drop (used for affix rolling on weapons). */
	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	int32 ItemLevel = 1;

	/** The rarity tier that was rolled (may differ from ItemDefinition->Rarity if rarity is randomized). */
	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	EOutlawItemRarity RolledRarity = EOutlawItemRarity::Common;
};

namespace OutlawLootTags
{
	// Loot-related gameplay tags
	// Example: Loot.Droppable, Loot.QuestItem, Loot.Currency
	// Define in DefaultGameplayTags.ini
}
