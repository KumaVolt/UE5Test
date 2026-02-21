// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawEnemyCharacter.h"
#include "AbilitySystem/OutlawAbilitySystemComponent.h"
#include "AI/OutlawAIController.h"
#include "AI/OutlawDemoAIBehavior.h"
#include "Combat/OutlawDeathComponent.h"
#include "Combat/OutlawEnemyDeathHandler.h"
#include "Combat/OutlawDamageNumberComponent.h"
#include "Animation/OutlawHitReactionComponent.h"
#include "Combat/OutlawStatusEffectComponent.h"
#include "Game/OutlawDemoAbilitySets.h"
#include "Game/OutlawDemoDataSubsystem.h"
#include "UI/OutlawDemoDamageNumber.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

AOutlawEnemyCharacter::AOutlawEnemyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	// ASC on enemy (Minimal replication)
	AbilitySystemComponent = CreateDefaultSubobject<UOutlawAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// AI
	AIControllerClass = AOutlawAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Combat components
	DeathComponent = CreateDefaultSubobject<UOutlawDeathComponent>(TEXT("Death"));

	EnemyDeathHandler = CreateDefaultSubobject<UOutlawEnemyDeathHandler>(TEXT("EnemyDeathHandler"));
	EnemyDeathHandler->BaseXPReward = 50;
	EnemyDeathHandler->NumLootDrops = 1;

	DamageNumberComponent = CreateDefaultSubobject<UOutlawDamageNumberComponent>(TEXT("DamageNumber"));
	DamageNumberComponent->DamageNumberWidgetClass = UOutlawDemoDamageNumber::StaticClass();

	HitReactionComponent = CreateDefaultSubobject<UOutlawHitReactionComponent>(TEXT("HitReaction"));
	HitReactionComponent->bCanBeStaggered = true;

	StatusEffectComponent = CreateDefaultSubobject<UOutlawStatusEffectComponent>(TEXT("StatusEffects"));

	// AI Behavior (replaces StateTree)
	AIBehaviorComponent = CreateDefaultSubobject<UOutlawDemoAIBehavior>(TEXT("AIBehavior"));

	// Default ability set — use CDO of EnemyDefault
	DefaultAbilitySet = GetMutableDefault<UOutlawAbilitySet_EnemyDefault>();

	// Visual body stand-in (no skeletal mesh)
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetRelativeScale3D(FVector(0.68f, 0.68f, 1.76f));
	BodyMesh->CastShadow = true;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderFinder.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderFinder.Object);
	}
}

void AOutlawEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Color the body mesh red for enemies
	if (BodyMesh)
	{
		UMaterialInstanceDynamic* DynMat = BodyMesh->CreateDynamicMaterialInstance(0);
		if (DynMat)
		{
			FLinearColor Red(0.8f, 0.1f, 0.1f);
			DynMat->SetVectorParameterValue(TEXT("Color"), Red);
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), Red);
			DynMat->SetVectorParameterValue(TEXT("Base Color"), Red);
		}
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	GrantDefaultAbilitySet();

	// Add Combat.Targetable tag so LockOn can find us
	AbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Combat.Targetable")));

	// Wire up loot table from demo data subsystem
	if (HasAuthority())
	{
		UOutlawDemoDataSubsystem* DataSub = GetWorld()->GetSubsystem<UOutlawDemoDataSubsystem>();
		if (DataSub && EnemyDeathHandler)
		{
			EnemyDeathHandler->LootTable = DataSub->GetBasicEnemyLootTable();
		}
	}
}
