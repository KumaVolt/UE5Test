#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "OutlawDemoInputConfig.generated.h"

/** Static helper that creates Enhanced Input actions and mapping context for the demo. */
UCLASS()
class OUTLAW_API UOutlawDemoInputConfig : public UObject
{
	GENERATED_BODY()

public:
	/** Creates the demo input mapping context with all key bindings. Caches on first call. */
	static UInputMappingContext* GetOrCreateMappingContext();

	static UInputAction* GetMoveAction();
	static UInputAction* GetLookAction();
	static UInputAction* GetJumpAction();
	static UInputAction* GetFireAction();
	static UInputAction* GetADSAction();
	static UInputAction* GetReloadAction();
	static UInputAction* GetCycleWeaponAction();
	static UInputAction* GetInventoryToggleAction();
	static UInputAction* GetFullInventoryAction();
	static UInputAction* GetSkillTreeAction();

private:
	static UInputAction* CreateAction(UObject* Outer, const FName& Name, EInputActionValueType ValueType);
	static void EnsureCreated();

	static UInputMappingContext* CachedIMC;
	static UInputAction* MoveAction;
	static UInputAction* LookAction;
	static UInputAction* JumpAction;
	static UInputAction* FireAction;
	static UInputAction* ADSAction;
	static UInputAction* ReloadAction;
	static UInputAction* CycleWeaponAction;
	static UInputAction* InventoryToggleAction;
	static UInputAction* FullInventoryAction;
	static UInputAction* SkillTreeAction;
	static bool bInitialized;
};
