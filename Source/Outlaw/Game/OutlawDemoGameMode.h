#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OutlawDemoGameMode.generated.h"

UCLASS()
class OUTLAW_API AOutlawDemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AOutlawDemoGameMode();
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

protected:
	virtual void BeginPlay() override;

private:
	void BuildArena();
	void SpawnLighting();
	void SpawnEnemySpawner();
	void ConfigureLootSubsystem();
};
