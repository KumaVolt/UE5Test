// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemComponent.h"
#include "OutlawDeathTypes.h"
#include "OutlawDeathComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UOutlawDeathComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOutlawDeathComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintAssignable, Category = "Death")
	FOnDeathStarted OnDeathStarted;

	UPROPERTY(BlueprintAssignable, Category = "Death")
	FOnDeathFinished OnDeathFinished;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	bool bAllowDownState = false;

private:
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
	FDelegateHandle HealthDelegateHandle;
	bool bIsDead = false;
};
