// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "Combat/AtomCombatTypes.h"
#include "AtomCombatLibrary.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS()
class OUTLAW_API UAtomCombatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Atom|Combat", meta = (WorldContext = "WorldContextObject"))
	static FHitResult PerformLineTrace(
		const UObject* WorldContextObject,
		const FVector& Origin,
		const FVector& Direction,
		float Range,
		const TArray<AActor*>& IgnoreActors,
		bool bTraceComplex = false,
		bool bShowDebug = false
	);

	UFUNCTION(BlueprintCallable, Category = "Atom|Combat", meta = (WorldContext = "WorldContextObject"))
	static TArray<FHitResult> PerformSphereOverlap(
		const UObject* WorldContextObject,
		const FVector& Origin,
		float Radius,
		const TArray<AActor*>& IgnoreActors,
		bool bShowDebug = false
	);

	UFUNCTION(BlueprintCallable, Category = "Atom|Combat", meta = (WorldContext = "WorldContextObject"))
	static TArray<FHitResult> PerformMeleeBoxTrace(
		const UObject* WorldContextObject,
		const FVector& Origin,
		const FVector& Direction,
		float Length,
		float Width,
		float Height,
		const TArray<AActor*>& IgnoreActors,
		bool bShowDebug = false
	);

	UFUNCTION(BlueprintCallable, Category = "Atom|Combat")
	static void ApplyDamageToTarget(
		UAbilitySystemComponent* SourceASC,
		UAbilitySystemComponent* TargetASC,
		TSubclassOf<UGameplayEffect> DamageEffectClass,
		float Level,
		const TMap<FGameplayTag, float>& SetByCallerMags
	);
};
