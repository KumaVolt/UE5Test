# Learnings - Core Gameplay Implementation

## Conventions & Patterns

### Attribute Accessors
- Project uses `ATTRIBUTE_ACCESSORS_BASIC` macro (not standard `ATTRIBUTE_ACCESSORS`)
- All new attributes must follow this pattern for consistency

### Module Dependencies (from existing Build.cs)
**Public Dependencies**:
- Core, CoreUObject, Engine, InputCore, EnhancedInput
- GameplayAbilities, GameplayTags, GameplayTasks
- UMG, CommonUI, CommonInput
- NetCore

**Private Dependencies**:
- Slate, SlateCore

**NEW (Task 0 adds)**:
- AIModule, NavigationSystem, Niagara (public)
- StateTreeModule, GameplayStateTreeModule (private)

### GAS Architecture
- **Player ASC**: Lives on `AOutlawPlayerState` (Mixed replication mode)
- **Enemy ASC**: Lives on `AOutlawEnemyCharacter` (Minimal replication, self-owned)
- **Ability base**: `UOutlawGameplayAbility` with 3 activation policies (OnInputTriggered, OnGranted, OnGameplayEvent)
- **Authority checks**: All C++ infrastructure has HasAuthority() checks despite single-player focus (future-proofing)

### Thin C++ / Rich Blueprint Philosophy
- C++ provides infrastructure (attribute bindings, delegates, input routing)
- Blueprint handles visual layout, styling, designer configuration
- Never expose implementation details to Blueprint that don't need designer control

### Component Pattern
- Reference `OutlawWeaponManagerComponent` for component creation patterns
- Components attach to character, owned by character, lifecycle managed by owner

---

## 2026-02-16 09:15 UTC Task: 0-foundation

### Implementation Summary
- ✅ Added AIModule, NavigationSystem, Niagara to PublicDependencyModuleNames
- ✅ Added StateTreeModule, GameplayStateTreeModule to PrivateDependencyModuleNames  
- ✅ Added Armor, MaxArmor, IncomingDamage attributes to OutlawAttributeSet
- ✅ Implemented GetLifetimeReplicatedProps for Armor/MaxArmor (replicated), excluded IncomingDamage (meta-only)
- ✅ Extended PreAttributeChange to clamp Armor to [0, MaxArmor]
- ✅ Extended PostGameplayEffectExecute to handle IncomingDamage meta attribute
- ✅ Build succeeded with zero compilation errors

### Key Discoveries
- **ATTRIBUTE_ACCESSORS_BASIC pattern confirmed**: All three new attributes follow existing macro (NOT standard ATTRIBUTE_ACCESSORS)
- **OnRep functions required**: Added OnRep_Armor, OnRep_MaxArmor declarations in header + implementations in cpp
- **Meta attribute pattern**: IncomingDamage declared without ReplicatedUsing (no OnRep needed), used as intermediate in PostGameplayEffectExecute
- **Constructor initialization**: Added InitArmor(0.f), InitMaxArmor(0.f) following existing pattern; IncomingDamage needs no Init (meta attribute)
- **Build warnings (non-critical)**: StateTree and GameplayStateTree plugins not listed in uproject but modules load correctly; xcodebuild simulator warnings are OS-level issues

### UE Build Command Notes
- Used Mac Development target with Build.sh script
- **Build result**: ✅ BUILD SUCCEEDED (29.00 seconds total)
- **Post-build warning**: No staged build directory (expected - game binary not needed for validation)
- **Evidence**: Build log saved to `.sisyphus/evidence/task-0-build.log`

*Agents append findings below using ## [TIMESTAMP] Task: {task-id} format*

## 2026-02-16 09:16 UTC Task: Combat / Damage Pipeline

### Implementation Summary
- ✅ Created OutlawCombatTags.h namespace with 8 centralized gameplay tags (Combat.CriticalHit, Status.DoT, Status.CC.*, SetByCaller.*)
- ✅ Implemented OutlawDamageExecution with 7 attribute captures (6 source snapshot + 1 target)
  - Source attributes: Firepower, PhysDmgMin/Max, CritMult, CritChance (weapon set), Strength (character set)
  - Target attributes: Armor (non-snapshot), IncomingDamage (output)
- ✅ Implemented shooter path (Firepower-based) and ARPG path (PhysDmgMin/Max range) with SetByCaller.WeaponType toggle
- ✅ Implemented crit check with Combat.CriticalHit tag output via AddOutputTag
- ✅ Implemented armor mitigation formula: `armor_factor = Armor / (Armor + K)` where `K = 50 + (10 * target_level)`
- ✅ Created OutlawCombatLibrary with 4 Blueprint-callable utilities:
  - PerformLineTrace (ECC_Pawn, optional debug drawing)
  - PerformSphereOverlap (multi-hit sphere sweep)
  - PerformMeleeBoxTrace (oriented box sweep)
  - ApplyDamageToTarget (GE application with SetByCaller magnitudes)
- ✅ Implemented OutlawDamageNumberComponent (listens to IncomingDamage, spawns widgets, detects crit tag)
- ✅ Implemented OutlawDamageNumberWidget (CommonUserWidget with InitDamageNumber + BlueprintImplementableEvent)
- ✅ Implemented OutlawStatusEffectComponent (tracks Status.* tag changes, maintains ActiveEffects array)
- ✅ Created OutlawCombatTypes.h with FOutlawDamageResult, FOutlawActiveStatusEffect structs and all delegates

### Key Discoveries
- **Tag registration pattern**: Project uses `inline const FGameplayTag = FGameplayTag::RequestGameplayTag(TEXT(...))` in namespace, following existing `OutlawAnimationTypes.h` pattern
- **Execution calc pattern**: Uses `DECLARE_ATTRIBUTE_CAPTUREDEF` / `DEFINE_ATTRIBUTE_CAPTUREDEF` macros with FDamageStatics singleton, matches Lyra pattern
- **Snapshot vs non-snapshot**: Source attributes (weapon/character stats) captured with snapshot=true, target attributes (Armor) captured with snapshot=false for dynamic evaluation
- **Output modifier**: IncomingDamage applied via `OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(..., EGameplayModOp::Additive, FinalDamage))`
- **Crit tag output**: Critical hits add `Combat.CriticalHit` tag to spec via `OutExecutionOutput.AddOutputTag(OutlawCombatTags::CriticalHit)` - downstream components detect via `HasMatchingGameplayTag`
- **Component lifecycle**: Both damage number and status effect components use `TWeakObjectPtr<UAbilitySystemComponent>` to avoid dangling references, bind delegates in BeginPlay, cleanup in EndPlay
- **ASC resolution pattern**: Try owner first via `IAbilitySystemInterface`, fallback to owner's instigator (supports both direct ownership and PlayerState patterns)
- **IncomingDamage as signal**: Damage number component listens to IncomingDamage attribute changes (not raw GE application) - automatically triggers on any damage source

### QA Verification Results
- ✅ 8 DEFINE_ATTRIBUTE_CAPTUREDEF found in OutlawDamageExecution.cpp (7 captures + 1 IncomingDamage declaration)
- ✅ IncomingDamage output modifier confirmed: `GetDamageStatics().IncomingDamageDef.AttributeToCapture` with Additive op
- ✅ Tag namespace usage: 5 references to `OutlawCombatTags::` across combat files (CriticalHit x2, SetByCaller x3)
- ✅ LSP diagnostics: False positives only (header-only files before compilation, CoreMinimal.h not resolved by clangd)

### Architecture Notes
- **No elemental damage**: Tier 1 scope limited to physical damage only (PhysDmgMin/Max attributes from weapon set)
- **Framework-only status effects**: Component tracks Status.* tag lifecycle, no specific effects (burn/bleed/freeze) implemented
- **Blueprint integration points**:
  - GE_ApplyDamage blueprint should use OutlawDamageExecution as execution calc class
  - Abilities call `ApplyDamageToTarget` from OutlawCombatLibrary with SetByCaller magnitudes
  - Damage number widget blueprint implements `OnDamageNumberInit` event for animation
  - Status effect component delegates (`OnStatusEffectAdded/Removed`) drive UI updates
- **No friendly fire**: ApplyDamageToTarget has no faction filtering - assumes caller handles targeting logic

### Build Notes
- ⚠️ UE build infrastructure not available (Engine directory missing, Xcode project not generated)
- ✅ Verified via grep/pattern matching:
  - 8 attribute capture definitions present
  - IncomingDamage output modifier correctly structured
  - Tag references use centralized namespace
- ✅ All C++ files created with correct includes, namespace usage, and delegate binding patterns
- **Next steps for Blueprint integration**:
  1. Create `GE_ApplyDamage` gameplay effect with `OutlawDamageExecution` as Execution Calculation Class
  2. Create `WBP_DamageNumber` widget inheriting from `UOutlawDamageNumberWidget`, implement `OnDamageNumberInit` event
  3. Set `DamageNumberWidgetClass` on damage number component to `WBP_DamageNumber`
  4. Blueprint abilities call `ApplyDamageToTarget` with SetByCaller values (WeaponType=1.0 for shooter, 0.0 for ARPG)

*End of Combat / Damage Pipeline learnings*
