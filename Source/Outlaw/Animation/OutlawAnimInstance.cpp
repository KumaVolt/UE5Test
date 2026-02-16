// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/OutlawAnimInstance.h"
#include "Animation/OutlawAnimationTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "KismetAnimationLibrary.h"

void UOutlawAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwningCharacter)
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OwningCharacter))
		{
			AbilitySystemComponent = ASI->GetAbilitySystemComponent();
		}
		else if (APlayerState* PS = OwningCharacter->GetPlayerState())
		{
			if (IAbilitySystemInterface* PSI = Cast<IAbilitySystemInterface>(PS))
			{
				AbilitySystemComponent = PSI->GetAbilitySystemComponent();
			}
		}
	}
}

void UOutlawAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter)
	{
		return;
	}

	Speed = OwningCharacter->GetVelocity().Size2D();
    Direction = UKismetAnimationLibrary::CalculateDirection(OwningCharacter->GetVelocity(), OwningCharacter->GetActorRotation());
	bIsInAir = OwningCharacter->GetMovementComponent()->IsFalling();

	if (AController* Controller = OwningCharacter->GetController())
	{
		FRotator ControlRotation = Controller->GetControlRotation();
		AimPitch = ControlRotation.Pitch;
		AimYaw = ControlRotation.Yaw;
	}

	if (AbilitySystemComponent)
	{
		bIsDead = AbilitySystemComponent->HasMatchingGameplayTag(OutlawAnimTags::Dead);
		bIsStaggered = AbilitySystemComponent->HasMatchingGameplayTag(OutlawAnimTags::Staggered);
	}
}

void UOutlawAnimInstance::PlayAbilityMontage(UAnimMontage* Montage, float Rate, FName Section)
{
	if (!Montage)
	{
		return;
	}

	Montage_Play(Montage, Rate);
	if (Section != NAME_None)
	{
		Montage_JumpToSection(Section, Montage);
	}
}
