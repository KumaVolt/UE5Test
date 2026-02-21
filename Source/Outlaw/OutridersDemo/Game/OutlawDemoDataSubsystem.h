#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OutlawDemoDataSubsystem.generated.h"

class UAtomShooterWeaponData;
class UAtomItemDefinition;
class UAtomLootTable;
class UAtomClassDefinition;
class UAtomLevelingConfig;

/** World subsystem that creates and holds all demo data assets in C++ (no Blueprint assets). */
UCLASS()
class OUTLAW_API UOutlawDemoDataSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UAtomShooterWeaponData* GetAssaultRifleData() const { return AssaultRifleData; }
	UAtomItemDefinition* GetAssaultRifleItemDef() const { return AssaultRifleItemDef; }
	UAtomItemDefinition* GetHealthPackItemDef() const { return HealthPackItemDef; }
	UAtomItemDefinition* GetAmmoPackItemDef() const { return AmmoPackItemDef; }
	UAtomItemDefinition* GetRareRifleItemDef() const { return RareRifleItemDef; }
	UAtomShooterWeaponData* GetHandCannonData() const { return HandCannonData; }
	UAtomItemDefinition* GetHandCannonItemDef() const { return HandCannonItemDef; }
	UAtomItemDefinition* GetRareHandCannonItemDef() const { return RareHandCannonItemDef; }
	UAtomLootTable* GetBasicEnemyLootTable() const { return BasicEnemyLootTable; }
	UAtomClassDefinition* GetDemoClassDef() const { return DemoClassDef; }
	UAtomLevelingConfig* GetDemoLevelingConfig() const { return DemoLevelingConfig; }

private:
	UPROPERTY()
	TObjectPtr<UAtomShooterWeaponData> AssaultRifleData;

	UPROPERTY()
	TObjectPtr<UAtomShooterWeaponData> RareAssaultRifleData;

	UPROPERTY()
	TObjectPtr<UAtomShooterWeaponData> HandCannonData;

	UPROPERTY()
	TObjectPtr<UAtomShooterWeaponData> RareHandCannonData;

	UPROPERTY()
	TObjectPtr<UAtomItemDefinition> AssaultRifleItemDef;

	UPROPERTY()
	TObjectPtr<UAtomItemDefinition> HealthPackItemDef;

	UPROPERTY()
	TObjectPtr<UAtomItemDefinition> AmmoPackItemDef;

	UPROPERTY()
	TObjectPtr<UAtomItemDefinition> RareRifleItemDef;

	UPROPERTY()
	TObjectPtr<UAtomItemDefinition> HandCannonItemDef;

	UPROPERTY()
	TObjectPtr<UAtomItemDefinition> RareHandCannonItemDef;

	UPROPERTY()
	TObjectPtr<UAtomLootTable> BasicEnemyLootTable;

	UPROPERTY()
	TObjectPtr<UAtomClassDefinition> DemoClassDef;

	UPROPERTY()
	TObjectPtr<UAtomLevelingConfig> DemoLevelingConfig;
};
