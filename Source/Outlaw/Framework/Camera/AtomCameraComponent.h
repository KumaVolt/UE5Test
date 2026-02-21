// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameplayTagContainer.h"
#include "Camera/AtomCameraTypes.h"
#include "AtomCameraComponent.generated.h"

class USpringArmComponent;
class UCameraShakeBase;

/**
 * Dual-mode camera component supporting OTS and Isometric modes with runtime toggle.
 * Features: ADS (aim down sights), recoil, screen shake, spring arm integration.
 */
UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UAtomCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UAtomCameraComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraMode(EAtomCameraMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "Camera|ADS")
	void EnterADS();

	UFUNCTION(BlueprintCallable, Category = "Camera|ADS")
	void ExitADS();

	UFUNCTION(BlueprintCallable, Category = "Camera|Recoil")
	void ApplyRecoil(float PitchRecoil, float YawRecoil);

	UFUNCTION(BlueprintCallable, Category = "Camera|Shake")
	void ApplyScreenShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale = 1.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera|Config")
	FAtomCameraConfig OTSConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera|Config")
	FAtomCameraConfig IsometricConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera|ADS")
	float ADSFieldOfView = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera|ADS")
	float ADSSensitivityMultiplier = 0.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera|Recoil")
	float RecoilRecoverySpeed = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera|Recoil")
	float MaxRecoilPitch = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera|Recoil")
	float MaxRecoilYaw = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera|Config")
	float CameraModeBlendSpeed = 5.f;

	UFUNCTION(BlueprintPure, Category = "Camera")
	EAtomCameraMode GetCurrentCameraMode() const { return CurrentMode; }

	UFUNCTION(BlueprintPure, Category = "Camera|ADS")
	bool IsADS() const { return bIsADS; }

private:
	EAtomCameraMode CurrentMode = EAtomCameraMode::OTS;
	bool bIsADS = false;
	FVector2D CurrentRecoilOffset = FVector2D::ZeroVector;

	float DefaultFieldOfView = 90.f;
	float TargetFieldOfView = 90.f;

	UPROPERTY()
	USpringArmComponent* SpringArm = nullptr;

	void InitializeSpringArm();
	void BlendCameraSettings(float DeltaTime);
};
