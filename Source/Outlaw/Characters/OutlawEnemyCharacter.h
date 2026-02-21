// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OutlawCharacterBase.h"
#include "OutlawEnemyCharacter.generated.h"

class UOutlawDeathComponent;
class UOutlawEnemyDeathHandler;
class UOutlawDamageNumberComponent;
class UOutlawHitReactionComponent;
class UOutlawStatusEffectComponent;
class UOutlawDemoAIBehavior;
class UStaticMeshComponent;

UCLASS()
class OUTLAW_API AOutlawEnemyCharacter : public AOutlawCharacterBase
{
	GENERATED_BODY()

public:
	AOutlawEnemyCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UOutlawDeathComponent> DeathComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UOutlawEnemyDeathHandler> EnemyDeathHandler;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UOutlawDamageNumberComponent> DamageNumberComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UOutlawHitReactionComponent> HitReactionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UOutlawStatusEffectComponent> StatusEffectComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UOutlawDemoAIBehavior> AIBehaviorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> BodyMesh;
};
