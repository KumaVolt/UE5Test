// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "AtomSTCondition_HasTarget.generated.h"

USTRUCT()
struct OUTLAW_API FAtomSTCondition_HasTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> TargetActor = nullptr;
};

USTRUCT()
struct OUTLAW_API FAtomSTCondition_HasTarget : public FStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAtomSTCondition_HasTargetInstanceData;

	FAtomSTCondition_HasTarget() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
