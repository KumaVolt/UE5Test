// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/OutlawStatBar.h"
#include "AbilitySystemComponent.h"

void UOutlawStatBar::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	if (!ASC || !AttributeToTrack.IsValid() || !MaxAttributeToTrack.IsValid())
	{
		return;
	}

	// Unbind from any previous ASC
	if (BoundASC.IsValid())
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(AttributeToTrack).Remove(AttributeDelegateHandle);
		BoundASC->GetGameplayAttributeValueChangeDelegate(MaxAttributeToTrack).Remove(MaxAttributeDelegateHandle);
	}

	BoundASC = ASC;

	// Bind to attribute change delegates
	AttributeDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeToTrack)
		.AddUObject(this, &UOutlawStatBar::HandleAttributeChanged);

	MaxAttributeDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(MaxAttributeToTrack)
		.AddUObject(this, &UOutlawStatBar::HandleMaxAttributeChanged);

	// Read initial values
	bool bFound = false;
	CachedCurrentValue = ASC->GetGameplayAttributeValue(AttributeToTrack, bFound);
	CachedMaxValue = ASC->GetGameplayAttributeValue(MaxAttributeToTrack, bFound);

	BroadcastStatChanged();
}

void UOutlawStatBar::NativeDestruct()
{
	if (BoundASC.IsValid())
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(AttributeToTrack).Remove(AttributeDelegateHandle);
		BoundASC->GetGameplayAttributeValueChangeDelegate(MaxAttributeToTrack).Remove(MaxAttributeDelegateHandle);
	}

	Super::NativeDestruct();
}

void UOutlawStatBar::HandleAttributeChanged(const FOnAttributeChangeData& Data)
{
	CachedCurrentValue = Data.NewValue;
	BroadcastStatChanged();
}

void UOutlawStatBar::HandleMaxAttributeChanged(const FOnAttributeChangeData& Data)
{
	CachedMaxValue = Data.NewValue;
	BroadcastStatChanged();
}

void UOutlawStatBar::BroadcastStatChanged()
{
	const float Percent = CachedMaxValue > 0.f ? FMath::Clamp(CachedCurrentValue / CachedMaxValue, 0.f, 1.f) : 0.f;
	OnStatChanged(CachedCurrentValue, CachedMaxValue, Percent);
}
