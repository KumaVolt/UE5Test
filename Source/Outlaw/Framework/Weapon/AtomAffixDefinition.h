// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AtomWeaponTypes.h"
#include "AtomAffixDefinition.generated.h"

class UGameplayEffect;

/**
 * Data asset defining a single affix that can roll on ARPG weapons.
 * Each affix applies a GameplayEffect with SetByCaller magnitude.
 */
UCLASS(BlueprintType, Const)
class OUTLAW_API UAtomAffixDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAtomAffixDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Display name shown in item tooltip (e.g. "of Fury", "Blazing"). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Affix")
	FText DisplayName;

	/** Whether this is a prefix or suffix. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Affix")
	EAtomAffixSlot AffixSlot = EAtomAffixSlot::Prefix;

	/** Minimum item level required to roll this affix. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Affix", meta = (ClampMin = "1"))
	int32 RequiredItemLevel = 1;

	/** Minimum value when rolling. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Affix|Range")
	float ValueMin = 1.0f;

	/** Maximum value when rolling. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Affix|Range")
	float ValueMax = 10.0f;

	/** Gameplay effect applied with SetByCaller magnitude. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Affix|Effect")
	TSubclassOf<UGameplayEffect> AffixEffect;

	/** Tag used to pass the rolled value into the GE via SetByCaller. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Affix|Effect", meta = (Categories = "SetByCaller"))
	FGameplayTag SetByCallerValueTag;

	/** Group tag to prevent duplicate affixes from the same group (e.g. Affix.Group.FlatPhysDamage). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Affix", meta = (Categories = "Affix.Group"))
	FGameplayTag AffixGroupTag;

	/** Weight for random selection. Higher = more common. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Affix", meta = (ClampMin = "1"))
	int32 Weight = 100;
};
