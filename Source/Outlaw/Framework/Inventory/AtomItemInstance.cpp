// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawItemInstance.h"
#include "OutlawItemDefinition.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/OutlawAbilitySet.h"
#include "Weapon/OutlawAffixDefinition.h"
#include "Weapon/OutlawAffixPoolDefinition.h"
#include "Weapon/OutlawSkillGemDefinition.h"
#include "Weapon/OutlawWeaponModDefinition.h"
#include "Weapon/OutlawARPGWeaponData.h"

DEFINE_LOG_CATEGORY_STATIC(LogOutlawItemInstance, Log, All);

UOutlawItemInstance::UOutlawItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// ── Shooter Mod API ─────────────────────────────────────────────

void UOutlawItemInstance::InstallMod(UOutlawWeaponModDefinition* ModDef, int32 Tier, UAbilitySystemComponent* ASC)
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

void UOutlawItemInstance::RemoveMod(int32 Tier, UAbilitySystemComponent* ASC)
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

bool UOutlawItemInstance::SocketGem(UOutlawSkillGemDefinition* GemDef, int32 SocketIndex)
{
	if (!GemDef || !SocketSlots.IsValidIndex(SocketIndex))
	{
		return false;
	}

	FOutlawSocketSlot& Socket = SocketSlots[SocketIndex];

	// Check socket type compatibility
	if (Socket.SocketTypeTag.IsValid() && GemDef->RequiredSocketTypeTag.IsValid()
		&& Socket.SocketTypeTag != GemDef->RequiredSocketTypeTag)
	{
		UE_LOG(LogOutlawItemInstance, Warning, TEXT("SocketGem: Gem '%s' requires socket type '%s' but socket has '%s'."),
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

UOutlawSkillGemDefinition* UOutlawItemInstance::UnsocketGem(int32 SocketIndex)
{
	if (!SocketSlots.IsValidIndex(SocketIndex))
	{
		return nullptr;
	}

	FOutlawSocketSlot& Socket = SocketSlots[SocketIndex];
	UOutlawSkillGemDefinition* RemovedGem = Socket.SocketedGem;
	Socket.SocketedGem = nullptr;
	return RemovedGem;
}

// ── ARPG Affix API ──────────────────────────────────────────────

void UOutlawItemInstance::RollAffixes(int32 ItemLevel)
{
	if (!ItemDef)
	{
		return;
	}

	const UOutlawARPGWeaponData* ARPGData = ItemDef->ARPGWeaponData;
	if (!ARPGData || !ARPGData->AffixPool)
	{
		UE_LOG(LogOutlawItemInstance, Warning, TEXT("RollAffixes: Item '%s' has no ARPG weapon data or affix pool."),
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

void UOutlawItemInstance::GrantAffixEffects(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	// Revoke any existing affix effects first
	RevokeAffixEffects(ASC);

	for (const FOutlawItemAffix& Affix : Affixes)
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

void UOutlawItemInstance::RevokeAffixEffects(UAbilitySystemComponent* ASC)
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

void UOutlawItemInstance::GrantSocketedGemAbilities(UAbilitySystemComponent* ASC)
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
		const FOutlawSocketSlot& Socket = SocketSlots[i];
		if (Socket.SocketedGem && Socket.SocketedGem->GrantedAbilitySet)
		{
			Socket.SocketedGem->GrantedAbilitySet->GiveToAbilitySystem(ASC, this, SocketedGemHandles[i]);
		}
	}
}

void UOutlawItemInstance::RevokeSocketedGemAbilities(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	for (FOutlawAbilitySetGrantedHandles& Handles : SocketedGemHandles)
	{
		Handles.RevokeFromASC(ASC);
	}
	SocketedGemHandles.Reset();
}
