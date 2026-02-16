// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "OutlawAnimNotify_DamageWindow.generated.h"

UCLASS()
class OUTLAW_API UOutlawAnimNotify_DamageWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outlaw|Animation")
	float DamageRadius = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outlaw|Animation")
	FVector BoxHalfExtent = FVector(50.f, 50.f, 50.f);
};
