// Fill out your copyright notice in the Description page of Project Settings.


#include "OutlawEnemyCharacter.h"
#include "AbilitySystem/OutlawAbilitySystemComponent.h"

// Sets default values
AOutlawEnemyCharacter::AOutlawEnemyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	AbilitySystemComponent = CreateDefaultSubobject<UOutlawAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

}

// Called when the game starts or when spawned
void AOutlawEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	GrantDefaultAbilitySet();
}
