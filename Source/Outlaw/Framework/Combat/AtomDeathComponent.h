// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemComponent.h"
#include "AtomDeathTypes.h"
#include "AtomDeathComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UAtomDeathComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtomDeathComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintAssignable, Category = "Death")
	FOnDeathStarted OnDeathStarted;

	UPROPERTY(BlueprintAssignable, Category = "Death")
	FOnDeathFinished OnDeathFinished;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	bool bAllowDownState = false;

	/** Manually bind to an ASC (for deferred init, e.g. player ASC on PlayerState). */
	void BindToAbilitySystem(UAbilitySystemComponent* ASC);

private:
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
	FDelegateHandle HealthDelegateHandle;
	bool bIsDead = false;
};
