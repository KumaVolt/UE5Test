#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AtomGameplayAbility.h"
#include "OutlawAbility_FireWeapon.generated.h"

UCLASS()
class OUTLAW_API UOutlawAbility_FireWeapon : public UAtomGameplayAbility
{
	GENERATED_BODY()

public:
	UOutlawAbility_FireWeapon();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseRecoilPitch = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseRecoilYaw = 0.3f;

private:
	void FireSingleShot();
	void StopFiring();

	FTimerHandle AutoFireTimer;
	int32 ShotsFired = 0;
	int32 BurstRemaining = 0;
};
