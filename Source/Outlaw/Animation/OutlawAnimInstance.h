// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "OutlawAnimInstance.generated.h"

class UAnimMontage;
class ACharacter;
class UAbilitySystemComponent;

UCLASS()
class OUTLAW_API UOutlawAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "Outlaw|Animation")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Outlaw|Animation")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Outlaw|Animation")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadOnly, Category = "Outlaw|Animation")
	float AimPitch = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Outlaw|Animation")
	float AimYaw = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Outlaw|Animation")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category = "Outlaw|Animation")
	bool bIsStaggered = false;

	UFUNCTION(BlueprintCallable, Category = "Outlaw|Animation")
	void PlayAbilityMontage(UAnimMontage* Montage, float Rate = 1.f, FName Section = NAME_None);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Outlaw|Animation")
	TObjectPtr<ACharacter> OwningCharacter = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Outlaw|Animation")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;
};
