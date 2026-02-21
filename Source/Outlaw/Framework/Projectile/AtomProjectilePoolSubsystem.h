// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OutlawProjectilePoolSubsystem.generated.h"

class AOutlawProjectileBase;

UCLASS(config=Game)
class OUTLAW_API UOutlawProjectilePoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	AOutlawProjectileBase* GetProjectile(TSubclassOf<AOutlawProjectileBase> ProjectileClass);

	void ReturnProjectile(AOutlawProjectileBase* Projectile);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void PreWarmPool(TSubclassOf<AOutlawProjectileBase> ProjectileClass, int32 Count);

	UPROPERTY(Config)
	int32 MaxPoolSizePerClass = 50;

private:
	// Non-UPROPERTY TMap because UHT doesn't support nested TObjectPtr containers
	TMap<TObjectPtr<UClass>, TArray<TObjectPtr<AOutlawProjectileBase>>> Pool;

	AOutlawProjectileBase* CreateNewProjectile(TSubclassOf<AOutlawProjectileBase> ProjectileClass);
};
