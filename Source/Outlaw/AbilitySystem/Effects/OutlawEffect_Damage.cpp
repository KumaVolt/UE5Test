#include "AbilitySystem/Effects/OutlawEffect_Damage.h"
#include "Combat/OutlawDamageExecution.h"

UOutlawEffect_Damage::UOutlawEffect_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition ExecDef;
	ExecDef.CalculationClass = UOutlawDamageExecution::StaticClass();
	Executions.Add(ExecDef);
}
