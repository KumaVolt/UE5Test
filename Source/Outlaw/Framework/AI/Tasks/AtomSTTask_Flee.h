// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "OutlawSTTask_Flee.generated.h"

USTRUCT()
struct OUTLAW_API FOutlawSTTask_FleeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> ThreatActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float FleeDistance = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 100.f;
};

USTRUCT()
struct OUTLAW_API FOutlawSTTask_Flee : public FStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FOutlawSTTask_FleeInstanceData;

	FOutlawSTTask_Flee() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
