#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AtomGameplayAbility.h"
#include "OutlawAbility_MeleeAttack.generated.h"

UCLASS()
class OUTLAW_API UOutlawAbility_MeleeAttack : public UAtomGameplayAbility
{
	GENERATED_BODY()

public:
	UOutlawAbility_MeleeAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float AttackRange = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float AttackDamage = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float AttackRadius = 75.f;
};
