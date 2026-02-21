// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "AtomSTTask_Chase.generated.h"

USTRUCT()
struct OUTLAW_API FAtomSTTask_ChaseInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 100.f;

	UPROPERTY()
	FVector LastKnownLocation = FVector::ZeroVector;
};

USTRUCT()
struct OUTLAW_API FAtomSTTask_Chase : public FStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAtomSTTask_ChaseInstanceData;

	FAtomSTTask_Chase() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
