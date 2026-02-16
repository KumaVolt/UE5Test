// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OutlawProjectileBase.h"
#include "OutlawSpellProjectile.generated.h"

UCLASS()
class OUTLAW_API AOutlawSpellProjectile : public AOutlawProjectileBase
{
	GENERATED_BODY()

public:
	AOutlawSpellProjectile(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float SplashRadius = 0.f;

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	void ApplySplashDamage(const FVector& ImpactLocation);
};
