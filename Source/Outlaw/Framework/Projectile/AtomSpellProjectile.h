// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AtomProjectileBase.h"
#include "AtomSpellProjectile.generated.h"

UCLASS()
class OUTLAW_API AAtomSpellProjectile : public AAtomProjectileBase
{
	GENERATED_BODY()

public:
	AAtomSpellProjectile(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float SplashRadius = 0.f;

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	void ApplySplashDamage(const FVector& ImpactLocation);
};
