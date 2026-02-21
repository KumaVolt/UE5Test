// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateTreeTaskBase.h"
#include "AtomSTTask_Attack.generated.h"

USTRUCT()
struct OUTLAW_API FAtomSTTask_AttackInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AttackRange = 200.f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float Cooldown = 1.f;

	UPROPERTY()
	float TimeSinceLastAttack = 0.f;
};

USTRUCT()
struct OUTLAW_API FAtomSTTask_Attack : public FStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAtomSTTask_AttackInstanceData;

	FAtomSTTask_Attack() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
