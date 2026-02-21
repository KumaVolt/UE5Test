// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "AbilitySystemComponent.h"
#include "AtomStatBar.generated.h"

UCLASS()
class OUTLAW_API UAtomStatBar : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Bar")
	FGameplayAttribute AttributeToTrack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Bar")
	FGameplayAttribute MaxAttributeToTrack;

	UFUNCTION(BlueprintCallable, Category = "Stat Bar")
	void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Stat Bar")
	void OnStatChanged(float CurrentValue, float MaxValue, float Percent);

	virtual void NativeDestruct() override;

private:
	void HandleAttributeChanged(const FOnAttributeChangeData& Data);
	void HandleMaxAttributeChanged(const FOnAttributeChangeData& Data);
	void BroadcastStatChanged();

	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
	FDelegateHandle AttributeDelegateHandle;
	FDelegateHandle MaxAttributeDelegateHandle;

	float CachedCurrentValue = 0.f;
	float CachedMaxValue = 0.f;
};
