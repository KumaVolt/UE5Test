// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomSTCondition_InRange.h"
#include "StateTreeExecutionContext.h"

bool FAtomSTCondition_InRange::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.OwnerActor || !InstanceData.TargetActor)
	{
		return false;
	}

	const float DistanceSq = FVector::DistSquared(
		InstanceData.OwnerActor->GetActorLocation(),
		InstanceData.TargetActor->GetActorLocation()
	);

	const float RangeSq = InstanceData.Range * InstanceData.Range;
	return DistanceSq <= RangeSq;
}
