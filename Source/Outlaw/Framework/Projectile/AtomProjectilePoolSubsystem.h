// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AtomProjectilePoolSubsystem.generated.h"

class AAtomProjectileBase;

UCLASS(config=Game)
class OUTLAW_API UAtomProjectilePoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	AAtomProjectileBase* GetProjectile(TSubclassOf<AAtomProjectileBase> ProjectileClass);

	void ReturnProjectile(AAtomProjectileBase* Projectile);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void PreWarmPool(TSubclassOf<AAtomProjectileBase> ProjectileClass, int32 Count);

	UPROPERTY(Config)
	int32 MaxPoolSizePerClass = 50;

private:
	// Non-UPROPERTY TMap because UHT doesn't support nested TObjectPtr containers
	TMap<TObjectPtr<UClass>, TArray<TObjectPtr<AAtomProjectileBase>>> Pool;

	AAtomProjectileBase* CreateNewProjectile(TSubclassOf<AAtomProjectileBase> ProjectileClass);
};
