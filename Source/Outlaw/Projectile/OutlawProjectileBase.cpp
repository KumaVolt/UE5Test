// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/OutlawProjectileBase.h"
#include "Projectile/OutlawProjectilePoolSubsystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "AbilitySystemInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"

AOutlawProjectileBase::AOutlawProjectileBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AOutlawProjectileBase::OnHit);
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TrailComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailComp"));
	TrailComp->SetupAttachment(RootComponent);
	TrailComp->bAutoActivate = false;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
}

void AOutlawProjectileBase::InitProjectile(const FOutlawProjectileInitData& InitData)
{
	SourceASC = InitData.SourceASC;
	DamageEffectClass = InitData.DamageEffect;
	DamageEffectLevel = InitData.Level;

	CurrentPenetrationCount = InitData.PenetrationCountOverride >= 0 ? InitData.PenetrationCountOverride : PenetrationCount;
	CurrentChainCount = InitData.ChainCountOverride >= 0 ? InitData.ChainCountOverride : ChainCount;

	HitActors.Reset();

	float FinalSpeed = InitData.Speed > 0.f ? InitData.Speed : Speed;
	ProjectileMovement->Velocity = InitData.Direction * FinalSpeed;

	if (InitData.HomingTarget)
	{
		ProjectileMovement->bIsHomingProjectile = true;
		ProjectileMovement->HomingTargetComponent = InitData.HomingTarget->GetRootComponent();
		ProjectileMovement->HomingAccelerationMagnitude = FinalSpeed * 2.0f;
	}

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(false);
	ProjectileMovement->SetActive(true);

	if (TrailComp)
	{
		TrailComp->Activate(true);
	}
}

void AOutlawProjectileBase::ReturnToPool()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->SetActive(false);

	if (TrailComp)
	{
		TrailComp->Deactivate();
	}

	HitActors.Reset();

	if (UWorld* World = GetWorld())
	{
		if (UOutlawProjectilePoolSubsystem* PoolSubsystem = World->GetSubsystem<UOutlawProjectilePoolSubsystem>())
		{
			PoolSubsystem->ReturnProjectile(this);
		}
	}
}

void AOutlawProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!OtherActor || OtherActor == GetOwner() || HitActors.Contains(OtherActor))
	{
		return;
	}

	HitActors.Add(OtherActor);

	ApplyDamageToTarget(OtherActor);

	if (CurrentPenetrationCount > 0)
	{
		CurrentPenetrationCount--;
		return;
	}

	if (CurrentChainCount > 0)
	{
		AActor* NextTarget = FindNextChainTarget(GetActorLocation(), ChainRadius);
		if (NextTarget && !HitActors.Contains(NextTarget))
		{
			CurrentChainCount--;
			FVector DirectionToNext = (NextTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			ProjectileMovement->Velocity = DirectionToNext * ProjectileMovement->Velocity.Size();
			return;
		}
	}

	ReturnToPool();
}

void AOutlawProjectileBase::ApplyDamageToTarget(AActor* Target)
{
	if (!SourceASC || !DamageEffectClass || !Target)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	EffectContext.AddInstigator(GetOwner(), GetOwner());
	EffectContext.AddHitResult(FHitResult());

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, DamageEffectLevel, EffectContext);
	if (SpecHandle.IsValid())
	{
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

AActor* AOutlawProjectileBase::FindNextChainTarget(const FVector& Origin, float Radius)
{
	if (!GetWorld())
	{
		return nullptr;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());
	for (AActor* HitActor : HitActors)
	{
		QueryParams.AddIgnoredActor(HitActor);
	}

	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Origin,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	float ClosestDistSq = FLT_MAX;
	AActor* ClosestTarget = nullptr;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (AActor* PotentialTarget = Overlap.GetActor())
		{
			if (UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PotentialTarget))
			{
				float DistSq = FVector::DistSquared(Origin, PotentialTarget->GetActorLocation());
				if (DistSq < ClosestDistSq)
				{
					ClosestDistSq = DistSq;
					ClosestTarget = PotentialTarget;
				}
			}
		}
	}

	return ClosestTarget;
}
