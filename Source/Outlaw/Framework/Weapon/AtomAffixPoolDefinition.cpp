// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomAffixPoolDefinition.h"
#include "AtomAffixDefinition.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtomAffixPool, Log, All);

UAtomAffixPoolDefinition::UAtomAffixPoolDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TArray<FAtomItemAffix> UAtomAffixPoolDefinition::RollAffixes(int32 ItemLevel, int32 NumPrefixes, int32 NumSuffixes) const
{
	TArray<FAtomItemAffix> Result;

	// Separate eligible affixes by slot type
	TArray<UAtomAffixDefinition*> EligiblePrefixes;
	TArray<UAtomAffixDefinition*> EligibleSuffixes;

	for (const TObjectPtr<UAtomAffixDefinition>& AffixDef : PossibleAffixes)
	{
		if (!AffixDef || AffixDef->RequiredItemLevel > ItemLevel)
		{
			continue;
		}

		if (AffixDef->AffixSlot == EAtomAffixSlot::Prefix)
		{
			EligiblePrefixes.Add(AffixDef);
		}
		else
		{
			EligibleSuffixes.Add(AffixDef);
		}
	}

	// Helper lambda: weighted random pick from a pool, excluding already-used group tags
	auto RollFromPool = [&Result](TArray<UAtomAffixDefinition*>& Pool, int32 Count, EAtomAffixSlot Slot)
	{
		TSet<FGameplayTag> UsedGroups;

		// Collect groups already used by previously rolled affixes
		for (const FAtomItemAffix& Existing : Result)
		{
			if (Existing.AffixDef && Existing.AffixDef->AffixGroupTag.IsValid())
			{
				UsedGroups.Add(Existing.AffixDef->AffixGroupTag);
			}
		}

		for (int32 i = 0; i < Count; ++i)
		{
			// Build weighted candidate list excluding used groups
			TArray<UAtomAffixDefinition*> Candidates;
			TArray<int32> Weights;
			int32 TotalWeight = 0;

			for (UAtomAffixDefinition* Def : Pool)
			{
				if (Def->AffixGroupTag.IsValid() && UsedGroups.Contains(Def->AffixGroupTag))
				{
					continue;
				}
				Candidates.Add(Def);
				Weights.Add(Def->Weight);
				TotalWeight += Def->Weight;
			}

			if (Candidates.Num() == 0 || TotalWeight <= 0)
			{
				break;
			}

			// Weighted random selection
			int32 Roll = FMath::RandRange(0, TotalWeight - 1);
			int32 SelectedIndex = 0;
			for (int32 j = 0; j < Candidates.Num(); ++j)
			{
				Roll -= Weights[j];
				if (Roll < 0)
				{
					SelectedIndex = j;
					break;
				}
			}

			UAtomAffixDefinition* Selected = Candidates[SelectedIndex];

			// Roll the value
			FAtomItemAffix NewAffix;
			NewAffix.AffixDef = Selected;
			NewAffix.Slot = Slot;
			NewAffix.RolledValue = FMath::FRandRange(Selected->ValueMin, Selected->ValueMax);
			Result.Add(NewAffix);

			// Mark this group as used
			if (Selected->AffixGroupTag.IsValid())
			{
				UsedGroups.Add(Selected->AffixGroupTag);
			}
		}
	};

	RollFromPool(EligiblePrefixes, FMath::Min(NumPrefixes, MaxPrefixes), EAtomAffixSlot::Prefix);
	RollFromPool(EligibleSuffixes, FMath::Min(NumSuffixes, MaxSuffixes), EAtomAffixSlot::Suffix);

	return Result;
}
