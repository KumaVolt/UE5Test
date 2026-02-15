// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawWeaponManagerComponent.h"
#include "Inventory/OutlawItemInstance.h"
#include "Inventory/OutlawItemDefinition.h"
#include "Inventory/OutlawInventoryComponent.h"
#include "OutlawShooterWeaponData.h"
#include "OutlawARPGWeaponData.h"
#include "AbilitySystem/OutlawAbilitySet.h"
#include "AbilitySystem/OutlawWeaponAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogOutlawWeaponManager, Log, All);

UOutlawWeaponManagerComponent::UOutlawWeaponManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UOutlawWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UOutlawWeaponManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UOutlawWeaponManagerComponent, ActiveWeaponSlotTag);
	DOREPLIFETIME(UOutlawWeaponManagerComponent, ActiveWeaponSetIndex);
	DOREPLIFETIME(UOutlawWeaponManagerComponent, ReserveAmmo);
}

// ── Shooter API ─────────────────────────────────────────────────

void UOutlawWeaponManagerComponent::CycleWeapon()
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

void UOutlawWeaponManagerComponent::SwitchToWeaponSlot(FGameplayTag SlotTag)
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
	UOutlawItemInstance* OldWeapon = GetActiveWeapon();
	if (OldWeapon)
	{
		DeactivateWeapon(OldWeapon);
	}

	ActiveWeaponSlotTag = SlotTag;

	// Activate new weapon
	UOutlawItemInstance* NewWeapon = GetWeaponInSlot(SlotTag);
	if (NewWeapon)
	{
		ActivateWeapon(NewWeapon);
	}

	OnActiveWeaponChanged.Broadcast(NewWeapon);
}

UOutlawItemInstance* UOutlawWeaponManagerComponent::GetActiveWeapon() const
{
	if (!ActiveWeaponSlotTag.IsValid())
	{
		return nullptr;
	}
	return GetWeaponInSlot(ActiveWeaponSlotTag);
}

int32 UOutlawWeaponManagerComponent::GetReserveAmmo(FGameplayTag AmmoTypeTag) const
{
	for (const FOutlawReserveAmmoEntry& Entry : ReserveAmmo)
	{
		if (Entry.AmmoTypeTag == AmmoTypeTag)
		{
			return Entry.Amount;
		}
	}
	return 0;
}

void UOutlawWeaponManagerComponent::AddReserveAmmo(FGameplayTag AmmoTypeTag, int32 Amount)
{
	if (!GetOwner()->HasAuthority() || Amount <= 0 || !AmmoTypeTag.IsValid())
	{
		return;
	}

	for (FOutlawReserveAmmoEntry& Entry : ReserveAmmo)
	{
		if (Entry.AmmoTypeTag == AmmoTypeTag)
		{
			Entry.Amount += Amount;
			return;
		}
	}

	// Create new entry
	FOutlawReserveAmmoEntry NewEntry;
	NewEntry.AmmoTypeTag = AmmoTypeTag;
	NewEntry.Amount = Amount;
	ReserveAmmo.Add(NewEntry);
}

int32 UOutlawWeaponManagerComponent::ConsumeReserveAmmo(FGameplayTag AmmoTypeTag, int32 Amount)
{
	if (!GetOwner()->HasAuthority() || Amount <= 0 || !AmmoTypeTag.IsValid())
	{
		return 0;
	}

	for (FOutlawReserveAmmoEntry& Entry : ReserveAmmo)
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

void UOutlawWeaponManagerComponent::SwapWeaponSet()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// Revoke abilities from current set
	RevokeWeaponSetAbilities(ActiveWeaponSetIndex);

	// Deactivate current active weapon (for attribute clearing)
	UOutlawItemInstance* OldWeapon = GetActiveWeapon();
	if (OldWeapon)
	{
		DeactivateWeapon(OldWeapon);
	}

	// Toggle set
	ActiveWeaponSetIndex = (ActiveWeaponSetIndex == 0) ? 1 : 0;

	// Grant abilities for new set
	GrantWeaponSetAbilities(ActiveWeaponSetIndex);

	// Activate first weapon in new set for stats
	TArray<UOutlawItemInstance*> NewSetWeapons = GetWeaponsInSet(ActiveWeaponSetIndex);
	if (NewSetWeapons.Num() > 0 && NewSetWeapons[0])
	{
		ActivateWeapon(NewSetWeapons[0]);
	}

	OnWeaponSetSwapped.Broadcast(ActiveWeaponSetIndex);
	OnActiveWeaponChanged.Broadcast(NewSetWeapons.Num() > 0 ? NewSetWeapons[0] : nullptr);
}

TArray<UOutlawItemInstance*> UOutlawWeaponManagerComponent::GetWeaponsInSet(int32 SetIndex) const
{
	TArray<UOutlawItemInstance*> Result;
	const TArray<FGameplayTag>& SlotTags = (SetIndex == 0) ? ARPGWeaponSetI : ARPGWeaponSetII;

	for (const FGameplayTag& SlotTag : SlotTags)
	{
		UOutlawItemInstance* Instance = GetWeaponInSlot(SlotTag);
		if (Instance)
		{
			Result.Add(Instance);
		}
	}

	return Result;
}

// ── Inventory Callbacks ─────────────────────────────────────────

void UOutlawWeaponManagerComponent::OnWeaponEquipped(UOutlawItemInstance* Instance, FGameplayTag SlotTag)
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

void UOutlawWeaponManagerComponent::OnWeaponUnequipped(UOutlawItemInstance* Instance, FGameplayTag SlotTag)
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

void UOutlawWeaponManagerComponent::ApplyWeaponStatsToASC(UOutlawItemInstance* Instance)
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

	const UOutlawWeaponAttributeSet* WeaponAttribs = ASC->GetSet<UOutlawWeaponAttributeSet>();
	if (!WeaponAttribs)
	{
		return;
	}

	// Cast away const — we're the authority setting base values
	UOutlawWeaponAttributeSet* MutableAttribs = const_cast<UOutlawWeaponAttributeSet*>(WeaponAttribs);

	// Apply shooter stats
	if (Instance->ItemDef->ShooterWeaponData)
	{
		const UOutlawShooterWeaponData* Data = Instance->ItemDef->ShooterWeaponData;
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
		const UOutlawARPGWeaponData* Data = Instance->ItemDef->ARPGWeaponData;
		MutableAttribs->SetPhysicalDamageMin(Data->PhysicalDamageMin);
		MutableAttribs->SetPhysicalDamageMax(Data->PhysicalDamageMax);
		MutableAttribs->SetAttackSpeed(Data->AttackSpeed);
		MutableAttribs->SetCriticalStrikeChance(Data->CriticalStrikeChance);
	}
}

void UOutlawWeaponManagerComponent::ClearWeaponStatsFromASC()
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	const UOutlawWeaponAttributeSet* WeaponAttribs = ASC->GetSet<UOutlawWeaponAttributeSet>();
	if (!WeaponAttribs)
	{
		return;
	}

	UOutlawWeaponAttributeSet* MutableAttribs = const_cast<UOutlawWeaponAttributeSet*>(WeaponAttribs);
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

UAbilitySystemComponent* UOutlawWeaponManagerComponent::GetASC() const
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

UOutlawInventoryComponent* UOutlawWeaponManagerComponent::GetInventoryComponent() const
{
	AActor* Owner = GetOwner();
	return Owner ? Owner->FindComponentByClass<UOutlawInventoryComponent>() : nullptr;
}

UOutlawItemInstance* UOutlawWeaponManagerComponent::GetWeaponInSlot(FGameplayTag SlotTag) const
{
	UOutlawInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory)
	{
		return nullptr;
	}

	return Inventory->GetItemInstance(SlotTag);
}

void UOutlawWeaponManagerComponent::ActivateWeapon(UOutlawItemInstance* Instance)
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
		const UOutlawShooterWeaponData* Data = Instance->ItemDef->ShooterWeaponData;
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
		const UOutlawARPGWeaponData* Data = Instance->ItemDef->ARPGWeaponData;
		if (Data->DefaultAttackAbilitySet)
		{
			Data->DefaultAttackAbilitySet->GiveToAbilitySystem(ASC, Instance, ActiveWeaponAbilityHandles);
		}
	}

	// Push stats to attribute set
	ApplyWeaponStatsToASC(Instance);
}

void UOutlawWeaponManagerComponent::DeactivateWeapon(UOutlawItemInstance* Instance)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (ASC)
	{
		ActiveWeaponAbilityHandles.RevokeFromASC(ASC);
	}

	ClearWeaponStatsFromASC();
}

void UOutlawWeaponManagerComponent::GrantWeaponSetAbilities(int32 SetIndex)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	TArray<UOutlawItemInstance*> Weapons = GetWeaponsInSet(SetIndex);
	for (UOutlawItemInstance* Weapon : Weapons)
	{
		if (Weapon)
		{
			Weapon->GrantSocketedGemAbilities(ASC);
		}
	}
}

void UOutlawWeaponManagerComponent::RevokeWeaponSetAbilities(int32 SetIndex)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	TArray<UOutlawItemInstance*> Weapons = GetWeaponsInSet(SetIndex);
	for (UOutlawItemInstance* Weapon : Weapons)
	{
		if (Weapon)
		{
			Weapon->RevokeSocketedGemAbilities(ASC);
		}
	}
}
