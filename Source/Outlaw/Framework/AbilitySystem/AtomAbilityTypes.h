// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "AtomAbilityTypes.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UAttributeSet;
class UAbilitySystemComponent;

/**
 * Single ability entry within an ability set.
 */
USTRUCT(BlueprintType)
struct FAtomAbilityBindInfo
{
	GENERATED_BODY()

	/** The gameplay ability class to grant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> AbilityClass;

	/** Level to grant the ability at. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 AbilityLevel = 1;

	/** Input tag used to activate this ability via input binding. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Input"))
	FGameplayTag InputTag;

	/** Tags required on the ASC for this ability to activate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer ActivationRequiredTags;

	/** Tags that block activation of this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer ActivationBlockedTags;
};

/**
 * Gameplay effect entry granted as part of an ability set (passives, auras).
 */
USTRUCT(BlueprintType)
struct FAtomGrantedEffect
{
	GENERATED_BODY()

	/** The gameplay effect class to apply. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> EffectClass;

	/** Level of the effect. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 EffectLevel = 1;
};

/**
 * Attribute set entry granted as part of an ability set.
 */
USTRUCT(BlueprintType)
struct FAtomGrantedAttributeSet
{
	GENERATED_BODY()

	/** The attribute set class to grant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAttributeSet> AttributeSetClass;
};

/**
 * DataTable row struct for authoring abilities in spreadsheet format.
 */
USTRUCT(BlueprintType)
struct FAtomAbilityTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identity tag for this row. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag IdentityTag;

	/** The gameplay ability class to grant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> AbilityClass;

	/** Level to grant the ability at. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 AbilityLevel = 1;

	/** Input tag used to activate this ability via input binding. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Input"))
	FGameplayTag InputTag;

	/** Tags required on the ASC for this ability to activate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer ActivationRequiredTags;

	/** Tags that block activation of this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer ActivationBlockedTags;
};

/**
 * Tracks all handles granted by an ability set so they can be atomically revoked.
 */
USTRUCT(BlueprintType)
struct FAtomAbilitySetGrantedHandles
{
	GENERATED_BODY()

	/** Revoke everything tracked by these handles from the given ASC. */
	void RevokeFromASC(UAbilitySystemComponent* ASC);

	/** Granted ability spec handles. */
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	/** Active gameplay effect handles. */
	TArray<FActiveGameplayEffectHandle> EffectHandles;

	/** Granted attribute set instances. */
	TArray<TObjectPtr<UAttributeSet>> AttributeSets;
};
