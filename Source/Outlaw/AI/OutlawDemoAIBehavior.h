#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OutlawDemoAIBehavior.generated.h"

class AOutlawAIController;

/** Simple AI states for the demo — replaces StateTree which needs editor assets. */
UENUM(BlueprintType)
enum class EOutlawDemoAIState : uint8
{
	Idle,
	Patrol,
	Chase,
	Attack,
	Flee
};

/**
 * Tick-based C++ AI behavior for the demo.
 * Simple state machine: Idle -> Patrol -> Chase -> Attack (or Flee if low health).
 * No StateTree or Behavior Tree assets needed.
 */
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UOutlawDemoAIBehavior : public UActorComponent
{
	GENERATED_BODY()

public:
	UOutlawDemoAIBehavior();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	float AttackRange = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	float ChaseRange = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	float AttackCooldown = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	float PatrolRadius = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	float FleeHealthPercent = 0.2f;

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	EOutlawDemoAIState CurrentState = EOutlawDemoAIState::Idle;

private:
	void TickIdle(float DeltaTime);
	void TickPatrol(float DeltaTime);
	void TickChase(float DeltaTime);
	void TickAttack(float DeltaTime);
	void TickFlee(float DeltaTime);

	void TransitionTo(EOutlawDemoAIState NewState);
	AActor* FindTarget() const;
	float GetHealthPercent() const;
	void TryMeleeAttack();

	UPROPERTY()
	TObjectPtr<AOutlawAIController> AIController;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	FVector HomeLocation = FVector::ZeroVector;
	FVector PatrolTarget = FVector::ZeroVector;
	float IdleTimer = 0.f;
	float AttackTimer = 0.f;
	bool bHasPatrolTarget = false;
};
