// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Tasks/OutlawSTTask_Patrol.h"
#include "AIController.h"
#include "GameFramework/Actor.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"

EStateTreeRunStatus FOutlawSTTask_Patrol::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.PatrolPoints.IsEmpty())
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

	if (InstanceData.CurrentPatrolIndex < 0 || InstanceData.CurrentPatrolIndex >= InstanceData.PatrolPoints.Num())
	{
		InstanceData.CurrentPatrolIndex = 0;
	}

	const FVector& TargetLocation = InstanceData.PatrolPoints[InstanceData.CurrentPatrolIndex];
	AIController->MoveToLocation(TargetLocation, InstanceData.AcceptanceRadius);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FOutlawSTTask_Patrol::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

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
		InstanceData.CurrentPatrolIndex = (InstanceData.CurrentPatrolIndex + 1) % InstanceData.PatrolPoints.Num();
		const FVector& NextLocation = InstanceData.PatrolPoints[InstanceData.CurrentPatrolIndex];
		AIController->MoveToLocation(NextLocation, InstanceData.AcceptanceRadius);
	}
	else if (MoveStatus == EPathFollowingStatus::Moving || MoveStatus == EPathFollowingStatus::Waiting)
	{
		return EStateTreeRunStatus::Running;
	}
	else
	{
		const FVector& CurrentLocation = InstanceData.PatrolPoints[InstanceData.CurrentPatrolIndex];
		AIController->MoveToLocation(CurrentLocation, InstanceData.AcceptanceRadius);
	}

	return EStateTreeRunStatus::Running;
}
