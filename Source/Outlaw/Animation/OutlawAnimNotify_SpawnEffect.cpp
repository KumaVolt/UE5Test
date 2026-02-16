// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/OutlawAnimNotify_SpawnEffect.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

void UOutlawAnimNotify_SpawnEffect::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !NiagaraSystem.Get())
	{
		return;
	}

	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	if (SocketName != NAME_None && MeshComp->DoesSocketExist(SocketName))
	{
		FTransform SocketTransform = MeshComp->GetSocketTransform(SocketName);
		SpawnLocation = SocketTransform.GetLocation() + SocketTransform.TransformVector(LocationOffset);
		SpawnRotation = SocketTransform.Rotator();
	}
	else
	{
		SpawnLocation = MeshComp->GetComponentLocation() + LocationOffset;
		SpawnRotation = MeshComp->GetComponentRotation();
	}

	if (bAttachToSocket && SocketName != NAME_None)
	{
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            NiagaraSystem.Get(),
            MeshComp,
            SocketName,
            LocationOffset,
            FRotator::ZeroRotator,
            Scale,
            EAttachLocation::SnapToTarget,
            true,
            ENCPoolMethod::None,
            true
        );
	}
	else
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			MeshComp->GetWorld(),
			NiagaraSystem.Get(),
			SpawnLocation,
			SpawnRotation,
			Scale,
			true
		);
	}
}
