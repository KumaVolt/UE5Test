// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/AtomProjectilePoolSubsystem.h"
#include "Projectile/AtomProjectileBase.h"

AAtomProjectileBase* UAtomProjectilePoolSubsystem::GetProjectile(TSubclassOf<AAtomProjectileBase> ProjectileClass)
{
	if (!ProjectileClass)
	{
		return nullptr;
	}

	UClass* Class = ProjectileClass.Get();
	TArray<TObjectPtr<AAtomProjectileBase>>* ClassPool = Pool.Find(Class);

	if (ClassPool && ClassPool->Num() > 0)
	{
		AAtomProjectileBase* Projectile = (*ClassPool).Pop();
		return Projectile;
	}

	return CreateNewProjectile(ProjectileClass);
}

void UAtomProjectilePoolSubsystem::ReturnProjectile(AAtomProjectileBase* Projectile)
{
	if (!Projectile)
	{
		return;
	}

	UClass* Class = Projectile->GetClass();
	TArray<TObjectPtr<AAtomProjectileBase>>& ClassPool = Pool.FindOrAdd(Class);

	if (ClassPool.Num() < MaxPoolSizePerClass)
	{
		ClassPool.Add(Projectile);
	}
	else
	{
		Projectile->Destroy();
	}
}

void UAtomProjectilePoolSubsystem::PreWarmPool(TSubclassOf<AAtomProjectileBase> ProjectileClass, int32 Count)
{
	if (!ProjectileClass)
	{
		return;
	}

	UClass* Class = ProjectileClass.Get();
	TArray<TObjectPtr<AAtomProjectileBase>>& ClassPool = Pool.FindOrAdd(Class);

	for (int32 i = 0; i < Count; ++i)
	{
		if (ClassPool.Num() >= MaxPoolSizePerClass)
		{
			break;
		}

		if (AAtomProjectileBase* Projectile = CreateNewProjectile(ProjectileClass))
		{
			Projectile->SetActorHiddenInGame(true);
			Projectile->SetActorEnableCollision(false);
			Projectile->SetActorTickEnabled(false);
			ClassPool.Add(Projectile);
		}
	}
}

AAtomProjectileBase* UAtomProjectilePoolSubsystem::CreateNewProjectile(TSubclassOf<AAtomProjectileBase> ProjectileClass)
{
	UWorld* World = GetWorld();
	if (!World || !ProjectileClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAtomProjectileBase* Projectile = World->SpawnActor<AAtomProjectileBase>(ProjectileClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	return Projectile;
}
