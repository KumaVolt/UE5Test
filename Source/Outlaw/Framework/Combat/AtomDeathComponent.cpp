// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawDeathComponent.h"
#include "AbilitySystem/OutlawAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Animation/OutlawAnimationTypes.h"

UOutlawDeathComponent::UOutlawDeathComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOutlawDeathComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UAbilitySystemComponent* ASC = nullptr;
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		ASC = ASI->GetAbilitySystemComponent();
	}

	if (!ASC && Owner->GetInstigator())
	{
		AActor* Instigator = Owner->GetInstigator();
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Instigator))
		{
			ASC = ASI->GetAbilitySystemComponent();
		}
	}

	if (ASC)
	{
		BoundASC = ASC;
		HealthDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(
			UOutlawAttributeSet::GetHealthAttribute()).AddUObject(
				this, &UOutlawDeathComponent::OnHealthChanged);
	}
}

void UOutlawDeathComponent::BindToAbilitySystem(UAbilitySystemComponent* ASC)
{
	if (!ASC || BoundASC.IsValid())
	{
		return;
	}

	BoundASC = ASC;
	HealthDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(
		UOutlawAttributeSet::GetHealthAttribute()).AddUObject(
			this, &UOutlawDeathComponent::OnHealthChanged);
}

void UOutlawDeathComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundASC.IsValid() && HealthDelegateHandle.IsValid())
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(
			UOutlawAttributeSet::GetHealthAttribute()).Remove(HealthDelegateHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UOutlawDeathComponent::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue > 0.f || bIsDead)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	bIsDead = true;

	// Note: Cannot access GEModData from attribute change delegate (forward-declared type)
	// Killer info must be passed via alternate means if needed
	AActor* Killer = nullptr;

	if (BoundASC.IsValid())
	{
		BoundASC->CancelAllAbilities();
		BoundASC->AddLooseGameplayTag(OutlawAnimTags::Dead);
	}

	OnDeathStarted.Broadcast(Killer);

	if (ACharacter* Character = Cast<ACharacter>(Owner))
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Movement->DisableMovement();
		}

		if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
		{
			Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
			Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		}

		if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
		{
			Character->DisableInput(PC);
		}
	}

	OnDeathFinished.Broadcast(Owner);
}
