#include "Game/OutlawDemoDataSubsystem.h"
#include "Weapon/OutlawShooterWeaponData.h"
#include "Inventory/OutlawItemDefinition.h"
#include "Loot/OutlawLootTable.h"
#include "Loot/OutlawLootTypes.h"
#include "Progression/OutlawClassDefinition.h"
#include "Progression/OutlawLevelingConfig.h"
#include "Progression/OutlawSkillTreeNodeDefinition.h"

void UOutlawDemoDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// ── Weapon Data ─────────────────────────────────────────────

	// ── Assault Rifle (Outriders-style: full-auto spray) ────────
	AssaultRifleData = NewObject<UOutlawShooterWeaponData>(this, TEXT("DA_AssaultRifle"));
	AssaultRifleData->bAutomatic = true;
	AssaultRifleData->Firepower = 25.f;
	AssaultRifleData->RPM = 600.f;
	AssaultRifleData->MagazineSize = 30;
	AssaultRifleData->ReloadTime = 2.f;
	AssaultRifleData->Range = 5000.f;
	AssaultRifleData->Accuracy = 75.f;
	AssaultRifleData->Stability = 60.f;
	AssaultRifleData->CritMultiplier = 1.5f;

	RareAssaultRifleData = NewObject<UOutlawShooterWeaponData>(this, TEXT("DA_RareAssaultRifle"));
	RareAssaultRifleData->bAutomatic = true;
	RareAssaultRifleData->Firepower = 40.f;
	RareAssaultRifleData->RPM = 600.f;
	RareAssaultRifleData->MagazineSize = 30;
	RareAssaultRifleData->ReloadTime = 2.f;
	RareAssaultRifleData->Range = 5000.f;
	RareAssaultRifleData->Accuracy = 85.f;
	RareAssaultRifleData->Stability = 75.f;
	RareAssaultRifleData->CritMultiplier = 1.8f;

	// ── Hand Cannon (Destiny-style: precise semi-auto) ──────────
	HandCannonData = NewObject<UOutlawShooterWeaponData>(this, TEXT("DA_HandCannon"));
	HandCannonData->bAutomatic = false;
	HandCannonData->BurstCount = 1;
	HandCannonData->WeaponType = EOutlawShooterWeaponType::Revolver;
	HandCannonData->Firepower = 80.f;
	HandCannonData->RPM = 140.f;
	HandCannonData->MagazineSize = 8;
	HandCannonData->ReloadTime = 1.8f;
	HandCannonData->Range = 4000.f;
	HandCannonData->Accuracy = 92.f;
	HandCannonData->Stability = 85.f;
	HandCannonData->CritMultiplier = 2.0f;

	RareHandCannonData = NewObject<UOutlawShooterWeaponData>(this, TEXT("DA_RareHandCannon"));
	RareHandCannonData->bAutomatic = false;
	RareHandCannonData->BurstCount = 1;
	RareHandCannonData->WeaponType = EOutlawShooterWeaponType::Revolver;
	RareHandCannonData->Firepower = 120.f;
	RareHandCannonData->RPM = 140.f;
	RareHandCannonData->MagazineSize = 8;
	RareHandCannonData->ReloadTime = 1.5f;
	RareHandCannonData->Range = 4500.f;
	RareHandCannonData->Accuracy = 95.f;
	RareHandCannonData->Stability = 88.f;
	RareHandCannonData->CritMultiplier = 2.2f;

	// ── Item Definitions ────────────────────────────────────────

	AssaultRifleItemDef = NewObject<UOutlawItemDefinition>(this, TEXT("ID_AssaultRifle"));
	AssaultRifleItemDef->DisplayName = FText::FromString(TEXT("Assault Rifle"));
	AssaultRifleItemDef->ItemType = EOutlawItemType::Weapon;
	AssaultRifleItemDef->Rarity = EOutlawItemRarity::Common;
	AssaultRifleItemDef->MaxStackSize = 1;
	AssaultRifleItemDef->bCanBeEquipped = true;
	AssaultRifleItemDef->EquipmentSlotTag = FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Primary1"));
	AssaultRifleItemDef->ShooterWeaponData = AssaultRifleData;

	HealthPackItemDef = NewObject<UOutlawItemDefinition>(this, TEXT("ID_HealthPack"));
	HealthPackItemDef->DisplayName = FText::FromString(TEXT("Health Pack"));
	HealthPackItemDef->ItemType = EOutlawItemType::Consumable;
	HealthPackItemDef->Rarity = EOutlawItemRarity::Common;
	HealthPackItemDef->MaxStackSize = 10;

	AmmoPackItemDef = NewObject<UOutlawItemDefinition>(this, TEXT("ID_AmmoPack"));
	AmmoPackItemDef->DisplayName = FText::FromString(TEXT("Ammo Pack"));
	AmmoPackItemDef->ItemType = EOutlawItemType::Consumable;
	AmmoPackItemDef->Rarity = EOutlawItemRarity::Common;
	AmmoPackItemDef->MaxStackSize = 50;

	RareRifleItemDef = NewObject<UOutlawItemDefinition>(this, TEXT("ID_RareRifle"));
	RareRifleItemDef->DisplayName = FText::FromString(TEXT("Modified Assault Rifle"));
	RareRifleItemDef->ItemType = EOutlawItemType::Weapon;
	RareRifleItemDef->Rarity = EOutlawItemRarity::Rare;
	RareRifleItemDef->MaxStackSize = 1;
	RareRifleItemDef->bCanBeEquipped = true;
	RareRifleItemDef->EquipmentSlotTag = FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Primary1"));
	RareRifleItemDef->ShooterWeaponData = RareAssaultRifleData;

	HandCannonItemDef = NewObject<UOutlawItemDefinition>(this, TEXT("ID_HandCannon"));
	HandCannonItemDef->DisplayName = FText::FromString(TEXT("Hand Cannon"));
	HandCannonItemDef->ItemType = EOutlawItemType::Weapon;
	HandCannonItemDef->Rarity = EOutlawItemRarity::Uncommon;
	HandCannonItemDef->MaxStackSize = 1;
	HandCannonItemDef->bCanBeEquipped = true;
	HandCannonItemDef->EquipmentSlotTag = FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Primary2"));
	HandCannonItemDef->ShooterWeaponData = HandCannonData;

	RareHandCannonItemDef = NewObject<UOutlawItemDefinition>(this, TEXT("ID_RareHandCannon"));
	RareHandCannonItemDef->DisplayName = FText::FromString(TEXT("Precision Hand Cannon"));
	RareHandCannonItemDef->ItemType = EOutlawItemType::Weapon;
	RareHandCannonItemDef->Rarity = EOutlawItemRarity::Rare;
	RareHandCannonItemDef->MaxStackSize = 1;
	RareHandCannonItemDef->bCanBeEquipped = true;
	RareHandCannonItemDef->EquipmentSlotTag = FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Primary2"));
	RareHandCannonItemDef->ShooterWeaponData = RareHandCannonData;

	// ── Loot Table ──────────────────────────────────────────────

	BasicEnemyLootTable = NewObject<UOutlawLootTable>(this, TEXT("LT_BasicEnemy"));

	FOutlawLootTableEntry HealthEntry;
	HealthEntry.ItemDefinition = HealthPackItemDef;
	HealthEntry.Weight = 50.f;
	HealthEntry.MinItemLevel = 1;
	HealthEntry.MaxItemLevel = 99;
	HealthEntry.MinQuantity = 1;
	HealthEntry.MaxQuantity = 3;
	BasicEnemyLootTable->Entries.Add(HealthEntry);

	FOutlawLootTableEntry AmmoEntry;
	AmmoEntry.ItemDefinition = AmmoPackItemDef;
	AmmoEntry.Weight = 40.f;
	AmmoEntry.MinItemLevel = 1;
	AmmoEntry.MaxItemLevel = 99;
	AmmoEntry.MinQuantity = 5;
	AmmoEntry.MaxQuantity = 20;
	BasicEnemyLootTable->Entries.Add(AmmoEntry);

	FOutlawLootTableEntry RareRifleEntry;
	RareRifleEntry.ItemDefinition = RareRifleItemDef;
	RareRifleEntry.Weight = 10.f;
	RareRifleEntry.MinItemLevel = 3;
	RareRifleEntry.MaxItemLevel = 99;
	RareRifleEntry.MinQuantity = 1;
	RareRifleEntry.MaxQuantity = 1;
	BasicEnemyLootTable->Entries.Add(RareRifleEntry);

	FOutlawLootTableEntry RareHCEntry;
	RareHCEntry.ItemDefinition = RareHandCannonItemDef;
	RareHCEntry.Weight = 8.f;
	RareHCEntry.MinItemLevel = 3;
	RareHCEntry.MaxItemLevel = 99;
	RareHCEntry.MinQuantity = 1;
	RareHCEntry.MaxQuantity = 1;
	BasicEnemyLootTable->Entries.Add(RareHCEntry);

	// ── Leveling Config ────────────────────────────────────────

	DemoLevelingConfig = NewObject<UOutlawLevelingConfig>(this, TEXT("LC_Demo"));
	DemoLevelingConfig->DefaultSkillPointsPerLevel = 1;
	for (int32 i = 0; i < 10; ++i)
	{
		FOutlawXPLevelEntry Entry;
		Entry.RequiredXP = (i + 1) * 100; // 100, 200, 300, ..., 1000
		Entry.SkillPointsAwarded = 1;
		DemoLevelingConfig->LevelTable.Add(Entry);
	}

	// ── Skill Tree Nodes ───────────────────────────────────────

	auto MakeNode = [this](const FName& Name, const TCHAR* TagStr, const FText& DisplayName,
		float PosX, float PosY, int32 MaxRank, int32 Cost) -> UOutlawSkillTreeNodeDefinition*
	{
		UOutlawSkillTreeNodeDefinition* Node = NewObject<UOutlawSkillTreeNodeDefinition>(this, Name);
		Node->NodeTag = FGameplayTag::RequestGameplayTag(FName(TagStr));
		Node->DisplayName = DisplayName;
		Node->MaxRank = MaxRank;
		Node->PointCostPerRank = Cost;
		Node->TreePositionX = PosX;
		Node->TreePositionY = PosY;
		return Node;
	};

	UOutlawSkillTreeNodeDefinition* HealthBoost = MakeNode(
		TEXT("SN_HealthBoost"), TEXT("Skill.Demo.HealthBoost"),
		FText::FromString(TEXT("Health Boost")), -1.5f, -3.f, 3, 1);

	UOutlawSkillTreeNodeDefinition* StaminaPool = MakeNode(
		TEXT("SN_StaminaPool"), TEXT("Skill.Demo.StaminaPool"),
		FText::FromString(TEXT("Stamina Pool")), 1.5f, -3.f, 3, 1);

	UOutlawSkillTreeNodeDefinition* ArmorUp = MakeNode(
		TEXT("SN_ArmorUp"), TEXT("Skill.Demo.ArmorUp"),
		FText::FromString(TEXT("Armor Up")), -1.5f, -1.5f, 2, 1);
	{
		FOutlawSkillNodePrerequisite Prereq;
		Prereq.RequiredNodeTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Demo.HealthBoost"));
		Prereq.RequiredRank = 1;
		ArmorUp->Prerequisites.Add(Prereq);
	}

	UOutlawSkillTreeNodeDefinition* CritChance = MakeNode(
		TEXT("SN_CritChance"), TEXT("Skill.Demo.CritChance"),
		FText::FromString(TEXT("Critical Eye")), 1.5f, -1.5f, 2, 1);
	{
		FOutlawSkillNodePrerequisite Prereq;
		Prereq.RequiredNodeTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Demo.StaminaPool"));
		Prereq.RequiredRank = 1;
		CritChance->Prerequisites.Add(Prereq);
	}

	UOutlawSkillTreeNodeDefinition* DamageBoost = MakeNode(
		TEXT("SN_DamageBoost"), TEXT("Skill.Demo.DamageBoost"),
		FText::FromString(TEXT("Raw Power")), 0.f, 0.f, 1, 1);
	{
		FOutlawSkillNodePrerequisite Prereq;
		Prereq.RequiredNodeTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Demo.ArmorUp"));
		Prereq.RequiredRank = 1;
		DamageBoost->Prerequisites.Add(Prereq);
	}

	UOutlawSkillTreeNodeDefinition* LifeSteal = MakeNode(
		TEXT("SN_LifeSteal"), TEXT("Skill.Demo.LifeSteal"),
		FText::FromString(TEXT("Life Steal")), 0.f, 1.5f, 1, 1);
	{
		FOutlawSkillNodePrerequisite Prereq;
		Prereq.RequiredNodeTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Demo.DamageBoost"));
		Prereq.RequiredRank = 1;
		LifeSteal->Prerequisites.Add(Prereq);
	}

	UOutlawSkillTreeNodeDefinition* GlassCannon = MakeNode(
		TEXT("SN_GlassCannon"), TEXT("Skill.Demo.GlassCannon"),
		FText::FromString(TEXT("Glass Cannon")), 0.f, 3.f, 1, 1);
	{
		FOutlawSkillNodePrerequisite Prereq;
		Prereq.RequiredNodeTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Demo.LifeSteal"));
		Prereq.RequiredRank = 1;
		GlassCannon->Prerequisites.Add(Prereq);
	}

	// ── Demo Class Definition ──────────────────────────────────

	DemoClassDef = NewObject<UOutlawClassDefinition>(this, TEXT("CD_DemoOutlaw"));
	DemoClassDef->ClassTag = FGameplayTag::RequestGameplayTag(TEXT("Class.Demo.Outlaw"));
	DemoClassDef->DisplayName = FText::FromString(TEXT("Outlaw"));
	DemoClassDef->Description = FText::FromString(TEXT("A versatile gunslinger specializing in survivability and raw damage."));
	DemoClassDef->ClassMode = EOutlawClassMode::FixedClass;
	DemoClassDef->bIsBaseClass = true;
	DemoClassDef->LevelingConfig = DemoLevelingConfig;
	DemoClassDef->SkillTreeNodes.Add(HealthBoost);
	DemoClassDef->SkillTreeNodes.Add(StaminaPool);
	DemoClassDef->SkillTreeNodes.Add(ArmorUp);
	DemoClassDef->SkillTreeNodes.Add(CritChance);
	DemoClassDef->SkillTreeNodes.Add(DamageBoost);
	DemoClassDef->SkillTreeNodes.Add(LifeSteal);
	DemoClassDef->SkillTreeNodes.Add(GlassCannon);

	UE_LOG(LogTemp, Log, TEXT("OutlawDemoDataSubsystem: All demo data assets created."));
}
