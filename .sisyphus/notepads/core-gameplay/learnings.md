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
