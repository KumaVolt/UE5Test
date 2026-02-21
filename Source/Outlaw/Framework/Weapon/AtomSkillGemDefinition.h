// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AtomSkillGemDefinition.generated.h"

class UAtomAbilitySet;
class UTexture2D;

/**
 * Data asset defining a skill gem that can be socketed into ARPG weapons.
 * Active gems grant abilities; support gems modify linked active gems.
 */
UCLASS(BlueprintType, Const)
class OUTLAW_API UAtomSkillGemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAtomSkillGemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Display name shown in UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gem")
	FText DisplayName;

	/** Icon texture for UI display. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gem")
	TSoftObjectPtr<UTexture2D> Icon;

	/** True for support gems, false for active skill gems. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gem")
	bool bIsSupportGem = false;

	/** Required socket type tag for compatibility (e.g. Socket.Red). Empty = fits any socket. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gem", meta = (Categories = "Socket"))
	FGameplayTag RequiredSocketTypeTag;

	/** Abilities granted when this gem is socketed and the weapon is active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gem")
	TObjectPtr<UAtomAbilitySet> GrantedAbilitySet;

	/** Tags describing this gem for support gem compatibility filtering. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gem")
	FGameplayTagContainer GemTags;
};
