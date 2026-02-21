// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OutlawProjectileBase.h"
#include "OutlawBulletProjectile.generated.h"

UCLASS()
class OUTLAW_API AOutlawBulletProjectile : public AOutlawProjectileBase
{
	GENERATED_BODY()

public:
	AOutlawBulletProjectile(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
