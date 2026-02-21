// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawAffixPoolDefinition.h"
#include "OutlawAffixDefinition.h"

DEFINE_LOG_CATEGORY_STATIC(LogOutlawAffixPool, Log, All);

UOutlawAffixPoolDefinition::UOutlawAffixPoolDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TArray<FOutlawItemAffix> UOutlawAffixPoolDefinition::RollAffixes(int32 ItemLevel, int32 NumPrefixes, int32 NumSuffixes) const
{
	TArray<FOutlawItemAffix> Result;

	// Separate eligible affixes by slot type
	TArray<UOutlawAffixDefinition*> EligiblePrefixes;
	TArray<UOutlawAffixDefinition*> EligibleSuffixes;

	for (const TObjectPtr<UOutlawAffixDefinition>& AffixDef : PossibleAffixes)
	{
		if (!AffixDef || AffixDef->RequiredItemLevel > ItemLevel)
		{
			continue;
		}

		if (AffixDef->AffixSlot == EOutlawAffixSlot::Prefix)
		{
			EligiblePrefixes.Add(AffixDef);
		}
		else
		{
			EligibleSuffixes.Add(AffixDef);
		}
	}

	// Helper lambda: weighted random pick from a pool, excluding already-used group tags
	auto RollFromPool = [&Result](TArray<UOutlawAffixDefinition*>& Pool, int32 Count, EOutlawAffixSlot Slot)
	{
		TSet<FGameplayTag> UsedGroups;

		// Collect groups already used by previously rolled affixes
		for (const FOutlawItemAffix& Existing : Result)
		{
			if (Existing.AffixDef && Existing.AffixDef->AffixGroupTag.IsValid())
			{
				UsedGroups.Add(Existing.AffixDef->AffixGroupTag);
			}
		}

		for (int32 i = 0; i < Count; ++i)
		{
			// Build weighted candidate list excluding used groups
			TArray<UOutlawAffixDefinition*> Candidates;
			TArray<int32> Weights;
			int32 TotalWeight = 0;

			for (UOutlawAffixDefinition* Def : Pool)
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

			UOutlawAffixDefinition* Selected = Candidates[SelectedIndex];

			// Roll the value
			FOutlawItemAffix NewAffix;
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

	RollFromPool(EligiblePrefixes, FMath::Min(NumPrefixes, MaxPrefixes), EOutlawAffixSlot::Prefix);
	RollFromPool(EligibleSuffixes, FMath::Min(NumSuffixes, MaxSuffixes), EOutlawAffixSlot::Suffix);

	return Result;
}
