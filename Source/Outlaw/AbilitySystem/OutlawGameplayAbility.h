// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "OutlawGameplayAbility.generated.h"

/**
 * How this ability is activated.
 */
UENUM(BlueprintType)
enum class EOutlawAbilityActivationPolicy : uint8
{
	/** Activated when the bound input is triggered. */
	OnInputTriggered,

	/** Activated immediately when granted (passive). */
	OnGranted,

	/** Activated when a matching gameplay event is received. */
	OnGameplayEvent,
};

/**
 * Base gameplay ability class for the Outlaw project.
 * All project abilities should derive from this.
 */
UCLASS(Abstract)
class OUTLAW_API UOutlawGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UOutlawGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** How this ability is activated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Outlaw|Activation")
	EOutlawAbilityActivationPolicy ActivationPolicy = EOutlawAbilityActivationPolicy::OnInputTriggered;

	/** Returns the activation policy. */
	EOutlawAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	/** Try to activate this ability if the policy is OnGranted. */
	void TryActivateOnGranted(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec);
};
