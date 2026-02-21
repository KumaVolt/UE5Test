// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "AtomSTCondition_HealthThreshold.generated.h"

USTRUCT()
struct OUTLAW_API FAtomSTCondition_HealthThresholdInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float ThresholdPercent = 30.f;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> OwnerActor = nullptr;
};

USTRUCT()
struct OUTLAW_API FAtomSTCondition_HealthThreshold : public FStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAtomSTCondition_HealthThresholdInstanceData;

	FAtomSTCondition_HealthThreshold() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
