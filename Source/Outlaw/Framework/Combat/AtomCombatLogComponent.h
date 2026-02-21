// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AtomDeathTypes.h"
#include "AtomCombatLogComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UAtomCombatLogComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtomCombatLogComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "CombatLog")
	void AddEntry(const FAtomCombatLogEntry& Entry);

	UFUNCTION(BlueprintCallable, Category = "CombatLog")
	const TArray<FAtomCombatLogEntry>& GetEntries() const { return CombatLogEntries; }

	UPROPERTY(BlueprintAssignable, Category = "CombatLog")
	FOnCombatLogEntryAdded OnCombatLogEntryAdded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatLog")
	int32 MaxEntries = 100;

private:
	UPROPERTY()
	TArray<FAtomCombatLogEntry> CombatLogEntries;
};
