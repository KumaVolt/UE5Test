// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AtomProjectileBase.h"
#include "AtomBulletProjectile.generated.h"

UCLASS()
class OUTLAW_API AAtomBulletProjectile : public AAtomProjectileBase
{
	GENERATED_BODY()

public:
	AAtomBulletProjectile(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
