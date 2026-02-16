// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OutlawProjectileTypes.h"
#include "OutlawProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS(Abstract)
class OUTLAW_API AOutlawProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AOutlawProjectileBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void InitProjectile(const FOutlawProjectileInitData& InitData);

	void ReturnToPool();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UNiagaraComponent> TrailComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float Speed = 3000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	int32 PenetrationCount = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	int32 ChainCount = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float ChainRadius = 500.f;

protected:
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	AActor* FindNextChainTarget(const FVector& Origin, float Radius);

	void ApplyDamageToTarget(AActor* Target);

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> SourceASC;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	int32 DamageEffectLevel = 1;

	int32 CurrentPenetrationCount = 0;
	int32 CurrentChainCount = 0;

	TSet<AActor*> HitActors;
};
