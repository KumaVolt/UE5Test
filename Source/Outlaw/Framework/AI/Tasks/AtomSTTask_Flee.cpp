// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Tasks/OutlawSTTask_Flee.h"
#include "AIController.h"
#include "GameFramework/Actor.h"
#include "StateTreeExecutionContext.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FOutlawSTTask_Flee::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.ThreatActor)
	{
		return EStateTreeRunStatus::Failed;
	}

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

	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const FVector ThreatLocation = InstanceData.ThreatActor->GetActorLocation();
	const FVector FleeDirection = (OwnerLocation - ThreatLocation).GetSafeNormal();
	const FVector FleeTarget = OwnerLocation + FleeDirection * InstanceData.FleeDistance;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(OwnerActor->GetWorld());
	if (!NavSys)
	{
		return EStateTreeRunStatus::Failed;
	}

	FNavLocation NavResult;
	if (NavSys->GetRandomReachablePointInRadius(FleeTarget, 500.f, NavResult))
	{
		AIController->MoveToLocation(NavResult.Location, InstanceData.AcceptanceRadius);
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FOutlawSTTask_Flee::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.ThreatActor)
	{
		return EStateTreeRunStatus::Failed;
	}

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
		return EStateTreeRunStatus::Succeeded;
	}
	else if (MoveStatus == EPathFollowingStatus::Moving || MoveStatus == EPathFollowingStatus::Waiting)
	{
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Failed;
}

void FOutlawSTTask_Flee::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
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
