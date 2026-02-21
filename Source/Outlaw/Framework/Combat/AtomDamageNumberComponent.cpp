// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomDamageNumberComponent.h"
#include "AtomCombatTags.h"
#include "AbilitySystem/AtomAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "UI/AtomDamageNumberWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"

UAtomDamageNumberComponent::UAtomDamageNumberComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAtomDamageNumberComponent::BeginPlay()
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
		IncomingDamageDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(
			UAtomAttributeSet::GetIncomingDamageAttribute()).AddUObject(
				this, &UAtomDamageNumberComponent::OnDamageReceived);
	}
}

void UAtomDamageNumberComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundASC.IsValid() && IncomingDamageDelegateHandle.IsValid())
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(
			UAtomAttributeSet::GetIncomingDamageAttribute()).Remove(IncomingDamageDelegateHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UAtomDamageNumberComponent::OnDamageReceived(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.f || !DamageNumberWidgetClass)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->GetWorld())
	{
		return;
	}

	bWasCritical = false;
	if (BoundASC.IsValid())
	{
		if (BoundASC->HasMatchingGameplayTag(AtomCombatTags::CriticalHit))
		{
			bWasCritical = true;
		}
	}

	const FVector SpawnLocation = Owner->GetActorLocation() + SpawnOffset;

	UAtomDamageNumberWidget* Widget = CreateWidget<UAtomDamageNumberWidget>(
		Owner->GetWorld(),
		DamageNumberWidgetClass
	);

	if (Widget)
	{
		Widget->AddToViewport();
		Widget->InitDamageNumber(Data.NewValue, bWasCritical, SpawnLocation);
	}
}
