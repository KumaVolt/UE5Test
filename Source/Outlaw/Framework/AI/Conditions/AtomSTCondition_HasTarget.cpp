// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomSTCondition_HasTarget.h"
#include "StateTreeExecutionContext.h"

bool FAtomSTCondition_HasTarget::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	return InstanceData.TargetActor != nullptr && IsValid(InstanceData.TargetActor);
}
