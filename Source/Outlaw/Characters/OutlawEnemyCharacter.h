// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OutlawCharacterBase.h"
#include "OutlawEnemyCharacter.generated.h"

UCLASS()
class OUTLAW_API AOutlawEnemyCharacter : public AOutlawCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AOutlawEnemyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
};
