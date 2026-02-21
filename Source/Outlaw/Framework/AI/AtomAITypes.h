// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AtomAITypes.generated.h"

/**
 * AI perception mode enum
 */
UENUM(BlueprintType)
enum class EAtomAIPerceptionMode : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Alerted UMETA(DisplayName = "Alerted"),
	Combat UMETA(DisplayName = "Combat")
};

/**
 * Enemy spawner mode
 */
UENUM(BlueprintType)
enum class EAtomSpawnerMode : uint8
{
	WaveBased UMETA(DisplayName = "Wave Based"),
	Triggered UMETA(DisplayName = "Triggered"),
	Ambient UMETA(DisplayName = "Ambient")
};

/**
 * Wave spawn data
 */
USTRUCT(BlueprintType)
struct FAtomWaveSpawnData
{
	GENERATED_BODY()

	/** Enemy class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TSoftClassPtr<class AActor> EnemyClass;

	/** Number of enemies to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 Count = 1;

	/** Delay between each spawn in this wave */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float SpawnInterval = 0.5f;

	FAtomWaveSpawnData()
		: Count(1)
		, SpawnInterval(0.5f)
	{}
};

/**
 * StateTree context for AI controller
 */
USTRUCT(BlueprintType)
struct FAtomAIContext
{
	GENERATED_BODY()

	/** Current target actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TObjectPtr<AActor> TargetActor = nullptr;

	/** Home location for patrol return */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FVector HomeLocation = FVector::ZeroVector;

	/** Current patrol point index */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	int32 CurrentPatrolIndex = 0;

	/** Last known target location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FVector LastKnownTargetLocation = FVector::ZeroVector;

	/** Time spent searching for lost target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float SearchTime = 0.f;

	FAtomAIContext()
		: TargetActor(nullptr)
		, HomeLocation(FVector::ZeroVector)
		, CurrentPatrolIndex(0)
		, LastKnownTargetLocation(FVector::ZeroVector)
		, SearchTime(0.f)
	{}
};

/**
 * AI gameplay tags
 */
namespace AtomAITags
{
	inline const FGameplayTag Behavior_Patrol = FGameplayTag::RequestGameplayTag(TEXT("AI.Behavior.Patrol"));
	inline const FGameplayTag Behavior_Chase = FGameplayTag::RequestGameplayTag(TEXT("AI.Behavior.Chase"));
	inline const FGameplayTag Behavior_Attack = FGameplayTag::RequestGameplayTag(TEXT("AI.Behavior.Attack"));
	inline const FGameplayTag Behavior_Flee = FGameplayTag::RequestGameplayTag(TEXT("AI.Behavior.Flee"));
	inline const FGameplayTag Behavior_Search = FGameplayTag::RequestGameplayTag(TEXT("AI.Behavior.Search"));
	
	inline const FGameplayTag State_HasTarget = FGameplayTag::RequestGameplayTag(TEXT("AI.State.HasTarget"));
	inline const FGameplayTag State_LowHealth = FGameplayTag::RequestGameplayTag(TEXT("AI.State.LowHealth"));
}
