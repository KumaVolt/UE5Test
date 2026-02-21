// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Combat/OutlawCombatTypes.h"
#include "OutlawStatusEffectComponent.generated.h"

class UAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UOutlawStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOutlawStatusEffectComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Combat|StatusEffects")
	TArray<FOutlawActiveStatusEffect> GetActiveStatusEffects() const;

	UFUNCTION(BlueprintCallable, Category = "Combat|StatusEffects")
	bool HasStatusEffect(FGameplayTag StatusTag) const;

	UPROPERTY(BlueprintAssignable, Category = "Combat|StatusEffects")
	FOnStatusEffectAdded OnStatusEffectAdded;

	UPROPERTY(BlueprintAssignable, Category = "Combat|StatusEffects")
	FOnStatusEffectRemoved OnStatusEffectRemoved;

private:
	void OnStatusTagChanged(const FGameplayTag StatusTag, int32 NewCount);

	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
	TArray<FOutlawActiveStatusEffect> ActiveEffects;
};
