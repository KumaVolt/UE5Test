// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomCombatLogComponent.h"

UAtomCombatLogComponent::UAtomCombatLogComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAtomCombatLogComponent::AddEntry(const FAtomCombatLogEntry& Entry)
{
	CombatLogEntries.Add(Entry);

	if (CombatLogEntries.Num() > MaxEntries)
	{
		CombatLogEntries.RemoveAt(0);
	}

	OnCombatLogEntryAdded.Broadcast(Entry);
}
