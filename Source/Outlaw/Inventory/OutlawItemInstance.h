// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Weapon/OutlawWeaponTypes.h"
#include "AbilitySystem/OutlawAbilityTypes.h"
#include "OutlawItemInstance.generated.h"

class UOutlawItemDefinition;
class UOutlawWeaponModDefinition;
class UOutlawSkillGemDefinition;
class UAbilitySystemComponent;

/**
 * Per-item mutable runtime state. Weapons need this for ammo, rolled affixes, socketed gems, etc.
 * Created by the inventory component when a weapon is added.
 * Supports sub-object replication via IsSupportedForNetworking().
 */
UCLASS(BlueprintType)
class OUTLAW_API UOutlawItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UOutlawItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool IsSupportedForNetworking() const override { return true; }

	// ── Identity ────────────────────────────────────────────────

	/** The item definition this instance is based on. */
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TObjectPtr<UOutlawItemDefinition> ItemDef;

	/** Unique instance ID matching the inventory entry. */
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 InstanceId = INDEX_NONE;

	// ── Shooter State ───────────────────────────────────────────

	/** Current ammo in the magazine. */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Shooter")
	int32 CurrentAmmo = 0;

	/** Installed weapon mod in Tier 1 slot. */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Shooter")
	TObjectPtr<UOutlawWeaponModDefinition> InstalledModTier1;

	/** Installed weapon mod in Tier 2 slot. */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Shooter")
	TObjectPtr<UOutlawWeaponModDefinition> InstalledModTier2;

	// ── ARPG State ──────────────────────────────────────────────

	/** Quality value (0-20). Increases base DPS by Quality%. */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|ARPG", meta = (ClampMin = "0", ClampMax = "20"))
	int32 Quality = 0;

	/** Rolled affixes on this weapon. */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|ARPG")
	TArray<FOutlawItemAffix> Affixes;

	/** Socket layout with optionally socketed gems. */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|ARPG")
	TArray<FOutlawSocketSlot> SocketSlots;

	// ── Shooter Mod API ─────────────────────────────────────────

	/**
	 * Install a weapon mod into the specified tier slot. Grants the mod's ability set.
	 * @param ModDef  The mod to install.
	 * @param Tier    1 or 2.
	 * @param ASC     The ability system component to grant abilities to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Mods")
	void InstallMod(UOutlawWeaponModDefinition* ModDef, int32 Tier, UAbilitySystemComponent* ASC);

	/**
	 * Remove the mod from the specified tier slot. Revokes its abilities.
	 * @param Tier  1 or 2.
	 * @param ASC   The ability system component to revoke from.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Mods")
	void RemoveMod(int32 Tier, UAbilitySystemComponent* ASC);

	// ── ARPG Gem API ────────────────────────────────────────────

	/**
	 * Socket a skill gem into the specified socket index.
	 * @param GemDef       The gem to socket.
	 * @param SocketIndex  Index into SocketSlots.
	 * @return True if the gem was successfully socketed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Gems")
	bool SocketGem(UOutlawSkillGemDefinition* GemDef, int32 SocketIndex);

	/**
	 * Remove a socketed gem from the specified socket index.
	 * @param SocketIndex  Index into SocketSlots.
	 * @return The removed gem definition, or nullptr if socket was empty.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Gems")
	UOutlawSkillGemDefinition* UnsocketGem(int32 SocketIndex);

	// ── ARPG Affix API ──────────────────────────────────────────

	/**
	 * Roll random affixes from the weapon's affix pool based on item level.
	 * Replaces any existing affixes.
	 * @param ItemLevel  Determines which affixes are eligible and how many roll.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Affixes")
	void RollAffixes(int32 ItemLevel);

	/**
	 * Apply all affix gameplay effects to the given ASC using SetByCaller magnitude.
	 * @param ASC  The ability system component to apply effects to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Affixes")
	void GrantAffixEffects(UAbilitySystemComponent* ASC);

	/** Revoke all previously granted affix effects from the ASC. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Affixes")
	void RevokeAffixEffects(UAbilitySystemComponent* ASC);

	// ── ARPG Gem Ability API ────────────────────────────────────

	/**
	 * Grant abilities from all socketed gems to the ASC.
	 * Call when this weapon becomes active.
	 * @param ASC  The ability system component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Gems")
	void GrantSocketedGemAbilities(UAbilitySystemComponent* ASC);

	/**
	 * Revoke all socketed gem abilities from the ASC.
	 * Call when this weapon becomes inactive.
	 * @param ASC  The ability system component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Gems")
	void RevokeSocketedGemAbilities(UAbilitySystemComponent* ASC);

private:
	/** Handles for Tier 1 mod abilities. Server-only. */
	FOutlawAbilitySetGrantedHandles ModTier1Handles;

	/** Handles for Tier 2 mod abilities. Server-only. */
	FOutlawAbilitySetGrantedHandles ModTier2Handles;

	/** Handles for each socketed gem's abilities. Server-only. */
	TArray<FOutlawAbilitySetGrantedHandles> SocketedGemHandles;

	/** Handles for affix gameplay effects. Server-only. */
	TArray<FActiveGameplayEffectHandle> AffixEffectHandles;
};
