// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OutlawWeaponTypes.h"
#include "OutlawAffixPoolDefinition.generated.h"

class UOutlawAffixDefinition;

/**
 * Data asset defining a pool of possible affixes for ARPG weapons.
 * Used by UOutlawItemInstance::RollAffixes() to randomly select affixes.
 */
UCLASS(BlueprintType, Const)
class OUTLAW_API UOutlawAffixPoolDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UOutlawAffixPoolDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** All possible affixes that can roll from this pool. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Affix Pool")
	TArray<TObjectPtr<UOutlawAffixDefinition>> PossibleAffixes;

	/** Maximum number of prefix affixes that can roll. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Affix Pool", meta = (ClampMin = "0", ClampMax = "6"))
	int32 MaxPrefixes = 3;

	/** Maximum number of suffix affixes that can roll. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Affix Pool", meta = (ClampMin = "0", ClampMax = "6"))
	int32 MaxSuffixes = 3;

	/**
	 * Roll a set of affixes from this pool.
	 * Filters by RequiredItemLevel, uses weighted random selection, and excludes duplicate AffixGroupTags.
	 * @param ItemLevel      The item level to filter eligible affixes.
	 * @param NumPrefixes    Number of prefix affixes to roll.
	 * @param NumSuffixes    Number of suffix affixes to roll.
	 * @return Array of rolled affixes.
	 */
	TArray<FOutlawItemAffix> RollAffixes(int32 ItemLevel, int32 NumPrefixes, int32 NumSuffixes) const;
};
