// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Tasks/OutlawSTTask_Search.h"
#include "AIController.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FOutlawSTTask_Search::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	InstanceData.ElapsedSearchTime = 0.f;
	InstanceData.bReachedSearchLocation = false;

	const AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (!OwnerActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAIController* AIController = Cast<AAIController>(OwnerActor->GetInstigatorController());
	if (!AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(OwnerActor->GetWorld());
	if (!NavSys)
	{
		return EStateTreeRunStatus::Failed;
	}

	FNavLocation NavResult;
	if (NavSys->GetRandomReachablePointInRadius(InstanceData.LastKnownLocation, InstanceData.SearchRadius, NavResult))
	{
		AIController->MoveToLocation(NavResult.Location, InstanceData.AcceptanceRadius);
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FOutlawSTTask_Search::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	InstanceData.ElapsedSearchTime += DeltaTime;

	if (InstanceData.ElapsedSearchTime >= InstanceData.SearchDuration)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.bReachedSearchLocation)
	{
		const AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
		if (!OwnerActor)
		{
			return EStateTreeRunStatus::Failed;
		}

		AAIController* AIController = Cast<AAIController>(OwnerActor->GetInstigatorController());
		if (!AIController)
		{
			return EStateTreeRunStatus::Failed;
		}

		const UPathFollowingComponent* PathFollowing = AIController->GetPathFollowingComponent();
		if (!PathFollowing)
		{
			return EStateTreeRunStatus::Failed;
		}

		const EPathFollowingStatus::Type MoveStatus = PathFollowing->GetStatus();

		if (MoveStatus == EPathFollowingStatus::Idle)
		{
			InstanceData.bReachedSearchLocation = true;
		}
		else if (MoveStatus != EPathFollowingStatus::Moving && MoveStatus != EPathFollowingStatus::Waiting)
		{
			return EStateTreeRunStatus::Failed;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FOutlawSTTask_Search::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (!OwnerActor)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(OwnerActor->GetInstigatorController());
	if (AIController)
	{
		AIController->StopMovement();
	}
}
