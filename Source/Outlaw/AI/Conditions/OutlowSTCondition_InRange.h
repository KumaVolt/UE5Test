// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "OutlowSTCondition_InRange.generated.h"

USTRUCT()
struct OUTLAW_API FOutlowSTCondition_InRangeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float Range = 500.f;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> OwnerActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> TargetActor = nullptr;
};

USTRUCT()
struct OUTLAW_API FOutlowSTCondition_InRange : public FStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FOutlowSTCondition_InRangeInstanceData;

	FOutlowSTCondition_InRange() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
