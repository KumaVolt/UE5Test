// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AtomProjectileTypes.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * Initialization data passed when spawning or retrieving a projectile from the pool.
 * Contains all runtime parameters needed to configure the projectile.
 */
USTRUCT(BlueprintType)
struct FAtomProjectileInitData
{
	GENERATED_BODY()

	/** Direction the projectile should travel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FVector Direction = FVector::ForwardVector;

	/** Initial speed of the projectile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Speed = 3000.f;

	/** Source ability system component (for damage application). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TObjectPtr<UAbilitySystemComponent> SourceASC = nullptr;

	/** Damage effect to apply on hit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<UGameplayEffect> DamageEffect = nullptr;

	/** Level of the damage effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	int32 Level = 1;

	/** Optional: Override penetration count for this specific projectile instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	int32 PenetrationCountOverride = -1;

	/** Optional: Override chain count for this specific projectile instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	int32 ChainCountOverride = -1;

	/** Optional: Homing target actor (for homing projectiles). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TObjectPtr<AActor> HomingTarget = nullptr;
};
