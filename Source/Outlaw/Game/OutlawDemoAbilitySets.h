#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/OutlawAbilitySet.h"
#include "OutlawDemoAbilitySets.generated.h"

/** Ability set for the player: fire weapon ability + player default attributes + attribute sets. */
UCLASS()
class OUTLAW_API UOutlawAbilitySet_PlayerDefault : public UOutlawAbilitySet
{
	GENERATED_BODY()

public:
	UOutlawAbilitySet_PlayerDefault();
};

/** Ability set for enemies: melee attack ability + enemy default attributes + attribute set. */
UCLASS()
class OUTLAW_API UOutlawAbilitySet_EnemyDefault : public UOutlawAbilitySet
{
	GENERATED_BODY()

public:
	UOutlawAbilitySet_EnemyDefault();
};
