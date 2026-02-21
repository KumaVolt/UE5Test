// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AtomHitscanLibrary.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS()
class OUTLAW_API UAtomHitscanLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Atom|Projectile", meta = (WorldContext = "WorldContextObject"))
	static TArray<FHitResult> FireHitscan(
		const UObject* WorldContextObject,
		UAbilitySystemComponent* SourceASC,
		FVector Origin,
		FVector Direction,
		float Range,
		TSubclassOf<UGameplayEffect> DamageEffect,
		int32 Level,
		int32 PenetrationCount = 0,
		float SpreadAngle = 0.f
	);
};
