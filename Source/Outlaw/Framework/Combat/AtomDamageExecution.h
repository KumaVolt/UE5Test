// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "OutlawDamageExecution.generated.h"

UCLASS()
class OUTLAW_API UOutlawDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UOutlawDamageExecution();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	                                    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
