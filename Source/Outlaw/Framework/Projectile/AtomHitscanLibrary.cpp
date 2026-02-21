// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/AtomHitscanLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "Kismet/KismetMathLibrary.h"

TArray<FHitResult> UAtomHitscanLibrary::FireHitscan(
	const UObject* WorldContextObject,
	UAbilitySystemComponent* SourceASC,
	FVector Origin,
	FVector Direction,
	float Range,
	TSubclassOf<UGameplayEffect> DamageEffect,
	int32 Level,
	int32 PenetrationCount,
	float SpreadAngle
)
{
	TArray<FHitResult> Hits;

	if (!WorldContextObject || !SourceASC || !DamageEffect)
	{
		return Hits;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return Hits;
	}

	FVector FinalDirection = Direction;
	if (SpreadAngle > 0.f)
	{
		FRotator DirectionRotator = Direction.Rotation();
		float RandomPitch = FMath::FRandRange(-SpreadAngle, SpreadAngle);
		float RandomYaw = FMath::FRandRange(-SpreadAngle, SpreadAngle);
		DirectionRotator.Pitch += RandomPitch;
		DirectionRotator.Yaw += RandomYaw;
		FinalDirection = DirectionRotator.Vector();
	}

	FVector TraceEnd = Origin + (FinalDirection * Range);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Cast<AActor>(SourceASC->GetOwner()));

	TArray<FHitResult> TraceHits;
	World->LineTraceMultiByChannel(
		TraceHits,
		Origin,
		TraceEnd,
		ECC_Pawn,
		QueryParams
	);

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddInstigator(SourceASC->GetOwner(), SourceASC->GetOwner());

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, Level, EffectContext);

	if (!SpecHandle.IsValid())
	{
		return Hits;
	}

	int32 RemainingPenetration = PenetrationCount;

	for (const FHitResult& Hit : TraceHits)
	{
		if (!Hit.GetActor())
		{
			continue;
		}

		Hits.Add(Hit);

		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Hit.GetActor()))
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}

		if (RemainingPenetration <= 0)
		{
			break;
		}

		RemainingPenetration--;
	}

	return Hits;
}
