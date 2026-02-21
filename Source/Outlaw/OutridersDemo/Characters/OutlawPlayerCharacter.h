// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OutlawCharacterBase.h"
#include "InputMappingContext.h"
#include "OutlawPlayerCharacter.generated.h"

class UInputAction;
struct FInputActionValue;
class USpringArmComponent;
class UOutlawCameraComponent;
class UOutlawLockOnComponent;
class UOutlawDeathComponent;
class UOutlawPlayerDeathHandler;
class UOutlawCombatLogComponent;
class UOutlawDamageNumberComponent;
class UOutlawHitReactionComponent;
class UOutlawStatusEffectComponent;
class UOutlawWeaponManagerComponent;
class UOutlawInventoryComponent;
class UOutlawProgressionComponent;
class UStaticMeshComponent;

UCLASS()
class OUTLAW_API AOutlawPlayerCharacter : public AOutlawCharacterBase
{
	GENERATED_BODY()

public:
	AOutlawPlayerCharacter();
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void BeginPlay() override;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* FireAction;

	/** ADS Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ADSAction;

	/** Reload Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ReloadAction;

	/** Cycle Weapon Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CycleWeaponAction;

	/** Inventory Toggle Input Action (Outriders-style equipment popup) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InventoryToggleAction;

	/** Full Inventory Input Action (Destiny-style full screen) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* FullInventoryAction;

	/** Skill Tree Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SkillTreeAction;

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	// ── Input Handlers ──────────────────────────────────────────

	void OnFireStarted();
	void OnFireCompleted();
	void OnADSStarted();
	void OnADSCompleted();
	void OnReloadStarted();
	void OnCycleWeaponStarted();
	void OnInventoryToggle();
	void OnFullInventoryToggle();
	void OnSkillTreeToggle();

	// ── Demo Components ─────────────────────────────────────────

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UOutlawCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UOutlawLockOnComponent> LockOnComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UOutlawDeathComponent> DeathComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UOutlawPlayerDeathHandler> PlayerDeathHandler;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UOutlawCombatLogComponent> CombatLogComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UOutlawDamageNumberComponent> DamageNumberComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UOutlawHitReactionComponent> HitReactionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UOutlawStatusEffectComponent> StatusEffectComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UOutlawWeaponManagerComponent> WeaponManagerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UOutlawInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	TObjectPtr<UOutlawProgressionComponent> ProgressionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> WeaponBodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> WeaponBarrelMesh;

private:
	void InitAbilitySystemComponent();
	void SetupDemoDefaults();
};
