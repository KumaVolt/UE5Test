// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawCombatLogComponent.h"

UOutlawCombatLogComponent::UOutlawCombatLogComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOutlawCombatLogComponent::AddEntry(const FOutlawCombatLogEntry& Entry)
{
	CombatLogEntries.Add(Entry);

	if (CombatLogEntries.Num() > MaxEntries)
	{
		CombatLogEntries.RemoveAt(0);
	}

	OnCombatLogEntryAdded.Broadcast(Entry);
}
