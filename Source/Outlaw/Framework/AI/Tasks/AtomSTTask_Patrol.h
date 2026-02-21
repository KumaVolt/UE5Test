// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "AtomSTTask_Patrol.generated.h"

USTRUCT()
struct OUTLAW_API FAtomSTTask_PatrolInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Parameter")
	TArray<FVector> PatrolPoints;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 100.f;

	UPROPERTY(EditAnywhere, Category = "Context")
	int32 CurrentPatrolIndex = 0;
};

USTRUCT()
struct OUTLAW_API FAtomSTTask_Patrol : public FStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAtomSTTask_PatrolInstanceData;

	FAtomSTTask_Patrol() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
