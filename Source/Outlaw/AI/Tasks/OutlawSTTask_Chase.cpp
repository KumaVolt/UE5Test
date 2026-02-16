// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Tasks/OutlawSTTask_Chase.h"
#include "AIController.h"
#include "GameFramework/Actor.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"

EStateTreeRunStatus FOutlawSTTask_Chase::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.TargetActor)
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

	InstanceData.LastKnownLocation = InstanceData.TargetActor->GetActorLocation();
	AIController->MoveToActor(InstanceData.TargetActor, InstanceData.AcceptanceRadius);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FOutlawSTTask_Chase::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.TargetActor)
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

	InstanceData.LastKnownLocation = InstanceData.TargetActor->GetActorLocation();

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

void FOutlawSTTask_Chase::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
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
