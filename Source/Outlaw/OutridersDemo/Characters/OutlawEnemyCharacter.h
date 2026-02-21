// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AtomCharacterBase.h"
#include "OutlawEnemyCharacter.generated.h"

class UAtomDeathComponent;
class UAtomEnemyDeathHandler;
class UAtomDamageNumberComponent;
class UAtomHitReactionComponent;
class UAtomStatusEffectComponent;
class UOutlawDemoAIBehavior;
class UStaticMeshComponent;

UCLASS()
class OUTLAW_API AOutlawEnemyCharacter : public AAtomCharacterBase
{
	GENERATED_BODY()

public:
	AOutlawEnemyCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAtomDeathComponent> DeathComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAtomEnemyDeathHandler> EnemyDeathHandler;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAtomDamageNumberComponent> DamageNumberComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAtomHitReactionComponent> HitReactionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAtomStatusEffectComponent> StatusEffectComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UOutlawDemoAIBehavior> AIBehaviorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> BodyMesh;
};
