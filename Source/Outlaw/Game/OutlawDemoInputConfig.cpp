#include "Game/OutlawDemoInputConfig.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"

// Static member definitions
UInputMappingContext* UOutlawDemoInputConfig::CachedIMC = nullptr;
UInputAction* UOutlawDemoInputConfig::MoveAction = nullptr;
UInputAction* UOutlawDemoInputConfig::LookAction = nullptr;
UInputAction* UOutlawDemoInputConfig::JumpAction = nullptr;
UInputAction* UOutlawDemoInputConfig::FireAction = nullptr;
UInputAction* UOutlawDemoInputConfig::ADSAction = nullptr;
UInputAction* UOutlawDemoInputConfig::ReloadAction = nullptr;
UInputAction* UOutlawDemoInputConfig::CycleWeaponAction = nullptr;
UInputAction* UOutlawDemoInputConfig::InventoryToggleAction = nullptr;
UInputAction* UOutlawDemoInputConfig::FullInventoryAction = nullptr;
UInputAction* UOutlawDemoInputConfig::SkillTreeAction = nullptr;
bool UOutlawDemoInputConfig::bInitialized = false;

UInputAction* UOutlawDemoInputConfig::CreateAction(UObject* Outer, const FName& Name, EInputActionValueType ValueType)
{
	UInputAction* Action = NewObject<UInputAction>(Outer, Name);
	Action->ValueType = ValueType;
	Action->AddToRoot();
	return Action;
}

void UOutlawDemoInputConfig::EnsureCreated()
{
	if (bInitialized)
	{
		return;
	}
	bInitialized = true;

	UObject* Outer = GetTransientPackage();

	MoveAction = CreateAction(Outer, TEXT("IA_DemoMove"), EInputActionValueType::Axis2D);
	LookAction = CreateAction(Outer, TEXT("IA_DemoLook"), EInputActionValueType::Axis2D);
	JumpAction = CreateAction(Outer, TEXT("IA_DemoJump"), EInputActionValueType::Boolean);
	FireAction = CreateAction(Outer, TEXT("IA_DemoFire"), EInputActionValueType::Boolean);
	ADSAction = CreateAction(Outer, TEXT("IA_DemoADS"), EInputActionValueType::Boolean);
	ReloadAction = CreateAction(Outer, TEXT("IA_DemoReload"), EInputActionValueType::Boolean);
	CycleWeaponAction = CreateAction(Outer, TEXT("IA_DemoCycleWeapon"), EInputActionValueType::Boolean);
	InventoryToggleAction = CreateAction(Outer, TEXT("IA_DemoInventoryToggle"), EInputActionValueType::Boolean);
	FullInventoryAction = CreateAction(Outer, TEXT("IA_DemoFullInventory"), EInputActionValueType::Boolean);
	SkillTreeAction = CreateAction(Outer, TEXT("IA_DemoSkillTree"), EInputActionValueType::Boolean);

	CachedIMC = NewObject<UInputMappingContext>(Outer, TEXT("IMC_Demo"));
	CachedIMC->AddToRoot();

	// WASD Movement
	auto AddMoveKey = [](UInputMappingContext* IMC, UInputAction* Action, FKey Key, bool bSwizzle, bool bNegate)
	{
		FEnhancedActionKeyMapping& Mapping = IMC->MapKey(Action, Key);
		if (bSwizzle)
		{
			UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(IMC);
			Swizzle->Order = EInputAxisSwizzle::YXZ;
			Mapping.Modifiers.Add(Swizzle);
		}
		if (bNegate)
		{
			UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(IMC);
			Mapping.Modifiers.Add(Negate);
		}
	};

	AddMoveKey(CachedIMC, MoveAction, EKeys::W, true, false);   // Forward (+Y)
	AddMoveKey(CachedIMC, MoveAction, EKeys::S, true, true);    // Backward (-Y)
	AddMoveKey(CachedIMC, MoveAction, EKeys::D, false, false);  // Right (+X)
	AddMoveKey(CachedIMC, MoveAction, EKeys::A, false, true);   // Left (-X)

	// Mouse Look
	CachedIMC->MapKey(LookAction, EKeys::Mouse2D);

	// Jump
	CachedIMC->MapKey(JumpAction, EKeys::SpaceBar);

	// Fire
	CachedIMC->MapKey(FireAction, EKeys::LeftMouseButton);

	// ADS
	CachedIMC->MapKey(ADSAction, EKeys::RightMouseButton);

	// Reload
	CachedIMC->MapKey(ReloadAction, EKeys::R);

	// Cycle Weapon
	CachedIMC->MapKey(CycleWeaponAction, EKeys::MouseScrollUp);

	// Inventory Toggle (Outriders-style equipment popup)
	CachedIMC->MapKey(InventoryToggleAction, EKeys::I);

	// Full Inventory (Destiny-style full screen)
	CachedIMC->MapKey(FullInventoryAction, EKeys::Tab);

	// Skill Tree
	CachedIMC->MapKey(SkillTreeAction, EKeys::P);
}

UInputMappingContext* UOutlawDemoInputConfig::GetOrCreateMappingContext()
{
	EnsureCreated();
	return CachedIMC;
}

UInputAction* UOutlawDemoInputConfig::GetMoveAction()  { EnsureCreated(); return MoveAction; }
UInputAction* UOutlawDemoInputConfig::GetLookAction()  { EnsureCreated(); return LookAction; }
UInputAction* UOutlawDemoInputConfig::GetJumpAction()  { EnsureCreated(); return JumpAction; }
UInputAction* UOutlawDemoInputConfig::GetFireAction()   { EnsureCreated(); return FireAction; }
UInputAction* UOutlawDemoInputConfig::GetADSAction()    { EnsureCreated(); return ADSAction; }
UInputAction* UOutlawDemoInputConfig::GetReloadAction() { EnsureCreated(); return ReloadAction; }
UInputAction* UOutlawDemoInputConfig::GetCycleWeaponAction() { EnsureCreated(); return CycleWeaponAction; }
UInputAction* UOutlawDemoInputConfig::GetInventoryToggleAction() { EnsureCreated(); return InventoryToggleAction; }
UInputAction* UOutlawDemoInputConfig::GetFullInventoryAction() { EnsureCreated(); return FullInventoryAction; }
UInputAction* UOutlawDemoInputConfig::GetSkillTreeAction() { EnsureCreated(); return SkillTreeAction; }
