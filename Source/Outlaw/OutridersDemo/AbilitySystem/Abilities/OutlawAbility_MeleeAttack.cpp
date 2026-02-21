#include "AbilitySystem/Abilities/OutlawAbility_MeleeAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/AtomAttributeSet.h"
#include "GameFramework/Character.h"

UOutlawAbility_MeleeAttack::UOutlawAbility_MeleeAttack()
{
	ActivationPolicy = EAtomAbilityActivationPolicy::OnInputTriggered;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UOutlawAbility_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector Start = AvatarActor->GetActorLocation();
	const FVector End = Start + AvatarActor->GetActorForwardVector() * AttackRange;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);

	TArray<FHitResult> Hits;
	AvatarActor->GetWorld()->SweepMultiByChannel(
		Hits,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(AttackRadius),
		QueryParams
	);

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor)
		{
			continue;
		}

		IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(HitActor);
		if (!ASI)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent();
		if (TargetASC)
		{
			// Directly reduce Health — ApplyModToAttribute fires the attribute change delegate
			// which DeathComponent monitors. Using IncomingDamage would require a GE to trigger
			// PostGameplayEffectExecute, so we modify Health directly instead.
			TargetASC->ApplyModToAttribute(UAtomAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -AttackDamage);
		}
		break;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
