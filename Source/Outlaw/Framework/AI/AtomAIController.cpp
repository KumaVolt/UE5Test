// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Components/StateTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AAtomAIController::AAtomAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));

	SetPerceptionComponent(*AIPerceptionComponent);
}

void AAtomAIController::BeginPlay()
{
	Super::BeginPlay();

	ConfigurePerception();

	if (StateTreeComponent)
	{
		StateTreeComponent->StartLogic();
	}

	if (APawn* ControlledPawn = GetPawn())
	{
		AIContext.HomeLocation = ControlledPawn->GetActorLocation();
	}
}

void AAtomAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn)
	{
		AIContext.HomeLocation = InPawn->GetActorLocation();
	}
}

AActor* AAtomAIController::GetTargetActor() const
{
	return AIContext.TargetActor;
}

void AAtomAIController::SetTargetActor(AActor* NewTarget)
{
	AIContext.TargetActor = NewTarget;

	if (NewTarget)
	{
		AIContext.LastKnownTargetLocation = NewTarget->GetActorLocation();
	}
}

void AAtomAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		if (!AIContext.TargetActor)
		{
			SetTargetActor(Actor);
		}
	}
	else
	{
		if (AIContext.TargetActor == Actor)
		{
			AIContext.LastKnownTargetLocation = Actor->GetActorLocation();
			AIContext.TargetActor = nullptr;
		}
	}
}

void AAtomAIController::ConfigurePerception()
{
	if (!AIPerceptionComponent)
	{
		return;
	}

	UAISenseConfig_Sight* SightConfig = NewObject<UAISenseConfig_Sight>(this);
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
	SightConfig->SetMaxAge(MaxAge);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	UAISenseConfig_Hearing* HearingConfig = NewObject<UAISenseConfig_Hearing>(this);
	HearingConfig->HearingRange = HearingRange;
	HearingConfig->SetMaxAge(MaxAge);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;

	UAISenseConfig_Damage* DamageConfig = NewObject<UAISenseConfig_Damage>(this);
	DamageConfig->SetMaxAge(MaxAge);

	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->ConfigureSense(*HearingConfig);
	AIPerceptionComponent->ConfigureSense(*DamageConfig);
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAtomAIController::OnPerceptionUpdated);
}
