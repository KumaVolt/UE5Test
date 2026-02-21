// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawLootBeamComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

UOutlawLootBeamComponent::UOutlawLootBeamComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	RarityColors.Add(EOutlawItemRarity::Common, FLinearColor::White);
	RarityColors.Add(EOutlawItemRarity::Uncommon, FLinearColor::Green);
	RarityColors.Add(EOutlawItemRarity::Rare, FLinearColor(0.0f, 0.5f, 1.0f));
	RarityColors.Add(EOutlawItemRarity::Epic, FLinearColor(0.6f, 0.0f, 1.0f));
	RarityColors.Add(EOutlawItemRarity::Legendary, FLinearColor(1.0f, 0.5f, 0.0f));
}

void UOutlawLootBeamComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UOutlawLootBeamComponent::InitForRarity(EOutlawItemRarity Rarity)
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
