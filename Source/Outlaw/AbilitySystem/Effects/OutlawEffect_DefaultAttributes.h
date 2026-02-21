#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "OutlawEffect_DefaultAttributes.generated.h"

/** Instant GE that sets enemy default attributes via Override modifiers. */
UCLASS()
class OUTLAW_API UOutlawEffect_DefaultAttributes : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UOutlawEffect_DefaultAttributes();
};
