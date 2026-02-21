// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/AtomSpellProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Engine/OverlapResult.h"

AAtomSpellProjectile::AAtomSpellProjectile(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Speed = 2000.f;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bIsHomingProjectile = false;
	SplashRadius = 0.f;
}

void AAtomSpellProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	if (SplashRadius > 0.f)
	{
		ApplySplashDamage(Hit.ImpactPoint);
		ReturnToPool();
	}
	else
	{
		Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
	}
}

void AAtomSpellProjectile::ApplySplashDamage(const FVector& ImpactLocation)
{
	if (!SourceASC || !DamageEffectClass || !GetWorld())
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());

	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		ImpactLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(SplashRadius),
		QueryParams
	);

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	EffectContext.AddInstigator(GetOwner(), GetOwner());

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, DamageEffectLevel, EffectContext);

	if (!SpecHandle.IsValid())
	{
		return;
	}

	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (AActor* Target = Overlap.GetActor())
		{
			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
			{
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}
}
