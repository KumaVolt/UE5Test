// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomWeaponManagerComponent.h"
#include "Inventory/AtomItemInstance.h"
#include "Inventory/AtomItemDefinition.h"
#include "Inventory/AtomInventoryComponent.h"
#include "AtomShooterWeaponData.h"
#include "AtomARPGWeaponData.h"
#include "AbilitySystem/AtomAbilitySet.h"
#include "AbilitySystem/AtomWeaponAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtomWeaponManager, Log, All);

UAtomWeaponManagerComponent::UAtomWeaponManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UAtomWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAtomWeaponManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAtomWeaponManagerComponent, ActiveWeaponSlotTag);
	DOREPLIFETIME(UAtomWeaponManagerComponent, ActiveWeaponSetIndex);
	DOREPLIFETIME(UAtomWeaponManagerComponent, ReserveAmmo);
}

// ── Shooter API ─────────────────────────────────────────────────

void UAtomWeaponManagerComponent::CycleWeapon()
{
	if (!GetOwner()->HasAuthority() || ShooterWeaponSlotOrder.Num() == 0)
	{
		return;
	}

	// Find current index in the slot order
	int32 CurrentIndex = ShooterWeaponSlotOrder.IndexOfByKey(ActiveWeaponSlotTag);
	if (CurrentIndex == INDEX_NONE)
	{
		CurrentIndex = 0;
	}
	else
	{
		CurrentIndex = (CurrentIndex + 1) % ShooterWeaponSlotOrder.Num();
	}

	SwitchToWeaponSlot(ShooterWeaponSlotOrder[CurrentIndex]);
}

void UAtomWeaponManagerComponent::SwitchToWeaponSlot(FGameplayTag SlotTag)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (ActiveWeaponSlotTag == SlotTag)
	{
		return;
	}

	// Deactivate current weapon
	UAtomItemInstance* OldWeapon = GetActiveWeapon();
	if (OldWeapon)
	{
		DeactivateWeapon(OldWeapon);
	}

	ActiveWeaponSlotTag = SlotTag;

	// Activate new weapon
	UAtomItemInstance* NewWeapon = GetWeaponInSlot(SlotTag);
	if (NewWeapon)
	{
		ActivateWeapon(NewWeapon);
	}

	OnActiveWeaponChanged.Broadcast(NewWeapon);
}

UAtomItemInstance* UAtomWeaponManagerComponent::GetActiveWeapon() const
{
	if (!ActiveWeaponSlotTag.IsValid())
	{
		return nullptr;
	}
	return GetWeaponInSlot(ActiveWeaponSlotTag);
}

int32 UAtomWeaponManagerComponent::GetReserveAmmo(FGameplayTag AmmoTypeTag) const
{
	for (const FAtomReserveAmmoEntry& Entry : ReserveAmmo)
	{
		if (Entry.AmmoTypeTag == AmmoTypeTag)
		{
			return Entry.Amount;
		}
	}
	return 0;
}

void UAtomWeaponManagerComponent::AddReserveAmmo(FGameplayTag AmmoTypeTag, int32 Amount)
{
	if (!GetOwner()->HasAuthority() || Amount <= 0 || !AmmoTypeTag.IsValid())
	{
		return;
	}

	for (FAtomReserveAmmoEntry& Entry : ReserveAmmo)
	{
		if (Entry.AmmoTypeTag == AmmoTypeTag)
		{
			Entry.Amount += Amount;
			return;
		}
	}

	// Create new entry
	FAtomReserveAmmoEntry NewEntry;
	NewEntry.AmmoTypeTag = AmmoTypeTag;
	NewEntry.Amount = Amount;
	ReserveAmmo.Add(NewEntry);
}

int32 UAtomWeaponManagerComponent::ConsumeReserveAmmo(FGameplayTag AmmoTypeTag, int32 Amount)
{
	if (!GetOwner()->HasAuthority() || Amount <= 0 || !AmmoTypeTag.IsValid())
	{
		return 0;
	}

	for (FAtomReserveAmmoEntry& Entry : ReserveAmmo)
	{
		if (Entry.AmmoTypeTag == AmmoTypeTag)
		{
			if (Entry.Amount <= 0)
			{
				return 0;
			}
			const int32 Consumed = FMath::Min(Amount, Entry.Amount);
			Entry.Amount -= Consumed;
			return Consumed;
		}
	}

	return 0;
}

// ── ARPG API ────────────────────────────────────────────────────

void UAtomWeaponManagerComponent::SwapWeaponSet()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// Revoke abilities from current set
	RevokeWeaponSetAbilities(ActiveWeaponSetIndex);

	// Deactivate current active weapon (for attribute clearing)
	UAtomItemInstance* OldWeapon = GetActiveWeapon();
	if (OldWeapon)
	{
		DeactivateWeapon(OldWeapon);
	}

	// Toggle set
	ActiveWeaponSetIndex = (ActiveWeaponSetIndex == 0) ? 1 : 0;

	// Grant abilities for new set
	GrantWeaponSetAbilities(ActiveWeaponSetIndex);

	// Activate first weapon in new set for stats
	TArray<UAtomItemInstance*> NewSetWeapons = GetWeaponsInSet(ActiveWeaponSetIndex);
	if (NewSetWeapons.Num() > 0 && NewSetWeapons[0])
	{
		ActivateWeapon(NewSetWeapons[0]);
	}

	OnWeaponSetSwapped.Broadcast(ActiveWeaponSetIndex);
	OnActiveWeaponChanged.Broadcast(NewSetWeapons.Num() > 0 ? NewSetWeapons[0] : nullptr);
}

TArray<UAtomItemInstance*> UAtomWeaponManagerComponent::GetWeaponsInSet(int32 SetIndex) const
{
	TArray<UAtomItemInstance*> Result;
	const TArray<FGameplayTag>& SlotTags = (SetIndex == 0) ? ARPGWeaponSetI : ARPGWeaponSetII;

	for (const FGameplayTag& SlotTag : SlotTags)
	{
		UAtomItemInstance* Instance = GetWeaponInSlot(SlotTag);
		if (Instance)
		{
			Result.Add(Instance);
		}
	}

	return Result;
}

// ── Inventory Callbacks ─────────────────────────────────────────

void UAtomWeaponManagerComponent::OnWeaponEquipped(UAtomItemInstance* Instance, FGameplayTag SlotTag)
{
	if (!Instance)
	{
		return;
	}

	// If this is the active shooter slot, activate the weapon
	if (ActiveWeaponSlotTag == SlotTag)
	{
		ActivateWeapon(Instance);
		OnActiveWeaponChanged.Broadcast(Instance);
	}

	// If this is in the active ARPG weapon set, grant gem abilities
	const TArray<FGameplayTag>& ActiveSetSlots = (ActiveWeaponSetIndex == 0) ? ARPGWeaponSetI : ARPGWeaponSetII;
	if (ActiveSetSlots.Contains(SlotTag))
	{
		UAbilitySystemComponent* ASC = GetASC();
		if (ASC)
		{
			Instance->GrantSocketedGemAbilities(ASC);
		}
	}
}

void UAtomWeaponManagerComponent::OnWeaponUnequipped(UAtomItemInstance* Instance, FGameplayTag SlotTag)
{
	if (!Instance)
	{
		return;
	}

	// If this was the active weapon, deactivate it
	if (ActiveWeaponSlotTag == SlotTag)
	{
		DeactivateWeapon(Instance);
		OnActiveWeaponChanged.Broadcast(nullptr);
	}

	// Revoke gem abilities if in active set
	const TArray<FGameplayTag>& ActiveSetSlots = (ActiveWeaponSetIndex == 0) ? ARPGWeaponSetI : ARPGWeaponSetII;
	if (ActiveSetSlots.Contains(SlotTag))
	{
		UAbilitySystemComponent* ASC = GetASC();
		if (ASC)
		{
			Instance->RevokeSocketedGemAbilities(ASC);
		}
	}
}

// ── Attribute Management ────────────────────────────────────────

void UAtomWeaponManagerComponent::ApplyWeaponStatsToASC(UAtomItemInstance* Instance)
{
	if (!Instance || !Instance->ItemDef)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	const UAtomWeaponAttributeSet* WeaponAttribs = ASC->GetSet<UAtomWeaponAttributeSet>();
	if (!WeaponAttribs)
	{
		return;
	}

	// Cast away const — we're the authority setting base values
	UAtomWeaponAttributeSet* MutableAttribs = const_cast<UAtomWeaponAttributeSet*>(WeaponAttribs);

	// Apply shooter stats
	if (Instance->ItemDef->ShooterWeaponData)
	{
		const UAtomShooterWeaponData* Data = Instance->ItemDef->ShooterWeaponData;
		MutableAttribs->SetFirepower(Data->Firepower);
		MutableAttribs->SetRPM(Data->RPM);
		MutableAttribs->SetAccuracy(Data->Accuracy);
		MutableAttribs->SetStability(Data->Stability);
		MutableAttribs->SetCritMultiplier(Data->CritMultiplier);
		MutableAttribs->SetWeaponRange(Data->Range);
	}

	// Apply ARPG stats
	if (Instance->ItemDef->ARPGWeaponData)
	{
		const UAtomARPGWeaponData* Data = Instance->ItemDef->ARPGWeaponData;
		MutableAttribs->SetPhysicalDamageMin(Data->PhysicalDamageMin);
		MutableAttribs->SetPhysicalDamageMax(Data->PhysicalDamageMax);
		MutableAttribs->SetAttackSpeed(Data->AttackSpeed);
		MutableAttribs->SetCriticalStrikeChance(Data->CriticalStrikeChance);
	}
}

void UAtomWeaponManagerComponent::ClearWeaponStatsFromASC()
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	const UAtomWeaponAttributeSet* WeaponAttribs = ASC->GetSet<UAtomWeaponAttributeSet>();
	if (!WeaponAttribs)
	{
		return;
	}

	UAtomWeaponAttributeSet* MutableAttribs = const_cast<UAtomWeaponAttributeSet*>(WeaponAttribs);
	MutableAttribs->SetFirepower(0.0f);
	MutableAttribs->SetRPM(0.0f);
	MutableAttribs->SetAccuracy(0.0f);
	MutableAttribs->SetStability(0.0f);
	MutableAttribs->SetCritMultiplier(0.0f);
	MutableAttribs->SetWeaponRange(0.0f);
	MutableAttribs->SetPhysicalDamageMin(0.0f);
	MutableAttribs->SetPhysicalDamageMax(0.0f);
	MutableAttribs->SetAttackSpeed(0.0f);
	MutableAttribs->SetCriticalStrikeChance(0.0f);
}

// ── Private Helpers ─────────────────────────────────────────────

UAbilitySystemComponent* UAtomWeaponManagerComponent::GetASC() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		return ASI->GetAbilitySystemComponent();
	}

	if (const APawn* Pawn = Cast<APawn>(Owner))
	{
		if (const APlayerState* PS = Pawn->GetPlayerState())
		{
			if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PS))
			{
				return ASI->GetAbilitySystemComponent();
			}
		}
	}

	return nullptr;
}

UAtomInventoryComponent* UAtomWeaponManagerComponent::GetInventoryComponent() const
{
	AActor* Owner = GetOwner();
	return Owner ? Owner->FindComponentByClass<UAtomInventoryComponent>() : nullptr;
}

UAtomItemInstance* UAtomWeaponManagerComponent::GetWeaponInSlot(FGameplayTag SlotTag) const
{
	UAtomInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory)
	{
		return nullptr;
	}

	return Inventory->GetItemInstance(SlotTag);
}

void UAtomWeaponManagerComponent::ActivateWeapon(UAtomItemInstance* Instance)
{
	if (!Instance || !Instance->ItemDef)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	// Grant weapon-specific ability sets
	if (Instance->ItemDef->ShooterWeaponData)
	{
		const UAtomShooterWeaponData* Data = Instance->ItemDef->ShooterWeaponData;
		if (Data->FireAbilitySet)
		{
			Data->FireAbilitySet->GiveToAbilitySystem(ASC, Instance, ActiveWeaponAbilityHandles);
		}
		if (Data->ReloadAbilitySet)
		{
			Data->ReloadAbilitySet->GiveToAbilitySystem(ASC, Instance, ActiveWeaponAbilityHandles);
		}
	}

	if (Instance->ItemDef->ARPGWeaponData)
	{
		const UAtomARPGWeaponData* Data = Instance->ItemDef->ARPGWeaponData;
		if (Data->DefaultAttackAbilitySet)
		{
			Data->DefaultAttackAbilitySet->GiveToAbilitySystem(ASC, Instance, ActiveWeaponAbilityHandles);
		}
	}

	// Push stats to attribute set
	ApplyWeaponStatsToASC(Instance);
}

void UAtomWeaponManagerComponent::DeactivateWeapon(UAtomItemInstance* Instance)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (ASC)
	{
		ActiveWeaponAbilityHandles.RevokeFromASC(ASC);
	}

	ClearWeaponStatsFromASC();
}

void UAtomWeaponManagerComponent::GrantWeaponSetAbilities(int32 SetIndex)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	TArray<UAtomItemInstance*> Weapons = GetWeaponsInSet(SetIndex);
	for (UAtomItemInstance* Weapon : Weapons)
	{
		if (Weapon)
		{
			Weapon->GrantSocketedGemAbilities(ASC);
		}
	}
}

void UAtomWeaponManagerComponent::RevokeWeaponSetAbilities(int32 SetIndex)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	TArray<UAtomItemInstance*> Weapons = GetWeaponsInSet(SetIndex);
	for (UAtomItemInstance* Weapon : Weapons)
	{
		if (Weapon)
		{
			Weapon->RevokeSocketedGemAbilities(ASC);
		}
	}
}
