// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OutlawAbilityTypes.h"
#include "OutlawAbilitySet.generated.h"

class UAbilitySystemComponent;
class UDataTable;

/**
 * Groups abilities, effects, and attribute sets into one grantable/revocable unit.
 * Designed after the Lyra AbilitySet pattern.
 */
UCLASS(BlueprintType, Const)
class OUTLAW_API UOutlawAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UOutlawAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Grants all abilities, effects, and attribute sets to the given ASC.
	 * @param ASC            The ability system component to grant to.
	 * @param SourceObject   The object responsible for granting (e.g. equipment actor).
	 * @param OutHandles     Filled with handles for later revocation.
	 */
	void GiveToAbilitySystem(UAbilitySystemComponent* ASC, UObject* SourceObject, FOutlawAbilitySetGrantedHandles& OutHandles) const;

	/**
	 * Populates the Abilities array from a DataTable of FOutlawAbilityTableRow rows.
	 */
	UFUNCTION(BlueprintCallable, Category = "Outlaw|AbilitySet")
	void PopulateFromDataTable(const UDataTable* DataTable);

protected:
	/** Gameplay abilities to grant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<FOutlawAbilityBindInfo> Abilities;

	/** Gameplay effects to apply (passives, auras). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TArray<FOutlawGrantedEffect> Effects;

	/** Attribute sets to grant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	TArray<FOutlawGrantedAttributeSet> AttributeSets;
};
