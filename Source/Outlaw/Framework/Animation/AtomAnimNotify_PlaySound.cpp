// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/OutlawAnimNotify_PlaySound.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UOutlawAnimNotify_PlaySound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !Sound)
	{
		return;
	}

	FVector SoundLocation = FVector::ZeroVector;

	if (SocketName != NAME_None && MeshComp->DoesSocketExist(SocketName))
	{
		SoundLocation = MeshComp->GetSocketLocation(SocketName);
	}
	else
	{
		SoundLocation = MeshComp->GetComponentLocation();
	}

	if (bAttachToSocket && SocketName != NAME_None)
	{
		UGameplayStatics::SpawnSoundAttached(
			Sound,
			MeshComp,
			SocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false,
			VolumeMultiplier,
			PitchMultiplier
		);
	}
	else
	{
		UGameplayStatics::PlaySoundAtLocation(
			MeshComp->GetWorld(),
			Sound,
			SoundLocation,
			VolumeMultiplier,
			PitchMultiplier
		);
	}
}
