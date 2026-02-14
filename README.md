# Outlaw

Unreal Engine 5.7 action game built on the Gameplay Ability System (GAS) with CommonUI-based HUD.

## Project Structure

```
Source/Outlaw/
  Outlaw.h / .cpp                              Module entry point
  Outlaw.Build.cs                              Module build rules
  AbilitySystem/
    OutlawAbilityTypes.h/.cpp                  Core data structs (bind info, handles, table rows)
    OutlawAbilitySet.h/.cpp                    Data asset grouping abilities, effects & attribute sets
    OutlawAbilitySystemComponent.h/.cpp        Grant/revoke ability sets, input-tag activation
    OutlawAttributeSet.h/.cpp                  Health, MaxHealth, Stamina, MaxStamina, Strength, MaxStrength
    OutlawGameplayAbility.h/.cpp               Base ability with activation policies
  Characters/
    OutlawCharacterBase.h/.cpp                 Abstract base (IAbilitySystemInterface, default ability set)
    OutlawPlayerCharacter.h/.cpp               Player character (Enhanced Input, ASC from PlayerState)
    OutlawEnemyCharacter.h/.cpp                Enemy character (owns its own ASC)
  Player/
    OutlawPlayerState.h/.cpp                   Owns ASC + AttributeSet for players
    OutlawPlayerController.h/.cpp              Creates and manages the HUD widget
  UI/
    OutlawHUDLayout.h/.cpp                     Root HUD widget (CommonActivatableWidget)
    OutlawStatBar.h/.cpp                       Reusable attribute-bound stat bar (CommonUserWidget)
```

## Gameplay Ability System

### Architecture

The GAS setup follows the Lyra-inspired **AbilitySet** pattern — fully data-driven, with no hardcoded ability arrays.

**ASC ownership is split by actor type:**

| Actor | ASC Lives On | Replication Mode | Reason |
|---|---|---|---|
| Player | `AOutlawPlayerState` | Mixed | Persists across pawn respawns |
| Enemy | `AOutlawEnemyCharacter` | Minimal | No PlayerState needed |

### Ability Sets (`UOutlawAbilitySet`)

A `UPrimaryDataAsset` that groups abilities, gameplay effects, and attribute sets into one grantable/revocable unit.

```cpp
// Grant
FOutlawAbilitySetGrantedHandles Handles = ASC->GrantAbilitySet(AbilitySet, SourceObject);

// Revoke
ASC->RevokeAbilitySet(Handles);
```

Each set contains:
- `TArray<FOutlawAbilityBindInfo> Abilities` — ability class, level, input tag, activation tags
- `TArray<FOutlawGrantedEffect> Effects` — passive gameplay effects (e.g. `GE_DefaultAttributes`)
- `TArray<FOutlawGrantedAttributeSet> AttributeSets` — attribute sets to grant dynamically

Can also be populated from a `DataTable` of `FOutlawAbilityTableRow` for spreadsheet-based authoring.

### Attribute Set (`UOutlawAttributeSet`)

Single attribute set with six attributes, all replicated:

| Attribute | Default | Notes |
|---|---|---|
| Health | 80 | Clamped to [0, MaxHealth] in Pre/Post |
| MaxHealth | — | Set via `GE_DefaultAttributes` |
| Stamina | — | |
| MaxStamina | — | |
| Strength | — | |
| MaxStrength | — | |

### Base Ability (`UOutlawGameplayAbility`)

All project abilities should derive from this class. Supports three activation policies:

- **OnInputTriggered** — standard input-activated ability
- **OnGranted** — auto-activates when granted (passives)
- **OnGameplayEvent** — activated by gameplay events

Input is routed through gameplay tags: the ASC's `AbilityInputTagPressed` / `AbilityInputTagReleased` methods scan activatable abilities by their dynamic source tags.

### Character Integration

`AOutlawCharacterBase` holds a `DefaultAbilitySet` property (set in Blueprint defaults) and provides `GrantDefaultAbilitySet()` / `RevokeDefaultAbilitySet()`.

- **Player character**: Calls `InitAbilitySystemComponent()` during `PossessedBy()` and `OnRep_PlayerState()` to grab the ASC/AttributeSet from PlayerState, then grants the default set.
- **Enemy character**: Creates its own ASC/AttributeSet in the constructor, initializes in `BeginPlay`.

## CommonUI HUD

### Architecture

The HUD is created by `AOutlawPlayerController` in `BeginPlayingState()` — this timing guarantees the PlayerState (and its ASC) are valid. The widget persists across pawn respawns since it lives on the controller.

### HUD Layout (`UOutlawHUDLayout`)

Root widget inheriting from `UCommonActivatableWidget`:
- `bAutoActivate = true` — activates immediately when added to viewport
- `GetDesiredInputConfig()` returns `ECommonInputMode::Game` with `CaptureDuringMouseDown` — the HUD never eats game input
- Marked `Abstract, Blueprintable` — actual layout is done in a Blueprint child (`WBP_HUDLayout`)

### Stat Bar (`UOutlawStatBar`)

Reusable widget inheriting from `UCommonUserWidget` that binds to any GAS attribute pair:

- `AttributeToTrack` / `MaxAttributeToTrack` — set in Blueprint defaults (e.g. Health/MaxHealth)
- `InitializeWithAbilitySystem(ASC)` — binds to `GetGameplayAttributeValueChangeDelegate`, reads initial values
- `OnStatChanged(CurrentValue, MaxValue, Percent)` — `BlueprintImplementableEvent` for visual updates
- Properly cleans up delegate bindings in `NativeDestruct()`

Drop multiple instances with different attributes (Health, Stamina, etc.) for a full stat display.

### Blueprint Setup

1. Create `WBP_HUDLayout` (child of `UOutlawHUDLayout`)
   - Add `UOutlawStatBar` children, set their `AttributeToTrack` / `MaxAttributeToTrack`
   - In Event Construct: get owning player → PlayerState → ASC → call `InitializeWithAbilitySystem`
   - Implement `OnStatChanged` to drive a ProgressBar percent
2. Create or update `BP_OutlawPlayerController` (child of `AOutlawPlayerController`)
   - Set `HUDLayoutClass` to `WBP_HUDLayout`
3. Update `BP_OutlawGameMode` to use the new PlayerController class

## Content Assets

| Asset | Type | Location |
|---|---|---|
| `BP_OutlawPlayerCharacter` | Blueprint (AOutlawPlayerCharacter) | Content/Characters/ |
| `BP_OutlawEnemyCharacter` | Blueprint (AOutlawEnemyCharacter) | Content/Characters/ |
| `BP_OutlawPlayerstate` | Blueprint (AOutlawPlayerState) | Content/Players/ |
| `BP_OutlawGameMode` | Blueprint (GameModeBase) | Content/Game/ |
| `DA_DefaultAbilitySet` | Data Asset (UOutlawAbilitySet) | Content/NewFolder/ |
| `GE_DefaultAttributes` | Gameplay Effect | Content/NewFolder/ |
| `IMC_Default` | Input Mapping Context | Content/Input/ |
| `Demo` | Map | Content/Maps/ |

## Module Dependencies

**Public**: Core, CoreUObject, Engine, InputCore, EnhancedInput, GameplayAbilities, GameplayTags, GameplayTasks, UMG, CommonUI, CommonInput

**Private**: Slate, SlateCore

## Building

```bash
# From the Engine's BatchFiles directory:
"<UE_ROOT>/Engine/Build/BatchFiles/Mac/Build.sh" OutlawEditor Mac Development "<PROJECT_ROOT>/Outlaw.uproject"
```

## Key Design Decisions

1. **Data-driven ability system** — No hardcoded ability arrays. Everything flows through `UOutlawAbilitySet` data assets that can be granted/revoked atomically at runtime.
2. **ASC on PlayerState** — Standard multiplayer pattern. The ability system survives pawn death/respawn.
3. **Single attribute set** — All attributes live in one `UOutlawAttributeSet`. Split into separate sets only when different characters need fundamentally different attribute compositions.
4. **CommonUI over raw UMG** — Provides automatic input-method switching (keyboard/mouse ↔ gamepad) with no extra code.
5. **Thin C++ / rich Blueprint** — C++ provides infrastructure (attribute bindings, delegate management, input routing). Blueprint handles visual layout, styling, and designer-facing configuration.
6. **UE 5.7 API** — Uses `GetDynamicSpecSourceTags()` (not deprecated `DynamicAbilityTags`).
