// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AtomAnimNotify_DamageWindow.generated.h"

UCLASS()
class OUTLAW_API UAtomAnimNotify_DamageWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atom|Animation")
	float DamageRadius = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atom|Animation")
	FVector BoxHalfExtent = FVector(50.f, 50.f, 50.f);
};
