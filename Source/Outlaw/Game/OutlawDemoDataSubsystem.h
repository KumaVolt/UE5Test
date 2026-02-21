#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OutlawDemoDataSubsystem.generated.h"

class UOutlawShooterWeaponData;
class UOutlawItemDefinition;
class UOutlawLootTable;
class UOutlawClassDefinition;
class UOutlawLevelingConfig;

/** World subsystem that creates and holds all demo data assets in C++ (no Blueprint assets). */
UCLASS()
class OUTLAW_API UOutlawDemoDataSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UOutlawShooterWeaponData* GetAssaultRifleData() const { return AssaultRifleData; }
	UOutlawItemDefinition* GetAssaultRifleItemDef() const { return AssaultRifleItemDef; }
	UOutlawItemDefinition* GetHealthPackItemDef() const { return HealthPackItemDef; }
	UOutlawItemDefinition* GetAmmoPackItemDef() const { return AmmoPackItemDef; }
	UOutlawItemDefinition* GetRareRifleItemDef() const { return RareRifleItemDef; }
	UOutlawShooterWeaponData* GetHandCannonData() const { return HandCannonData; }
	UOutlawItemDefinition* GetHandCannonItemDef() const { return HandCannonItemDef; }
	UOutlawItemDefinition* GetRareHandCannonItemDef() const { return RareHandCannonItemDef; }
	UOutlawLootTable* GetBasicEnemyLootTable() const { return BasicEnemyLootTable; }
	UOutlawClassDefinition* GetDemoClassDef() const { return DemoClassDef; }
	UOutlawLevelingConfig* GetDemoLevelingConfig() const { return DemoLevelingConfig; }

private:
	UPROPERTY()
	TObjectPtr<UOutlawShooterWeaponData> AssaultRifleData;

	UPROPERTY()
	TObjectPtr<UOutlawShooterWeaponData> RareAssaultRifleData;

	UPROPERTY()
	TObjectPtr<UOutlawShooterWeaponData> HandCannonData;

	UPROPERTY()
	TObjectPtr<UOutlawShooterWeaponData> RareHandCannonData;

	UPROPERTY()
	TObjectPtr<UOutlawItemDefinition> AssaultRifleItemDef;

	UPROPERTY()
	TObjectPtr<UOutlawItemDefinition> HealthPackItemDef;

	UPROPERTY()
	TObjectPtr<UOutlawItemDefinition> AmmoPackItemDef;

	UPROPERTY()
	TObjectPtr<UOutlawItemDefinition> RareRifleItemDef;

	UPROPERTY()
	TObjectPtr<UOutlawItemDefinition> HandCannonItemDef;

	UPROPERTY()
	TObjectPtr<UOutlawItemDefinition> RareHandCannonItemDef;

	UPROPERTY()
	TObjectPtr<UOutlawLootTable> BasicEnemyLootTable;

	UPROPERTY()
	TObjectPtr<UOutlawClassDefinition> DemoClassDef;

	UPROPERTY()
	TObjectPtr<UOutlawLevelingConfig> DemoLevelingConfig;
};
