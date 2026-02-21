#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "OutlawEffect_PlayerDefaults.generated.h"

/** Instant GE that sets player default attributes via Override modifiers. */
UCLASS()
class OUTLAW_API UOutlawEffect_PlayerDefaults : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UOutlawEffect_PlayerDefaults();
};
