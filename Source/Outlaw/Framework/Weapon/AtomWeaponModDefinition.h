// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AtomWeaponModDefinition.generated.h"

class UAtomAbilitySet;
class UTexture2D;

/**
 * Data asset defining a weapon mod that can be installed in shooter weapons.
 * Each mod occupies a tier slot (Tier 1 or Tier 2) and grants abilities while installed.
 */
UCLASS(BlueprintType, Const)
class OUTLAW_API UAtomWeaponModDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAtomWeaponModDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Display name shown in UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mod")
	FText DisplayName;

	/** Description shown in tooltip. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mod")
	FText Description;

	/** Icon texture for UI display. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mod")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Which tier this mod can be installed in. 0 = any, 1 = Tier 1 only, 2 = Tier 2 only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mod", meta = (ClampMin = "0", ClampMax = "2"))
	int32 AllowedTier = 0;

	/** Abilities/effects granted while this mod is installed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mod")
	TObjectPtr<UAtomAbilitySet> GrantedAbilitySet;

	/** Cooldown tag for mod-specific cooldown tracking. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mod", meta = (Categories = "Cooldown"))
	FGameplayTag CooldownTag;
};
