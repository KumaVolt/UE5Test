// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomLootTable.h"
#include "Inventory/AtomItemDefinition.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtomLootTable, Log, All);

UAtomLootTable::UAtomLootTable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TArray<FAtomLootDrop> UAtomLootTable::RollLoot(int32 EnemyLevel, int32 NumDrops, float RarityBonus) const
{
	TArray<FAtomLootDrop> Result;

	if (Entries.Num() == 0 || NumDrops <= 0)
	{
		return Result;
	}

	for (int32 DropIndex = 0; DropIndex < NumDrops; ++DropIndex)
	{
		TArray<const FAtomLootTableEntry*> EligibleEntries;
		TArray<float> Weights;
		float TotalWeight = 0.0f;

		for (const FAtomLootTableEntry& Entry : Entries)
		{
			if (EnemyLevel < Entry.MinItemLevel || EnemyLevel > Entry.MaxItemLevel)
			{
				continue;
			}

			if (!Entry.ItemDefinition.IsNull())
			{
				EligibleEntries.Add(&Entry);
				Weights.Add(Entry.Weight);
				TotalWeight += Entry.Weight;
			}
		}

		if (EligibleEntries.Num() == 0 || TotalWeight <= 0.0f)
		{
			continue;
		}

		float RandomRoll = FMath::FRandRange(0.0f, TotalWeight);
		float CurrentWeight = 0.0f;
		const FAtomLootTableEntry* SelectedEntry = nullptr;

		for (int32 i = 0; i < EligibleEntries.Num(); ++i)
		{
			CurrentWeight += Weights[i];
			if (RandomRoll <= CurrentWeight)
			{
				SelectedEntry = EligibleEntries[i];
				break;
			}
		}

		if (!SelectedEntry)
		{
			SelectedEntry = EligibleEntries.Last();
		}

		const UAtomItemDefinition* ItemDef = SelectedEntry->ItemDefinition.LoadSynchronous();
		if (!ItemDef)
		{
			continue;
		}

		FAtomLootDrop Drop;
		Drop.ItemDefinition = ItemDef;
		Drop.Quantity = FMath::RandRange(SelectedEntry->MinQuantity, SelectedEntry->MaxQuantity);
		Drop.ItemLevel = FMath::RandRange(SelectedEntry->MinItemLevel, SelectedEntry->MaxItemLevel);
		Drop.RolledRarity = ItemDef->Rarity;

		Result.Add(Drop);
	}

	return Result;
}
