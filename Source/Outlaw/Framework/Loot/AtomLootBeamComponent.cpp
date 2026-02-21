// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomLootBeamComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

UAtomLootBeamComponent::UAtomLootBeamComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	RarityColors.Add(EAtomItemRarity::Common, FLinearColor::White);
	RarityColors.Add(EAtomItemRarity::Uncommon, FLinearColor::Green);
	RarityColors.Add(EAtomItemRarity::Rare, FLinearColor(0.0f, 0.5f, 1.0f));
	RarityColors.Add(EAtomItemRarity::Epic, FLinearColor(0.6f, 0.0f, 1.0f));
	RarityColors.Add(EAtomItemRarity::Legendary, FLinearColor(1.0f, 0.5f, 0.0f));
}

void UAtomLootBeamComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAtomLootBeamComponent::InitForRarity(EAtomItemRarity Rarity)
{
	if (!BeamEffect.IsNull())
	{
		UNiagaraSystem* LoadedEffect = BeamEffect.LoadSynchronous();
		if (LoadedEffect)
		{
			if (!BeamComponent)
			{
				BeamComponent = NewObject<UNiagaraComponent>(this, TEXT("BeamNiagaraComponent"));
				BeamComponent->SetupAttachment(this);
				BeamComponent->RegisterComponent();
			}

			BeamComponent->SetAsset(LoadedEffect);

			if (const FLinearColor* Color = RarityColors.Find(Rarity))
			{
				BeamComponent->SetVariableLinearColor(FName("BeamColor"), *Color);
			}

			BeamComponent->Activate(true);
		}
	}
}
