// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Inventory/AtomItemDefinition.h"
#include "AtomLootBeamComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UAtomLootBeamComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAtomLootBeamComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot Beam")
	TSoftObjectPtr<UNiagaraSystem> BeamEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot Beam")
	TMap<EAtomItemRarity, FLinearColor> RarityColors;

	UFUNCTION(BlueprintCallable, Category = "Loot Beam")
	void InitForRarity(EAtomItemRarity Rarity);

private:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> BeamComponent;
};
