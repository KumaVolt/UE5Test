// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OutlawDeathTypes.h"
#include "OutlawCombatLogComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UOutlawCombatLogComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOutlawCombatLogComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "CombatLog")
	void AddEntry(const FOutlawCombatLogEntry& Entry);

	UFUNCTION(BlueprintCallable, Category = "CombatLog")
	const TArray<FOutlawCombatLogEntry>& GetEntries() const { return CombatLogEntries; }

	UPROPERTY(BlueprintAssignable, Category = "CombatLog")
	FOnCombatLogEntryAdded OnCombatLogEntryAdded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatLog")
	int32 MaxEntries = 100;

private:
	UPROPERTY()
	TArray<FOutlawCombatLogEntry> CombatLogEntries;
};
