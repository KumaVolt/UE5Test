// Fill out your copyright notice in the Description page of Project Settings.

#include "Camera/AtomCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

UAtomCameraComponent::UAtomCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;

	OTSConfig.ArmLength = 300.f;
	OTSConfig.SocketOffset = FVector(0.f, 50.f, 60.f);
	OTSConfig.FieldOfView = 90.f;
	OTSConfig.bUsePawnControlRotation = true;

	IsometricConfig.ArmLength = 1200.f;
	IsometricConfig.CameraAngle = FRotator(-55.f, 0.f, 0.f);
	IsometricConfig.FieldOfView = 60.f;
	IsometricConfig.bUsePawnControlRotation = false;
}

void UAtomCameraComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeSpringArm();

	DefaultFieldOfView = OTSConfig.FieldOfView;
	TargetFieldOfView = DefaultFieldOfView;
	SetFieldOfView(DefaultFieldOfView);

	SetCameraMode(EAtomCameraMode::OTS);
}

void UAtomCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CurrentRecoilOffset = FMath::Vector2DInterpTo(CurrentRecoilOffset, FVector2D::ZeroVector, DeltaTime, RecoilRecoverySpeed);

	if (SpringArm)
	{
		FRotator BaseRotation = CurrentMode == EAtomCameraMode::OTS 
			? SpringArm->GetComponentRotation() 
			: IsometricConfig.CameraAngle;

		FRotator FinalRotation = BaseRotation;
		FinalRotation.Pitch += CurrentRecoilOffset.X;
		FinalRotation.Yaw += CurrentRecoilOffset.Y;

		if (CurrentMode == EAtomCameraMode::OTS && SpringArm->bUsePawnControlRotation)
		{
			AActor* Owner = GetOwner();
			if (APawn* Pawn = Cast<APawn>(Owner))
			{
				FRotator ControlRotation = Pawn->GetControlRotation();
				FinalRotation = ControlRotation;
				FinalRotation.Pitch += CurrentRecoilOffset.X;
				FinalRotation.Yaw += CurrentRecoilOffset.Y;
			}
		}
		else if (CurrentMode == EAtomCameraMode::Isometric)
		{
			FinalRotation = IsometricConfig.CameraAngle;
			FinalRotation.Pitch += CurrentRecoilOffset.X;
			FinalRotation.Yaw += CurrentRecoilOffset.Y;
			SpringArm->SetWorldRotation(FinalRotation);
		}
	}

	BlendCameraSettings(DeltaTime);
}

void UAtomCameraComponent::SetCameraMode(EAtomCameraMode NewMode)
{
	if (CurrentMode == NewMode) return;
	CurrentMode = NewMode;

	const FAtomCameraConfig& TargetConfig = (NewMode == EAtomCameraMode::OTS) ? OTSConfig : IsometricConfig;

	if (SpringArm)
	{
		SpringArm->TargetArmLength = TargetConfig.ArmLength;
		SpringArm->SocketOffset = TargetConfig.SocketOffset;
		SpringArm->SetRelativeRotation(TargetConfig.CameraAngle);
		SpringArm->bUsePawnControlRotation = TargetConfig.bUsePawnControlRotation;
	}

	TargetFieldOfView = bIsADS ? ADSFieldOfView : TargetConfig.FieldOfView;
}

void UAtomCameraComponent::EnterADS()
{
	if (CurrentMode != EAtomCameraMode::OTS) return;
	if (bIsADS) return;

	bIsADS = true;
	TargetFieldOfView = ADSFieldOfView;
}

void UAtomCameraComponent::ExitADS()
{
	if (!bIsADS) return;

	bIsADS = false;
	const FAtomCameraConfig& CurrentConfig = (CurrentMode == EAtomCameraMode::OTS) ? OTSConfig : IsometricConfig;
	TargetFieldOfView = CurrentConfig.FieldOfView;
}

void UAtomCameraComponent::ApplyRecoil(float PitchRecoil, float YawRecoil)
{
	CurrentRecoilOffset.X = FMath::Clamp(CurrentRecoilOffset.X + PitchRecoil, -MaxRecoilPitch, MaxRecoilPitch);
	CurrentRecoilOffset.Y = FMath::Clamp(CurrentRecoilOffset.Y + YawRecoil, -MaxRecoilYaw, MaxRecoilYaw);
}

void UAtomCameraComponent::ApplyScreenShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale)
{
	if (!ShakeClass) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* Pawn = Cast<APawn>(Owner);
	if (!Pawn) return;

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return;

	APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
	if (!CameraManager) return;

	CameraManager->StartCameraShake(ShakeClass, Scale);
}

void UAtomCameraComponent::InitializeSpringArm()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	TArray<USpringArmComponent*> SpringArms;
	Owner->GetComponents<USpringArmComponent>(SpringArms);

	if (SpringArms.Num() > 0)
	{
		SpringArm = SpringArms[0];
		SpringArm->bDoCollisionTest = true;
	}
}

void UAtomCameraComponent::BlendCameraSettings(float DeltaTime)
{
	float CurrentFOV = FieldOfView;
	float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFieldOfView, DeltaTime, CameraModeBlendSpeed);
	FieldOfView = NewFOV;

	if (SpringArm)
	{
		const FAtomCameraConfig& TargetConfig = (CurrentMode == EAtomCameraMode::OTS) ? OTSConfig : IsometricConfig;
		
		float CurrentArmLength = SpringArm->TargetArmLength;
		float NewArmLength = FMath::FInterpTo(CurrentArmLength, TargetConfig.ArmLength, DeltaTime, CameraModeBlendSpeed);
		SpringArm->TargetArmLength = NewArmLength;

		FVector CurrentOffset = SpringArm->SocketOffset;
		FVector NewOffset = FMath::VInterpTo(CurrentOffset, TargetConfig.SocketOffset, DeltaTime, CameraModeBlendSpeed);
		SpringArm->SocketOffset = NewOffset;
	}
}
