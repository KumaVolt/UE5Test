// Fill out your copyright notice in the Description page of Project Settings.


#include "OutlawEnemyCharacter.h"
#include "AbilitySystem/OutlawAbilitySystemComponent.h"
#include "AI/OutlawAIController.h"

// Sets default values
AOutlawEnemyCharacter::AOutlawEnemyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	
	AbilitySystemComponent = CreateDefaultSubobject<UOutlawAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AIControllerClass = AOutlawAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void AOutlawEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	GrantDefaultAbilitySet();
}
