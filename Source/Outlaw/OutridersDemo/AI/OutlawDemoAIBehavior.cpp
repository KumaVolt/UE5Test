#include "AI/OutlawDemoAIBehavior.h"
#include "AI/AtomAIController.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/AtomAttributeSet.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UOutlawDemoAIBehavior::UOutlawDemoAIBehavior()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for AI
}

void UOutlawDemoAIBehavior::BeginPlay()
{
	Super::BeginPlay();

	HomeLocation = GetOwner()->GetActorLocation();

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		AIController = Cast<AAtomAIController>(Pawn->GetController());
	}

	TransitionTo(EOutlawDemoAIState::Idle);
}

void UOutlawDemoAIBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// Check for flee condition in any state
	if (CurrentState != EOutlawDemoAIState::Flee && GetHealthPercent() < FleeHealthPercent)
	{
		TransitionTo(EOutlawDemoAIState::Flee);
	}

	switch (CurrentState)
	{
	case EOutlawDemoAIState::Idle:    TickIdle(DeltaTime); break;
	case EOutlawDemoAIState::Patrol:  TickPatrol(DeltaTime); break;
	case EOutlawDemoAIState::Chase:   TickChase(DeltaTime); break;
	case EOutlawDemoAIState::Attack:  TickAttack(DeltaTime); break;
	case EOutlawDemoAIState::Flee:    TickFlee(DeltaTime); break;
	}
}

void UOutlawDemoAIBehavior::TickIdle(float DeltaTime)
{
	IdleTimer += DeltaTime;

	// Check for enemies in range
	AActor* Target = FindTarget();
	if (Target)
	{
		TargetActor = Target;
		TransitionTo(EOutlawDemoAIState::Chase);
		return;
	}

	// After a few seconds, start patrolling
	if (IdleTimer > 3.f)
	{
		TransitionTo(EOutlawDemoAIState::Patrol);
	}
}

void UOutlawDemoAIBehavior::TickPatrol(float DeltaTime)
{
	// Check for enemies
	AActor* Target = FindTarget();
	if (Target)
	{
		TargetActor = Target;
		TransitionTo(EOutlawDemoAIState::Chase);
		return;
	}

	if (!bHasPatrolTarget)
	{
		// Pick a random point near home
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		if (NavSys)
		{
			FNavLocation Result;
			if (NavSys->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, Result))
			{
				PatrolTarget = Result.Location;
				bHasPatrolTarget = true;
			}
		}

		if (!bHasPatrolTarget)
		{
			// Fallback: random offset
			PatrolTarget = HomeLocation + FVector(
				FMath::RandRange(-PatrolRadius, PatrolRadius),
				FMath::RandRange(-PatrolRadius, PatrolRadius),
				0.f
			);
			bHasPatrolTarget = true;
		}
	}

	// Move toward patrol target
	if (AIController)
	{
		AIController->MoveToLocation(PatrolTarget, 50.f);

		float Dist = FVector::Dist2D(GetOwner()->GetActorLocation(), PatrolTarget);
		if (Dist < 100.f)
		{
			bHasPatrolTarget = false;
			TransitionTo(EOutlawDemoAIState::Idle);
		}
	}
}

void UOutlawDemoAIBehavior::TickChase(float DeltaTime)
{
	if (!TargetActor)
	{
		TransitionTo(EOutlawDemoAIState::Idle);
		return;
	}

	float Dist = FVector::Dist(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation());

	// Target out of chase range — give up
	if (Dist > ChaseRange * 1.5f)
	{
		TargetActor = nullptr;
		TransitionTo(EOutlawDemoAIState::Idle);
		return;
	}

	// In attack range — switch to attack
	if (Dist <= AttackRange)
	{
		TransitionTo(EOutlawDemoAIState::Attack);
		return;
	}

	// Move toward target
	if (AIController)
	{
		AIController->MoveToActor(TargetActor.Get(), AttackRange * 0.8f);
	}
}

void UOutlawDemoAIBehavior::TickAttack(float DeltaTime)
{
	if (!TargetActor)
	{
		TransitionTo(EOutlawDemoAIState::Idle);
		return;
	}

	float Dist = FVector::Dist(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation());

	// Target moved out of attack range — chase again
	if (Dist > AttackRange * 1.2f)
	{
		TransitionTo(EOutlawDemoAIState::Chase);
		return;
	}

	// Face target
	FVector Dir = (TargetActor->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal2D();
	GetOwner()->SetActorRotation(Dir.Rotation());

	// Attack on cooldown
	AttackTimer += DeltaTime;
	if (AttackTimer >= AttackCooldown)
	{
		AttackTimer = 0.f;
		TryMeleeAttack();
	}
}

void UOutlawDemoAIBehavior::TickFlee(float DeltaTime)
{
	if (!TargetActor)
	{
		TransitionTo(EOutlawDemoAIState::Idle);
		return;
	}

	// Run away from target
	FVector AwayDir = (GetOwner()->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal2D();
	FVector FleeTarget = GetOwner()->GetActorLocation() + AwayDir * 500.f;

	if (AIController)
	{
		AIController->MoveToLocation(FleeTarget, 50.f);
	}

	// If health recovered, go back to chase
	if (GetHealthPercent() > FleeHealthPercent * 2.f)
	{
		TransitionTo(EOutlawDemoAIState::Chase);
	}
}

void UOutlawDemoAIBehavior::TransitionTo(EOutlawDemoAIState NewState)
{
	CurrentState = NewState;
	IdleTimer = 0.f;
	AttackTimer = 0.f;
	bHasPatrolTarget = false;

	if (AIController && NewState != EOutlawDemoAIState::Chase && NewState != EOutlawDemoAIState::Flee)
	{
		AIController->StopMovement();
	}
}

AActor* UOutlawDemoAIBehavior::FindTarget() const
{
	// Use the AIController's perception target if available
	if (AIController)
	{
		// Check if the AI controller has a target actor set
		APawn* Pawn = AIController->GetPawn();
		if (!Pawn)
		{
			return nullptr;
		}
	}

	// Fallback: find nearest player pawn
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (Player)
	{
		float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
		if (Dist <= ChaseRange)
		{
			return Player;
		}
	}

	return nullptr;
}

float UOutlawDemoAIBehavior::GetHealthPercent() const
{
	if (IAbilitySystemInterface* ASCOwner = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = ASCOwner->GetAbilitySystemComponent())
		{
			bool bFound = false;
			float Health = ASC->GetGameplayAttributeValue(UAtomAttributeSet::GetHealthAttribute(), bFound);
			float MaxHealth = ASC->GetGameplayAttributeValue(UAtomAttributeSet::GetMaxHealthAttribute(), bFound);
			if (bFound && MaxHealth > 0.f)
			{
				return Health / MaxHealth;
			}
		}
	}
	return 1.f;
}

void UOutlawDemoAIBehavior::TryMeleeAttack()
{
	// Activate melee ability via input tag
	if (IAbilitySystemInterface* ASCOwner = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = ASCOwner->GetAbilitySystemComponent())
		{
			FGameplayTag MeleeTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Ability.MeleeAttack"));

			// Find and activate the melee ability
			for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
			{
				if (Spec.GetDynamicSpecSourceTags().HasTag(MeleeTag))
				{
					ASC->TryActivateAbility(Spec.Handle);
					break;
				}
			}
		}
	}
}
