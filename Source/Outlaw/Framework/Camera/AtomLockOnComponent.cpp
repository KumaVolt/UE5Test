// Fill out your copyright notice in the Description page of Project Settings.

#include "Camera/AtomLockOnComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"

UAtomLockOnComponent::UAtomLockOnComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAtomLockOnComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAtomLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentTarget && !IsTargetValid(CurrentTarget))
	{
		BreakLockOn();
	}
}

void UAtomLockOnComponent::ToggleLockOn()
{
	if (CurrentTarget)
	{
		BreakLockOn();
		return;
	}

	TArray<AActor*> Candidates = FindCandidates();
	if (Candidates.Num() == 0) return;

	CurrentTarget = FindNearestCandidate(Candidates);
	if (CurrentTarget)
	{
		OnLockOnTargetChanged.Broadcast(CurrentTarget);
	}
}

void UAtomLockOnComponent::CycleLockOnTarget(bool bNext)
{
	TArray<AActor*> Candidates = FindCandidates();
	if (Candidates.Num() == 0)
	{
		BreakLockOn();
		return;
	}

	int32 CurrentIndex = Candidates.Find(CurrentTarget);
	if (CurrentIndex == INDEX_NONE)
	{
		CurrentTarget = FindNearestCandidate(Candidates);
	}
	else
	{
		int32 NewIndex = bNext ? (CurrentIndex + 1) % Candidates.Num() : (CurrentIndex - 1 + Candidates.Num()) % Candidates.Num();
		CurrentTarget = Candidates[NewIndex];
	}

	if (CurrentTarget)
	{
		OnLockOnTargetChanged.Broadcast(CurrentTarget);
	}
}

void UAtomLockOnComponent::BreakLockOn()
{
	if (!CurrentTarget) return;

	CurrentTarget = nullptr;
	OnLockOnBroken.Broadcast();
}

TArray<AActor*> UAtomLockOnComponent::FindCandidates() const
{
	TArray<AActor*> Candidates;

	AActor* Owner = GetOwner();
	if (!Owner) return Candidates;

	UWorld* World = GetWorld();
	if (!World) return Candidates;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(
		Overlaps,
		Owner->GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(LockOnRange),
		QueryParams
	);

	FGameplayTag TargetableTag = FGameplayTag::RequestGameplayTag(FName("Combat.Targetable"));

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (!Actor) continue;

		IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Actor);
		if (!ASI) continue;

		UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
		if (!ASC) continue;

		if (ASC->HasMatchingGameplayTag(TargetableTag))
		{
			Candidates.Add(Actor);
		}
	}

	return Candidates;
}

AActor* UAtomLockOnComponent::FindNearestCandidate(const TArray<AActor*>& Candidates) const
{
	AActor* Owner = GetOwner();
	if (!Owner || Candidates.Num() == 0) return nullptr;

	FVector OwnerLocation = Owner->GetActorLocation();
	AActor* Nearest = nullptr;
	float MinDistance = FLT_MAX;

	for (AActor* Candidate : Candidates)
	{
		if (!Candidate) continue;

		float Distance = FVector::Dist(OwnerLocation, Candidate->GetActorLocation());
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			Nearest = Candidate;
		}
	}

	return Nearest;
}

bool UAtomLockOnComponent::IsTargetValid(AActor* Target) const
{
	if (!Target || Target->IsPendingKillPending()) return false;

	AActor* Owner = GetOwner();
	if (!Owner) return false;

	float Distance = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
	if (Distance > BreakLockOnDistance) return false;

	FGameplayTag TargetableTag = FGameplayTag::RequestGameplayTag(FName("Combat.Targetable"));

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target);
	if (!ASI) return false;

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return false;

	return ASC->HasMatchingGameplayTag(TargetableTag);
}
