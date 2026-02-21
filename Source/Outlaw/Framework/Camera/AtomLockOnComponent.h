// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "OutlawLockOnComponent.generated.h"

class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockOnTargetChanged, AActor*, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLockOnBroken);

/**
 * Lock-on targeting component for camera focus.
 * Finds nearest targetable enemy via Combat.Targetable tag, camera interpolates to keep target in view.
 */
UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UOutlawLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOutlawLockOnComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void ToggleLockOn();

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void CycleLockOnTarget(bool bNext);

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void BreakLockOn();

	UFUNCTION(BlueprintPure, Category = "LockOn")
	AActor* GetCurrentTarget() const { return CurrentTarget; }

	UFUNCTION(BlueprintPure, Category = "LockOn")
	bool IsLockedOn() const { return CurrentTarget != nullptr; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LockOn|Config")
	float LockOnRange = 1500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LockOn|Config")
	float LockOnFOV = 45.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LockOn|Config")
	bool bLockOnEnabled = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LockOn|Config")
	float BreakLockOnDistance = 2000.f;

	UPROPERTY(BlueprintAssignable, Category = "LockOn")
	FOnLockOnTargetChanged OnLockOnTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "LockOn")
	FOnLockOnBroken OnLockOnBroken;

private:
	UPROPERTY()
	AActor* CurrentTarget = nullptr;

	TArray<AActor*> FindCandidates() const;
	AActor* FindNearestCandidate(const TArray<AActor*>& Candidates) const;
	bool IsTargetValid(AActor* Target) const;
};
