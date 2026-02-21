// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AtomCharacterBase.h"
#include "InputMappingContext.h"
#include "OutlawPlayerCharacter.generated.h"

class UInputAction;
struct FInputActionValue;
class USpringArmComponent;
class UAtomCameraComponent;
class UAtomLockOnComponent;
class UAtomDeathComponent;
class UAtomPlayerDeathHandler;
class UAtomCombatLogComponent;
class UAtomDamageNumberComponent;
class UAtomHitReactionComponent;
class UAtomStatusEffectComponent;
class UAtomWeaponManagerComponent;
class UAtomInventoryComponent;
class UAtomProgressionComponent;
class UStaticMeshComponent;

UCLASS()
class OUTLAW_API AOutlawPlayerCharacter : public AAtomCharacterBase
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
	TObjectPtr<UAtomCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UAtomLockOnComponent> LockOnComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAtomDeathComponent> DeathComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAtomPlayerDeathHandler> PlayerDeathHandler;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAtomCombatLogComponent> CombatLogComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAtomDamageNumberComponent> DamageNumberComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAtomHitReactionComponent> HitReactionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAtomStatusEffectComponent> StatusEffectComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UAtomWeaponManagerComponent> WeaponManagerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UAtomInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	TObjectPtr<UAtomProgressionComponent> ProgressionComponent;

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
