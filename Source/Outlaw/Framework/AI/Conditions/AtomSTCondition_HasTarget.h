// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "OutlawSTCondition_HasTarget.generated.h"

USTRUCT()
struct OUTLAW_API FOutlawSTCondition_HasTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> TargetActor = nullptr;
};

USTRUCT()
struct OUTLAW_API FOutlawSTCondition_HasTarget : public FStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FOutlawSTCondition_HasTargetInstanceData;

	FOutlawSTCondition_HasTarget() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
