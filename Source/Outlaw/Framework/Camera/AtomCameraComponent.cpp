// Fill out your copyright notice in the Description page of Project Settings.

#include "Camera/OutlawCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

UOutlawCameraComponent::UOutlawCameraComponent(const FObjectInitializer& ObjectInitializer)
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

void UOutlawCameraComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeSpringArm();

	DefaultFieldOfView = OTSConfig.FieldOfView;
	TargetFieldOfView = DefaultFieldOfView;
	SetFieldOfView(DefaultFieldOfView);

	SetCameraMode(EOutlawCameraMode::OTS);
}

void UOutlawCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CurrentRecoilOffset = FMath::Vector2DInterpTo(CurrentRecoilOffset, FVector2D::ZeroVector, DeltaTime, RecoilRecoverySpeed);

	if (SpringArm)
	{
		FRotator BaseRotation = CurrentMode == EOutlawCameraMode::OTS 
			? SpringArm->GetComponentRotation() 
			: IsometricConfig.CameraAngle;

		FRotator FinalRotation = BaseRotation;
		FinalRotation.Pitch += CurrentRecoilOffset.X;
		FinalRotation.Yaw += CurrentRecoilOffset.Y;

		if (CurrentMode == EOutlawCameraMode::OTS && SpringArm->bUsePawnControlRotation)
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
		else if (CurrentMode == EOutlawCameraMode::Isometric)
		{
			FinalRotation = IsometricConfig.CameraAngle;
			FinalRotation.Pitch += CurrentRecoilOffset.X;
			FinalRotation.Yaw += CurrentRecoilOffset.Y;
			SpringArm->SetWorldRotation(FinalRotation);
		}
	}

	BlendCameraSettings(DeltaTime);
}

void UOutlawCameraComponent::SetCameraMode(EOutlawCameraMode NewMode)
{
	if (CurrentMode == NewMode) return;
	CurrentMode = NewMode;

	const FOutlawCameraConfig& TargetConfig = (NewMode == EOutlawCameraMode::OTS) ? OTSConfig : IsometricConfig;

	if (SpringArm)
	{
		SpringArm->TargetArmLength = TargetConfig.ArmLength;
		SpringArm->SocketOffset = TargetConfig.SocketOffset;
		SpringArm->SetRelativeRotation(TargetConfig.CameraAngle);
		SpringArm->bUsePawnControlRotation = TargetConfig.bUsePawnControlRotation;
	}

	TargetFieldOfView = bIsADS ? ADSFieldOfView : TargetConfig.FieldOfView;
}

void UOutlawCameraComponent::EnterADS()
{
	if (CurrentMode != EOutlawCameraMode::OTS) return;
	if (bIsADS) return;

	bIsADS = true;
	TargetFieldOfView = ADSFieldOfView;
}

void UOutlawCameraComponent::ExitADS()
{
	if (!bIsADS) return;

	bIsADS = false;
	const FOutlawCameraConfig& CurrentConfig = (CurrentMode == EOutlawCameraMode::OTS) ? OTSConfig : IsometricConfig;
	TargetFieldOfView = CurrentConfig.FieldOfView;
}

void UOutlawCameraComponent::ApplyRecoil(float PitchRecoil, float YawRecoil)
{
	CurrentRecoilOffset.X = FMath::Clamp(CurrentRecoilOffset.X + PitchRecoil, -MaxRecoilPitch, MaxRecoilPitch);
	CurrentRecoilOffset.Y = FMath::Clamp(CurrentRecoilOffset.Y + YawRecoil, -MaxRecoilYaw, MaxRecoilYaw);
}

void UOutlawCameraComponent::ApplyScreenShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale)
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

void UOutlawCameraComponent::InitializeSpringArm()
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

void UOutlawCameraComponent::BlendCameraSettings(float DeltaTime)
{
	float CurrentFOV = FieldOfView;
	float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFieldOfView, DeltaTime, CameraModeBlendSpeed);
	FieldOfView = NewFOV;

	if (SpringArm)
	{
		const FOutlawCameraConfig& TargetConfig = (CurrentMode == EOutlawCameraMode::OTS) ? OTSConfig : IsometricConfig;
		
		float CurrentArmLength = SpringArm->TargetArmLength;
		float NewArmLength = FMath::FInterpTo(CurrentArmLength, TargetConfig.ArmLength, DeltaTime, CameraModeBlendSpeed);
		SpringArm->TargetArmLength = NewArmLength;

		FVector CurrentOffset = SpringArm->SocketOffset;
		FVector NewOffset = FMath::VInterpTo(CurrentOffset, TargetConfig.SocketOffset, DeltaTime, CameraModeBlendSpeed);
		SpringArm->SocketOffset = NewOffset;
	}
}
