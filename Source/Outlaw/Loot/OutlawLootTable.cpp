// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawLootTable.h"
#include "Inventory/OutlawItemDefinition.h"

DEFINE_LOG_CATEGORY_STATIC(LogOutlawLootTable, Log, All);

UOutlawLootTable::UOutlawLootTable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TArray<FOutlawLootDrop> UOutlawLootTable::RollLoot(int32 EnemyLevel, int32 NumDrops, float RarityBonus) const
{
	TArray<FOutlawLootDrop> Result;

	if (Entries.Num() == 0 || NumDrops <= 0)
	{
		return Result;
	}

	for (int32 DropIndex = 0; DropIndex < NumDrops; ++DropIndex)
	{
		TArray<const FOutlawLootTableEntry*> EligibleEntries;
		TArray<float> Weights;
		float TotalWeight = 0.0f;

		for (const FOutlawLootTableEntry& Entry : Entries)
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
		const FOutlawLootTableEntry* SelectedEntry = nullptr;

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

		const UOutlawItemDefinition* ItemDef = SelectedEntry->ItemDefinition.LoadSynchronous();
		if (!ItemDef)
		{
			continue;
		}

		FOutlawLootDrop Drop;
		Drop.ItemDefinition = ItemDef;
		Drop.Quantity = FMath::RandRange(SelectedEntry->MinQuantity, SelectedEntry->MaxQuantity);
		Drop.ItemLevel = FMath::RandRange(SelectedEntry->MinItemLevel, SelectedEntry->MaxItemLevel);
		Drop.RolledRarity = ItemDef->Rarity;

		Result.Add(Drop);
	}

	return Result;
}
