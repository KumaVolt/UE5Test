// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawAbilitySet.h"
#include "AbilitySystemComponent.h"
#include "OutlawGameplayAbility.h"
#include "Engine/DataTable.h"

DEFINE_LOG_CATEGORY_STATIC(LogOutlawAbilitySet, Log, All);

UOutlawAbilitySet::UOutlawAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UOutlawAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC, UObject* SourceObject, FOutlawAbilitySetGrantedHandles& OutHandles) const
{
	if (!ASC)
	{
		UE_LOG(LogOutlawAbilitySet, Error, TEXT("GiveToAbilitySystem called with null ASC. Set: %s"), *GetName());
		return;
	}

	// Grant abilities
	for (const FOutlawAbilityBindInfo& AbilityInfo : Abilities)
	{
		if (!AbilityInfo.AbilityClass)
		{
			UE_LOG(LogOutlawAbilitySet, Error, TEXT("Null ability class in set %s. Skipping."), *GetName());
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilityInfo.AbilityClass, AbilityInfo.AbilityLevel, INDEX_NONE, SourceObject);

		// Tag the spec with the input tag so the ASC can find it later
		if (AbilityInfo.InputTag.IsValid())
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityInfo.InputTag);
		}

		const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AbilitySpec);
		OutHandles.AbilitySpecHandles.Add(Handle);
	}

	// Apply effects
	for (const FOutlawGrantedEffect& EffectInfo : Effects)
	{
		if (!EffectInfo.EffectClass)
		{
			UE_LOG(LogOutlawAbilitySet, Error, TEXT("Null effect class in set %s. Skipping."), *GetName());
			continue;
		}

		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(SourceObject);

		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectInfo.EffectClass, EffectInfo.EffectLevel, EffectContext);
		if (SpecHandle.IsValid())
		{
			const FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			OutHandles.EffectHandles.Add(EffectHandle);
		}
	}

	// Grant attribute sets
	for (const FOutlawGrantedAttributeSet& AttribSetInfo : AttributeSets)
	{
		if (!AttribSetInfo.AttributeSetClass)
		{
			UE_LOG(LogOutlawAbilitySet, Error, TEXT("Null attribute set class in set %s. Skipping."), *GetName());
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(ASC->GetOwner(), AttribSetInfo.AttributeSetClass);
		ASC->AddAttributeSetSubobject(NewSet);
		OutHandles.AttributeSets.Add(NewSet);
	}
}

void UOutlawAbilitySet::PopulateFromDataTable(const UDataTable* DataTable)
{
	if (!DataTable)
	{
		UE_LOG(LogOutlawAbilitySet, Warning, TEXT("PopulateFromDataTable called with null DataTable."));
		return;
	}

	Abilities.Reset();

	TArray<FOutlawAbilityTableRow*> Rows;
	DataTable->GetAllRows<FOutlawAbilityTableRow>(TEXT("PopulateFromDataTable"), Rows);

	for (const FOutlawAbilityTableRow* Row : Rows)
	{
		if (!Row || !Row->AbilityClass)
		{
			continue;
		}

		FOutlawAbilityBindInfo Info;
		Info.AbilityClass = Row->AbilityClass;
		Info.AbilityLevel = Row->AbilityLevel;
		Info.InputTag = Row->InputTag;
		Info.ActivationRequiredTags = Row->ActivationRequiredTags;
		Info.ActivationBlockedTags = Row->ActivationBlockedTags;
		Abilities.Add(Info);
	}
}
