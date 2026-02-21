#include "AbilitySystem/Abilities/OutlawAbility_FireWeapon.h"
#include "AbilitySystem/OutlawAbilitySystemComponent.h"
#include "AbilitySystem/Effects/OutlawEffect_Damage.h"
#include "AbilitySystem/OutlawWeaponAttributeSet.h"
#include "Weapon/OutlawWeaponManagerComponent.h"
#include "Inventory/OutlawItemInstance.h"
#include "Inventory/OutlawItemDefinition.h"
#include "Weapon/OutlawShooterWeaponData.h"
#include "Projectile/OutlawHitscanLibrary.h"
#include "Camera/OutlawCameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"

UOutlawAbility_FireWeapon::UOutlawAbility_FireWeapon()
{
	ActivationPolicy = EOutlawAbilityActivationPolicy::OnInputTriggered;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UOutlawAbility_FireWeapon::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Dead"))))
		{
			return false;
		}
	}

	return true;
}

void UOutlawAbility_FireWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	UOutlawWeaponManagerComponent* WeaponMgr = AvatarActor->FindComponentByClass<UOutlawWeaponManagerComponent>();
	if (!WeaponMgr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UOutlawItemInstance* ActiveWeapon = WeaponMgr->GetActiveWeapon();
	if (!ActiveWeapon || !ActiveWeapon->ItemDef || !ActiveWeapon->ItemDef->ShooterWeaponData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ShotsFired = 0;
	BurstRemaining = 0;
	const UOutlawShooterWeaponData* WeaponData = ActiveWeapon->ItemDef->ShooterWeaponData;

	// Fire first shot immediately
	FireSingleShot();

	// Check if ability already ended (e.g. out of ammo on first shot check inside FireSingleShot)
	if (!IsActive())
	{
		return;
	}

	if (WeaponData->bAutomatic)
	{
		// Full-auto: loop until trigger release or ammo runs out
		float TimerInterval = 60.f / FMath::Max(WeaponData->RPM, 1.f);
		GetWorld()->GetTimerManager().SetTimer(
			AutoFireTimer,
			this,
			&UOutlawAbility_FireWeapon::FireSingleShot,
			TimerInterval,
			true
		);
	}
	else if (WeaponData->BurstCount > 1)
	{
		// Burst fire: fire remaining burst shots then end
		BurstRemaining = WeaponData->BurstCount - 1;
		float TimerInterval = 60.f / FMath::Max(WeaponData->RPM, 1.f);
		GetWorld()->GetTimerManager().SetTimer(
			AutoFireTimer,
			this,
			&UOutlawAbility_FireWeapon::FireSingleShot,
			TimerInterval,
			true
		);
	}
	else
	{
		// Single shot: end immediately
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UOutlawAbility_FireWeapon::FireSingleShot()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo)
	{
		StopFiring();
		return;
	}

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor)
	{
		StopFiring();
		return;
	}

	UOutlawWeaponManagerComponent* WeaponMgr = AvatarActor->FindComponentByClass<UOutlawWeaponManagerComponent>();
	if (!WeaponMgr)
	{
		StopFiring();
		return;
	}

	UOutlawItemInstance* ActiveWeapon = WeaponMgr->GetActiveWeapon();
	if (!ActiveWeapon || ActiveWeapon->CurrentAmmo <= 0)
	{
		StopFiring();
		return;
	}

	ActiveWeapon->CurrentAmmo--;

	ACharacter* Character = Cast<ACharacter>(AvatarActor);
	if (!Character || !Character->GetController())
	{
		StopFiring();
		return;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	Character->GetController()->GetPlayerViewPoint(EyeLocation, EyeRotation);

	const FVector Direction = EyeRotation.Vector();
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	float WeaponRange = 5000.f;
	float Accuracy = 80.f;
	float Stability = 70.f;
	if (const UOutlawWeaponAttributeSet* WeaponAttrs = ASC ? ASC->GetSet<UOutlawWeaponAttributeSet>() : nullptr)
	{
		WeaponRange = WeaponAttrs->GetWeaponRange();
		Accuracy = WeaponAttrs->GetAccuracy();
		Stability = WeaponAttrs->GetStability();
	}

	// Spread: base from accuracy + grows during sustained fire
	float SpreadAngle = (100.f - Accuracy) * 0.05f + ShotsFired * 0.002f;

	UOutlawHitscanLibrary::FireHitscan(
		AvatarActor,
		ASC,
		EyeLocation,
		Direction,
		WeaponRange,
		UOutlawEffect_Damage::StaticClass(),
		1,
		0,
		FMath::Max(SpreadAngle, 0.f)
	);

	// Apply recoil scaled by stability
	if (UOutlawCameraComponent* Camera = AvatarActor->FindComponentByClass<UOutlawCameraComponent>())
	{
		float RecoilScale = (100.f - Stability) / 100.f;
		Camera->ApplyRecoil(
			BaseRecoilPitch * RecoilScale,
			FMath::RandRange(-BaseRecoilYaw, BaseRecoilYaw) * RecoilScale
		);
	}

	ShotsFired++;

	// Handle burst mode countdown
	if (BurstRemaining > 0)
	{
		BurstRemaining--;
		if (BurstRemaining <= 0)
		{
			StopFiring();
		}
	}
}

void UOutlawAbility_FireWeapon::StopFiring()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoFireTimer);
	}
	ShotsFired = 0;
	BurstRemaining = 0;

	if (IsActive())
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
	}
}

void UOutlawAbility_FireWeapon::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// For auto weapons: stop firing when trigger is released
	// For burst: let the burst complete (BurstRemaining > 0)
	// For semi-auto: ability already ended, this is a no-op
	if (BurstRemaining <= 0)
	{
		StopFiring();
	}
}

void UOutlawAbility_FireWeapon::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoFireTimer);
	}
	ShotsFired = 0;
	BurstRemaining = 0;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
