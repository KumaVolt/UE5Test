// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomItemInstance.h"
#include "AtomItemDefinition.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AtomAbilitySet.h"
#include "Weapon/AtomAffixDefinition.h"
#include "Weapon/AtomAffixPoolDefinition.h"
#include "Weapon/AtomSkillGemDefinition.h"
#include "Weapon/AtomWeaponModDefinition.h"
#include "Weapon/AtomARPGWeaponData.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtomItemInstance, Log, All);

UAtomItemInstance::UAtomItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// ── Shooter Mod API ─────────────────────────────────────────────

void UAtomItemInstance::InstallMod(UAtomWeaponModDefinition* ModDef, int32 Tier, UAbilitySystemComponent* ASC)
{
	if (!ModDef || !ASC || (Tier != 1 && Tier != 2))
	{
		return;
	}

	// Remove existing mod in this tier first
	RemoveMod(Tier, ASC);

	if (Tier == 1)
	{
		InstalledModTier1 = ModDef;
		if (ModDef->GrantedAbilitySet)
		{
			ModDef->GrantedAbilitySet->GiveToAbilitySystem(ASC, this, ModTier1Handles);
		}
	}
	else
	{
		InstalledModTier2 = ModDef;
		if (ModDef->GrantedAbilitySet)
		{
			ModDef->GrantedAbilitySet->GiveToAbilitySystem(ASC, this, ModTier2Handles);
		}
	}
}

void UAtomItemInstance::RemoveMod(int32 Tier, UAbilitySystemComponent* ASC)
{
	if (!ASC || (Tier != 1 && Tier != 2))
	{
		return;
	}

	if (Tier == 1)
	{
		if (InstalledModTier1)
		{
			ModTier1Handles.RevokeFromASC(ASC);
			InstalledModTier1 = nullptr;
		}
	}
	else
	{
		if (InstalledModTier2)
		{
			ModTier2Handles.RevokeFromASC(ASC);
			InstalledModTier2 = nullptr;
		}
	}
}

// ── ARPG Gem API ────────────────────────────────────────────────

bool UAtomItemInstance::SocketGem(UAtomSkillGemDefinition* GemDef, int32 SocketIndex)
{
	if (!GemDef || !SocketSlots.IsValidIndex(SocketIndex))
	{
		return false;
	}

	FAtomSocketSlot& Socket = SocketSlots[SocketIndex];

	// Check socket type compatibility
	if (Socket.SocketTypeTag.IsValid() && GemDef->RequiredSocketTypeTag.IsValid()
		&& Socket.SocketTypeTag != GemDef->RequiredSocketTypeTag)
	{
		UE_LOG(LogAtomItemInstance, Warning, TEXT("SocketGem: Gem '%s' requires socket type '%s' but socket has '%s'."),
			*GemDef->DisplayName.ToString(), *GemDef->RequiredSocketTypeTag.ToString(), *Socket.SocketTypeTag.ToString());
		return false;
	}

	// Remove existing gem if any
	if (Socket.SocketedGem)
	{
		UnsocketGem(SocketIndex);
	}

	Socket.SocketedGem = GemDef;
	return true;
}

UAtomSkillGemDefinition* UAtomItemInstance::UnsocketGem(int32 SocketIndex)
{
	if (!SocketSlots.IsValidIndex(SocketIndex))
	{
		return nullptr;
	}

	FAtomSocketSlot& Socket = SocketSlots[SocketIndex];
	UAtomSkillGemDefinition* RemovedGem = Socket.SocketedGem;
	Socket.SocketedGem = nullptr;
	return RemovedGem;
}

// ── ARPG Affix API ──────────────────────────────────────────────

void UAtomItemInstance::RollAffixes(int32 ItemLevel)
{
	if (!ItemDef)
	{
		return;
	}

	const UAtomARPGWeaponData* ARPGData = ItemDef->ARPGWeaponData;
	if (!ARPGData || !ARPGData->AffixPool)
	{
		UE_LOG(LogAtomItemInstance, Warning, TEXT("RollAffixes: Item '%s' has no ARPG weapon data or affix pool."),
			*ItemDef->DisplayName.ToString());
		return;
	}

	// Determine number of affixes based on item level
	const int32 MaxPrefixes = ARPGData->AffixPool->MaxPrefixes;
	const int32 MaxSuffixes = ARPGData->AffixPool->MaxSuffixes;

	// Scale affix count with item level (minimum 1 prefix + 1 suffix at any level)
	const int32 NumPrefixes = FMath::Clamp(1 + ItemLevel / 20, 1, MaxPrefixes);
	const int32 NumSuffixes = FMath::Clamp(1 + ItemLevel / 25, 1, MaxSuffixes);

	Affixes = ARPGData->AffixPool->RollAffixes(ItemLevel, NumPrefixes, NumSuffixes);
}

void UAtomItemInstance::GrantAffixEffects(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	// Revoke any existing affix effects first
	RevokeAffixEffects(ASC);

	for (const FAtomItemAffix& Affix : Affixes)
	{
		if (!Affix.AffixDef || !Affix.AffixDef->AffixEffect)
		{
			continue;
		}

		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Affix.AffixDef->AffixEffect, 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			// Pass the rolled value via SetByCaller
			if (Affix.AffixDef->SetByCallerValueTag.IsValid())
			{
				SpecHandle.Data->SetSetByCallerMagnitude(Affix.AffixDef->SetByCallerValueTag, Affix.RolledValue);
			}

			FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			if (EffectHandle.IsValid())
			{
				AffixEffectHandles.Add(EffectHandle);
			}
		}
	}
}

void UAtomItemInstance::RevokeAffixEffects(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	for (const FActiveGameplayEffectHandle& Handle : AffixEffectHandles)
	{
		if (Handle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}
	}
	AffixEffectHandles.Reset();
}

// ── ARPG Gem Ability API ────────────────────────────────────────

void UAtomItemInstance::GrantSocketedGemAbilities(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	// Revoke existing gem abilities first
	RevokeSocketedGemAbilities(ASC);

	SocketedGemHandles.SetNum(SocketSlots.Num());

	for (int32 i = 0; i < SocketSlots.Num(); ++i)
	{
		const FAtomSocketSlot& Socket = SocketSlots[i];
		if (Socket.SocketedGem && Socket.SocketedGem->GrantedAbilitySet)
		{
			Socket.SocketedGem->GrantedAbilitySet->GiveToAbilitySystem(ASC, this, SocketedGemHandles[i]);
		}
	}
}

void UAtomItemInstance::RevokeSocketedGemAbilities(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	for (FAtomAbilitySetGrantedHandles& Handles : SocketedGemHandles)
	{
		Handles.RevokeFromASC(ASC);
	}
	SocketedGemHandles.Reset();
}
