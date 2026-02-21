// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "AtomSTTask_Search.generated.h"

USTRUCT()
struct OUTLAW_API FAtomSTTask_SearchInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	FVector LastKnownLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float SearchRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float SearchDuration = 5.f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 100.f;

	UPROPERTY()
	float ElapsedSearchTime = 0.f;

	UPROPERTY()
	bool bReachedSearchLocation = false;
};

USTRUCT()
struct OUTLAW_API FAtomSTTask_Search : public FStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAtomSTTask_SearchInstanceData;

	FAtomSTTask_Search() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
