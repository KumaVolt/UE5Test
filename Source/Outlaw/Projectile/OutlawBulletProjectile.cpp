// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/OutlawBulletProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

AOutlawBulletProjectile::AOutlawBulletProjectile(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Speed = 10000.f;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	PenetrationCount = 0;
}
