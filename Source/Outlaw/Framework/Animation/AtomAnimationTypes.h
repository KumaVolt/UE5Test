// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AtomAnimationTypes.generated.h"

class UAnimMontage;

/**
 * Hit reaction type based on damage percentage.
 */
UENUM(BlueprintType)
enum class EAtomHitReactionType : uint8
{
	None        UMETA(DisplayName = "None"),
	Light       UMETA(DisplayName = "Light"),     // < 10% MaxHealth
	Medium      UMETA(DisplayName = "Medium"),    // 10-30% MaxHealth
	Heavy       UMETA(DisplayName = "Heavy")      // > 30% MaxHealth
};

/**
 * Directional hit reaction variants.
 */
UENUM(BlueprintType)
enum class EAtomHitDirection : uint8
{
	Front       UMETA(DisplayName = "Front"),
	Back        UMETA(DisplayName = "Back"),
	Left        UMETA(DisplayName = "Left"),
	Right       UMETA(DisplayName = "Right")
};

/**
 * Configuration for a single hit reaction montage.
 */
USTRUCT(BlueprintType)
struct FAtomHitReactionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAtomHitReactionType ReactionType = EAtomHitReactionType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAtomHitDirection Direction = EAtomHitDirection::Front;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PlayRate = 1.f;
};

/**
 * Namespace for Animation Gameplay Tags.
 * Register these tags in your project's DefaultGameplayTags.ini or via data asset.
 */
namespace AtomAnimTags
{
	// Combat.DamageWindowActive — set during melee damage window notifies
	inline const FGameplayTag DamageWindowActive = FGameplayTag::RequestGameplayTag(TEXT("Combat.DamageWindowActive"));

	// State.Dead — character is dead
	inline const FGameplayTag Dead = FGameplayTag::RequestGameplayTag(TEXT("State.Dead"));

	// State.Staggered — character is playing hit reaction
	inline const FGameplayTag Staggered = FGameplayTag::RequestGameplayTag(TEXT("State.Staggered"));

	// Anim.HitReaction.Light — light hit reaction tag
	inline const FGameplayTag HitReactionLight = FGameplayTag::RequestGameplayTag(TEXT("Anim.HitReaction.Light"));

	// Anim.HitReaction.Medium — medium hit reaction tag
	inline const FGameplayTag HitReactionMedium = FGameplayTag::RequestGameplayTag(TEXT("Anim.HitReaction.Medium"));

	// Anim.HitReaction.Heavy — heavy hit reaction tag
	inline const FGameplayTag HitReactionHeavy = FGameplayTag::RequestGameplayTag(TEXT("Anim.HitReaction.Heavy"));
}
