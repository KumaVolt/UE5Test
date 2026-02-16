// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawDamageNumberComponent.h"
#include "OutlawCombatTags.h"
#include "AbilitySystem/OutlawAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "UI/OutlawDamageNumberWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"

UOutlawDamageNumberComponent::UOutlawDamageNumberComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOutlawDamageNumberComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UAbilitySystemComponent* ASC = nullptr;
	if (Owner->Implements<UAbilitySystemInterface>())
	{
		ASC = IAbilitySystemInterface::Execute_GetAbilitySystemComponent(Owner);
	}

	if (!ASC && Owner->GetInstigator())
	{
		AActor* Instigator = Owner->GetInstigator();
		if (Instigator && Instigator->Implements<UAbilitySystemInterface>())
		{
			ASC = IAbilitySystemInterface::Execute_GetAbilitySystemComponent(Instigator);
		}
	}

	if (ASC)
	{
		BoundASC = ASC;
		IncomingDamageDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(
			UOutlawAttributeSet::GetIncomingDamageAttribute()).AddUObject(
				this, &UOutlawDamageNumberComponent::OnDamageReceived);
	}
}

void UOutlawDamageNumberComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundASC.IsValid() && IncomingDamageDelegateHandle.IsValid())
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(
			UOutlawAttributeSet::GetIncomingDamageAttribute()).Remove(IncomingDamageDelegateHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UOutlawDamageNumberComponent::OnDamageReceived(const FOnAttributeChangeData& Data)
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
		if (BoundASC->HasMatchingGameplayTag(OutlawCombatTags::CriticalHit))
		{
			bWasCritical = true;
		}
	}

	const FVector SpawnLocation = Owner->GetActorLocation() + SpawnOffset;

	UOutlawDamageNumberWidget* Widget = CreateWidget<UOutlawDamageNumberWidget>(
		Owner->GetWorld(),
		DamageNumberWidgetClass
	);

	if (Widget)
	{
		Widget->AddToViewport();
		Widget->InitDamageNumber(Data.NewValue, bWasCritical, SpawnLocation);
	}
}
