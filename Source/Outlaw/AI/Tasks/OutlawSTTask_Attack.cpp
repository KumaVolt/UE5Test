// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Tasks/OutlawSTTask_Attack.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "GameFramework/Actor.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FOutlawSTTask_Attack::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	InstanceData.TimeSinceLastAttack = InstanceData.Cooldown;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FOutlawSTTask_Attack::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	InstanceData.TimeSinceLastAttack += DeltaTime;

	if (!InstanceData.TargetActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	const AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (!OwnerActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	const float DistanceSq = FVector::DistSquared(OwnerActor->GetActorLocation(), InstanceData.TargetActor->GetActorLocation());
	const float RangeSq = InstanceData.AttackRange * InstanceData.AttackRange;

	if (DistanceSq > RangeSq)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.TimeSinceLastAttack < InstanceData.Cooldown)
	{
		return EStateTreeRunStatus::Running;
	}

	const IAbilitySystemInterface* ASI = Cast<const IAbilitySystemInterface>(OwnerActor);
	if (!ASI)
	{
		return EStateTreeRunStatus::Failed;
	}

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.AbilityTag.IsValid())
	{
		const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(InstanceData.AbilityTag));
		if (bActivated)
		{
			InstanceData.TimeSinceLastAttack = 0.f;
		}
	}

	return EStateTreeRunStatus::Running;
}
