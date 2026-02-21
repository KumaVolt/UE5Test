// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/AtomBulletProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

AAtomBulletProjectile::AAtomBulletProjectile(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Speed = 10000.f;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	PenetrationCount = 0;
}
