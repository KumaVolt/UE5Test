// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawCombatLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

FHitResult UOutlawCombatLibrary::PerformLineTrace(
	const UObject* WorldContextObject,
	const FVector& Origin,
	const FVector& Direction,
	float Range,
	const TArray<AActor*>& IgnoreActors,
	bool bTraceComplex,
	bool bShowDebug)
{
	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		return FHitResult();
	}

	UWorld* World = WorldContextObject->GetWorld();
	const FVector End = Origin + (Direction.GetSafeNormal() * Range);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(IgnoreActors);
	QueryParams.bTraceComplex = bTraceComplex;

	FHitResult HitResult;
	World->LineTraceSingleByChannel(
		HitResult,
		Origin,
		End,
		ECC_Pawn,
		QueryParams
	);

	if (bShowDebug)
	{
		const FColor DebugColor = HitResult.bBlockingHit ? FColor::Red : FColor::Green;
		DrawDebugLine(World, Origin, End, DebugColor, false, 2.f, 0, 1.f);
		if (HitResult.bBlockingHit)
		{
			DrawDebugSphere(World, HitResult.ImpactPoint, 10.f, 8, FColor::Red, false, 2.f);
		}
	}

	return HitResult;
}

TArray<FHitResult> UOutlawCombatLibrary::PerformSphereOverlap(
	const UObject* WorldContextObject,
	const FVector& Origin,
	float Radius,
	const TArray<AActor*>& IgnoreActors,
	bool bShowDebug)
{
	TArray<FHitResult> HitResults;

	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		return HitResults;
	}

	UWorld* World = WorldContextObject->GetWorld();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(IgnoreActors);

	World->SweepMultiByChannel(
		HitResults,
		Origin,
		Origin,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	if (bShowDebug)
	{
		DrawDebugSphere(World, Origin, Radius, 16, FColor::Orange, false, 2.f);
		for (const FHitResult& Hit : HitResults)
		{
			DrawDebugSphere(World, Hit.ImpactPoint, 10.f, 8, FColor::Red, false, 2.f);
		}
	}

	return HitResults;
}

TArray<FHitResult> UOutlawCombatLibrary::PerformMeleeBoxTrace(
	const UObject* WorldContextObject,
	const FVector& Origin,
	const FVector& Direction,
	float Length,
	float Width,
	float Height,
	const TArray<AActor*>& IgnoreActors,
	bool bShowDebug)
{
	TArray<FHitResult> HitResults;

	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		return HitResults;
	}

	UWorld* World = WorldContextObject->GetWorld();

	const FVector NormalizedDirection = Direction.GetSafeNormal();
	const FVector End = Origin + (NormalizedDirection * Length);
	const FQuat Rotation = NormalizedDirection.ToOrientationQuat();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(IgnoreActors);

	World->SweepMultiByChannel(
		HitResults,
		Origin,
		End,
		Rotation,
		ECC_Pawn,
		FCollisionShape::MakeBox(FVector(Length * 0.5f, Width * 0.5f, Height * 0.5f)),
		QueryParams
	);

	if (bShowDebug)
	{
		DrawDebugBox(World, (Origin + End) * 0.5f, FVector(Length * 0.5f, Width * 0.5f, Height * 0.5f), Rotation, FColor::Cyan, false, 2.f);
		for (const FHitResult& Hit : HitResults)
		{
			DrawDebugSphere(World, Hit.ImpactPoint, 10.f, 8, FColor::Red, false, 2.f);
		}
	}

	return HitResults;
}

void UOutlawCombatLibrary::ApplyDamageToTarget(
	UAbilitySystemComponent* SourceASC,
	UAbilitySystemComponent* TargetASC,
	TSubclassOf<UGameplayEffect> DamageEffectClass,
	float Level,
	const TMap<FGameplayTag, float>& SetByCallerMags)
{
	if (!SourceASC || !TargetASC || !DamageEffectClass)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(SourceASC->GetAvatarActor());

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, Level, EffectContext);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	for (const auto& Pair : SetByCallerMags)
	{
		SpecHandle.Data->SetSetByCallerMagnitude(Pair.Key, Pair.Value);
	}

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}
