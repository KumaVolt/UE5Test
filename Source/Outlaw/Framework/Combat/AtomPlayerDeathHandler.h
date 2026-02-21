// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AtomPlayerDeathHandler.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UAtomPlayerDeathHandler : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtomPlayerDeathHandler(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Death")
	void SetCheckpoint(FVector Location, FRotator Rotation);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	float RespawnDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	TSubclassOf<UUserWidget> DeathScreenWidgetClass;

private:
	UFUNCTION()
	void OnDeathStarted(AActor* Killer);

	void RespawnAtCheckpoint();

	FVector CheckpointLocation = FVector::ZeroVector;
	FRotator CheckpointRotation = FRotator::ZeroRotator;
	FTimerHandle RespawnTimerHandle;
	TObjectPtr<UUserWidget> DeathScreenWidget;
};
