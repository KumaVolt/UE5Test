#include "AbilitySystem/Effects/OutlawEffect_Damage.h"
#include "Combat/AtomDamageExecution.h"

UOutlawEffect_Damage::UOutlawEffect_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition ExecDef;
	ExecDef.CalculationClass = UAtomDamageExecution::StaticClass();
	Executions.Add(ExecDef);
}
