// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/OutlawAbilityTypes.h"
#include "OutlawWeaponManagerComponent.generated.h"

class UOutlawItemInstance;
class UOutlawWeaponAttributeSet;
class UAbilitySystemComponent;
class UOutlawInventoryComponent;

/** Reserve ammo entry — maps an ammo type tag to a count. */
USTRUCT(BlueprintType)
struct FOutlawReserveAmmoEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (Categories = "Ammo"))
	FGameplayTag AmmoTypeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 Amount = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveWeaponChanged, UOutlawItemInstance*, NewWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSetSwapped, int32, NewSetIndex);

/**
 * Manages the active weapon, weapon cycling (shooter), weapon set swapping (ARPG),
 * ammo reserves, and coordination between weapons and the GAS attribute set.
 *
 * Config determines which API is used:
 * - ShooterWeaponSlotOrder: Outriders-style 3-slot cycling
 * - ARPGWeaponSetI/II: PoE2-style weapon swap sets
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UOutlawWeaponManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOutlawWeaponManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ── Shooter API ─────────────────────────────────────────────

	/**
	 * Cycle to the next weapon in the shooter slot order.
	 * Primary1 -> Primary2 -> Sidearm -> Primary1
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Shooter")
	void CycleWeapon();

	/** Switch directly to a specific weapon slot by tag. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Shooter")
	void SwitchToWeaponSlot(FGameplayTag SlotTag);

	/** Get the currently active weapon instance. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	UOutlawItemInstance* GetActiveWeapon() const;

	/** Get the slot tag of the currently active weapon. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FGameplayTag GetActiveWeaponSlotTag() const { return ActiveWeaponSlotTag; }

	/** Get the reserve ammo count for a given ammo type. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Ammo")
	int32 GetReserveAmmo(FGameplayTag AmmoTypeTag) const;

	/** Add reserve ammo for a given ammo type. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Ammo")
	void AddReserveAmmo(FGameplayTag AmmoTypeTag, int32 Amount);

	/** Consume reserve ammo. Returns actual amount consumed. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Ammo")
	int32 ConsumeReserveAmmo(FGameplayTag AmmoTypeTag, int32 Amount);

	// ── ARPG API ────────────────────────────────────────────────

	/**
	 * Toggle between weapon Set I and Set II.
	 * Revokes old gem abilities, grants new ones.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|ARPG")
	void SwapWeaponSet();

	/** Get the currently active weapon set index (0 or 1). */
	UFUNCTION(BlueprintCallable, Category = "Weapon|ARPG")
	int32 GetActiveWeaponSetIndex() const { return ActiveWeaponSetIndex; }

	/** Get all weapon instances in the specified set. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|ARPG")
	TArray<UOutlawItemInstance*> GetWeaponsInSet(int32 SetIndex) const;

	// ── Inventory Callbacks ─────────────────────────────────────

	/** Called by the inventory component when a weapon is equipped into a slot. */
	void OnWeaponEquipped(UOutlawItemInstance* Instance, FGameplayTag SlotTag);

	/** Called by the inventory component when a weapon is unequipped from a slot. */
	void OnWeaponUnequipped(UOutlawItemInstance* Instance, FGameplayTag SlotTag);

	// ── Attribute Management ────────────────────────────────────

	/** Push the active weapon's stats into UOutlawWeaponAttributeSet on the ASC. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ApplyWeaponStatsToASC(UOutlawItemInstance* Instance);

	/** Clear all weapon stats from the ASC attribute set. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ClearWeaponStatsFromASC();

	// ── Delegates ───────────────────────────────────────────────

	/** Fires when the active weapon changes (for UI weapon display). */
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnActiveWeaponChanged OnActiveWeaponChanged;

	/** Fires when weapon sets are swapped (for UI ability bar swap). */
	UPROPERTY(BlueprintAssignable, Category = "Weapon|ARPG")
	FOnWeaponSetSwapped OnWeaponSetSwapped;

	// ── Configuration ───────────────────────────────────────────

	/** Ordered list of weapon slot tags for shooter cycling (e.g. [Weapon.Slot.Primary1, Primary2, Sidearm]). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Config|Shooter", meta = (Categories = "Weapon.Slot"))
	TArray<FGameplayTag> ShooterWeaponSlotOrder;

	/** Slot tags for ARPG weapon Set I (e.g. [Equipment.Slot.WeaponSetI.MainHand, .OffHand]). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Config|ARPG", meta = (Categories = "Equipment.Slot"))
	TArray<FGameplayTag> ARPGWeaponSetI;

	/** Slot tags for ARPG weapon Set II. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Config|ARPG", meta = (Categories = "Equipment.Slot"))
	TArray<FGameplayTag> ARPGWeaponSetII;

private:
	/** Resolve the ASC from the owning actor. */
	UAbilitySystemComponent* GetASC() const;

	/** Resolve the inventory component from the owning actor. */
	UOutlawInventoryComponent* GetInventoryComponent() const;

	/** Get the item instance equipped in a given slot via the inventory component. */
	UOutlawItemInstance* GetWeaponInSlot(FGameplayTag SlotTag) const;

	/** Internal: activate a weapon (grant abilities, push stats, fire delegate). */
	void ActivateWeapon(UOutlawItemInstance* Instance);

	/** Internal: deactivate a weapon (revoke abilities, clear stats). */
	void DeactivateWeapon(UOutlawItemInstance* Instance);

	/** Grant/revoke gem abilities for all weapons in a set. */
	void GrantWeaponSetAbilities(int32 SetIndex);
	void RevokeWeaponSetAbilities(int32 SetIndex);

	// ── Replicated State ────────────────────────────────────────

	/** Currently active weapon slot tag (shooter mode). */
	UPROPERTY(Replicated)
	FGameplayTag ActiveWeaponSlotTag;

	/** Currently active weapon set index (ARPG mode, 0 or 1). */
	UPROPERTY(Replicated)
	int32 ActiveWeaponSetIndex = 0;

	/** Reserve ammo pools by ammo type tag. */
	UPROPERTY(Replicated)
	TArray<FOutlawReserveAmmoEntry> ReserveAmmo;

	// ── Server-Only Handles ─────────────────────────────────────

	/** Handles for the currently active weapon's fire/reload/attack ability sets. */
	FOutlawAbilitySetGrantedHandles ActiveWeaponAbilityHandles;

	/** Handles for ARPG weapon set gem abilities — index 0 = Set I, index 1 = Set II. */
	TArray<FOutlawAbilitySetGrantedHandles> WeaponSetGemHandles;
};
