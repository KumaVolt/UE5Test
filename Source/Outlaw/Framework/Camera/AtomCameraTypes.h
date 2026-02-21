// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AtomCameraTypes.generated.h"

/**
 * Camera mode enum — switches between OTS and Isometric at runtime.
 */
UENUM(BlueprintType)
enum class EAtomCameraMode : uint8
{
	OTS UMETA(DisplayName = "Over-The-Shoulder"),
	Isometric UMETA(DisplayName = "Isometric")
};

/**
 * Camera configuration struct — defines settings for each camera mode.
 */
USTRUCT(BlueprintType)
struct FAtomCameraConfig
{
	GENERATED_BODY()

	/** Distance from the target (character) to the camera. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float ArmLength = 300.f;

	/** Socket offset on the spring arm (right/up/forward). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	FVector SocketOffset = FVector::ZeroVector;

	/** Fixed camera angle (pitch, yaw, roll) for this mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	FRotator CameraAngle = FRotator::ZeroRotator;

	/** Field of view for this camera mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float FieldOfView = 90.f;

	/** Whether the camera follows pawn control rotation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	bool bUsePawnControlRotation = true;
};

/**
 * Gameplay tags for the camera system.
 * 
 * Expected tags:
 * - Combat.Targetable — tag for actors that can be targeted by lock-on
 */
namespace AtomCameraTags
{
	// Combat.Targetable tag is defined in the project's gameplay tags data table
}
