// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Inventory/OutlawItemDefinition.h"
#include "OutlawLootBeamComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UOutlawLootBeamComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UOutlawLootBeamComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot Beam")
	TSoftObjectPtr<UNiagaraSystem> BeamEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot Beam")
	TMap<EOutlawItemRarity, FLinearColor> RarityColors;

	UFUNCTION(BlueprintCallable, Category = "Loot Beam")
	void InitForRarity(EOutlawItemRarity Rarity);

private:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> BeamComponent;
};
