// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/OutlawProjectilePoolSubsystem.h"
#include "Projectile/OutlawProjectileBase.h"

AOutlawProjectileBase* UOutlawProjectilePoolSubsystem::GetProjectile(TSubclassOf<AOutlawProjectileBase> ProjectileClass)
{
	if (!ProjectileClass)
	{
		return nullptr;
	}

	UClass* Class = ProjectileClass.Get();
	TArray<TObjectPtr<AOutlawProjectileBase>>* ClassPool = Pool.Find(Class);

	if (ClassPool && ClassPool->Num() > 0)
	{
		AOutlawProjectileBase* Projectile = (*ClassPool).Pop();
		return Projectile;
	}

	return CreateNewProjectile(ProjectileClass);
}

void UOutlawProjectilePoolSubsystem::ReturnProjectile(AOutlawProjectileBase* Projectile)
{
	if (!Projectile)
	{
		return;
	}

	UClass* Class = Projectile->GetClass();
	TArray<TObjectPtr<AOutlawProjectileBase>>& ClassPool = Pool.FindOrAdd(Class);

	if (ClassPool.Num() < MaxPoolSizePerClass)
	{
		ClassPool.Add(Projectile);
	}
	else
	{
		Projectile->Destroy();
	}
}

void UOutlawProjectilePoolSubsystem::PreWarmPool(TSubclassOf<AOutlawProjectileBase> ProjectileClass, int32 Count)
{
	if (!ProjectileClass)
	{
		return;
	}

	UClass* Class = ProjectileClass.Get();
	TArray<TObjectPtr<AOutlawProjectileBase>>& ClassPool = Pool.FindOrAdd(Class);

	for (int32 i = 0; i < Count; ++i)
	{
		if (ClassPool.Num() >= MaxPoolSizePerClass)
		{
			break;
		}

		if (AOutlawProjectileBase* Projectile = CreateNewProjectile(ProjectileClass))
		{
			Projectile->SetActorHiddenInGame(true);
			Projectile->SetActorEnableCollision(false);
			Projectile->SetActorTickEnabled(false);
			ClassPool.Add(Projectile);
		}
	}
}

AOutlawProjectileBase* UOutlawProjectilePoolSubsystem::CreateNewProjectile(TSubclassOf<AOutlawProjectileBase> ProjectileClass)
{
	UWorld* World = GetWorld();
	if (!World || !ProjectileClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AOutlawProjectileBase* Projectile = World->SpawnActor<AOutlawProjectileBase>(ProjectileClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	return Projectile;
}
