# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Build the editor (Mac)
"/Users/Shared/Epic Games/UE_5.7/Engine/Build/BatchFiles/Mac/Build.sh" OutlawEditor Mac Development "/Users/miqueldelafuente/Projects/Outlaw/Outlaw.uproject" -waitmutex
```

There is no test framework or linter configured. Validation is done by compiling successfully.

## Project Overview

Unreal Engine 5.7 action game combining Outriders-style third-person shooting with Path of Exile 2-style ARPG loot/progression. Single C++ module (`Outlaw`) using GAS (Gameplay Ability System), CommonUI, and Enhanced Input.

**Plugins**: GameplayAbilities, CommonUI, ModelingToolsEditorMode (editor only)

## Architecture

### GAS Ownership Pattern
- **Players**: ASC lives on `AOutlawPlayerState` (Mixed replication mode) — survives pawn respawn
- **Enemies**: ASC lives on `AOutlawEnemyCharacter` (Minimal replication) — dies with pawn
- Both implement `IAbilitySystemInterface`

### Data-Driven Ability Sets
All ability granting flows through `UOutlawAbilitySet` (a `UPrimaryDataAsset`). Grant/revoke is atomic via `FOutlawAbilitySetGrantedHandles`. No hardcoded ability arrays anywhere.

### Input Tag Routing
Abilities are found by matching gameplay tags (e.g., `Input.Ability.Fire`), not hardcoded bindings. ASC scans for abilities matching the tag in their dynamic source tags.

### Dual-Mode Inventory
`UOutlawInventoryComponent` supports flat slots (Outriders-style, `InventoryGridWidth=0`) or spatial grid (PoE 2-style, `InventoryGridWidth>0`). Equipment grants/revokes ability sets through GAS. Replication uses `FFastArraySerializer`.

### Weapon System (Composition, Not Inheritance)
`UOutlawItemDefinition` holds optional `ShooterWeaponData` and/or `ARPGWeaponData` pointers. `UOutlawItemInstance` stores per-item mutable state (ammo, rolled affixes, socketed gems). `UOutlawWeaponManagerComponent` manages active weapon, cycling, and GAS coordination. Abilities read weapon stats from `UOutlawWeaponAttributeSet`, not weapon data directly.

**SetByCaller affixes**: Each affix uses a generic GE with SetByCaller magnitude — one GE + one data asset per stat, no custom code per affix type.

### Progression System
`UOutlawProgressionComponent` handles XP, leveling, class selection, and skill tree allocation. Two class modes via `EOutlawClassMode`: FixedClass (Outriders-style) and AscendancyClass (PoE 2-style). Stat growth uses `SetNumericAttributeBase()` on existing attributes so GE buffs stack on top.

### AI System
StateTree-based (UE 5.7, not behavior trees). `AOutlawAIController` with perception, task nodes in `AI/Tasks/`, condition nodes in `AI/Conditions/`. `AOutlawEnemySpawner` handles wave-based spawning with difficulty scaling.

### Combat Pipeline
- `UOutlawDamageExecution` — GAS execution calc (weapon damage + stats vs armor)
- `UOutlawDeathComponent` — monitors health, fires death delegates
- `UOutlawEnemyDeathHandler` — ragdoll, dissolve, XP award, loot drop
- `UOutlawPlayerDeathHandler` — respawn at checkpoint
- `UOutlawStatusEffectComponent` — DoT/CC via GAS tags + effects

### Projectile System
`AOutlawProjectileBase` with `BulletProjectile` and `SpellProjectile` variants. `UOutlawProjectilePoolSubsystem` for object pooling. `UOutlawHitscanLibrary` for instant line traces.

### Loot System
`UOutlawLootTable` data assets with rarity weighting. `UOutlawLootSubsystem` generates drops. `AOutlawLootPickup` world actors with rarity beams.

## Source Layout

All source is under `Source/Outlaw/`:

| Directory | Purpose |
|-----------|---------|
| `AbilitySystem/` | ASC, AttributeSets, AbilitySets, base GameplayAbility |
| `Characters/` | CharacterBase, PlayerCharacter, EnemyCharacter |
| `Player/` | PlayerState (owns ASC), PlayerController (creates HUD) |
| `Inventory/` | ItemDefinition, ItemInstance, InventoryComponent, types |
| `Weapon/` | ShooterData, ARPGData, Affixes, Gems, Mods, WeaponManager |
| `Progression/` | LevelingConfig, ClassDefinition, SkillTreeNodes, ProgressionComponent |
| `Combat/` | DamageExecution, Death, StatusEffects, HitReactions, DamageNumbers, CombatLog |
| `Animation/` | AnimInstance, AnimNotifies (DamageWindow, Sound, Effect), HitReactions |
| `Projectile/` | ProjectileBase, Bullet, Spell, PoolSubsystem, HitscanLibrary |
| `Loot/` | LootTable, LootPickup, LootSubsystem, LootBeam |
| `Camera/` | CameraComponent (OTS + isometric), LockOnComponent |
| `AI/` | AIController, DifficultyConfig, Spawner, Tasks/, Conditions/ |
| `UI/` | HUDLayout, StatBar, DamageNumberWidget |

## Module Dependencies

**Build.cs** (`Source/Outlaw/Outlaw.Build.cs`):
- **Public**: Core, CoreUObject, Engine, InputCore, EnhancedInput, GameplayAbilities, GameplayTags, GameplayTasks, UMG, CommonUI, CommonInput, NetCore, AIModule, NavigationSystem, Niagara, AnimGraphRuntime
- **Private**: Slate, SlateCore, StateTreeModule, GameplayStateTreeModule
- PCH mode: `UseExplicitOrSharedPCHs`
- Public include path: `Outlaw/` (allows `#include "SubDir/Header.h"`)

## Code Conventions

- **Prefix**: `Outlaw` on all project classes/structs/enums (e.g., `AOutlawPlayerCharacter`, `FOutlawInventoryEntry`, `EOutlawItemType`)
- **Delegates**: `FOnInventoryChanged`, `FOnDeathStarted` pattern
- **Booleans**: `bCanBeEquipped`, `bAutoLoot` prefix
- **Headers**: `#pragma once`, forward-declare where possible, CoreMinimal first
- **Replication**: Always check `HasAuthority()` before mutating. Use `FFastArraySerializer` for arrays, `DOREPLIFETIME` for simple properties
- **GAS access**: Components find ASC via `IAbilitySystemInterface` cast on owner (works for both character-owned and PlayerState-owned ASCs)
- **Design philosophy**: Thin C++ infrastructure, rich Blueprint content. Composition over inheritance. Data assets over hardcoded values.

## UE 5.7 Gotchas

- **`TMap` cannot be replicated** — use `TArray` of structs instead
- **`FFastArraySerializer`** symbols require `NetCore` in Build.cs dependencies
- **`APlayerState`** needs explicit `#include "GameFramework/PlayerState.h"` (not always pulled in by shared PCH)
- **Unity build** file composition changes when adding new .cpp files — can expose missing includes in pre-existing code
- **`GetDynamicSpecSourceTags()`** is the current API (not the deprecated `DynamicAbilityTags`)
- **Target files** use `IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7`

## Key Design Decisions

1. **ASC on PlayerState** — survives pawn respawn, standard multiplayer pattern
2. **Atomic ability sets** — `UOutlawAbilitySet` groups abilities/effects/attributes as a single grantable unit
3. **Input tag routing** — no hardcoded ability bindings, abilities matched by gameplay tags
4. **Dual-mode inventory** — one component, two configurations (flat vs grid), all features work in both modes
5. **Composition weapons** — optional data pointers on ItemDefinition, not subclasses
6. **Weapon attribute set** — abilities read from GAS attributes (enables buffs/debuffs), not raw weapon data
7. **SetByCaller affixes** — generic GE per stat mod, rolled value passed via tag
8. **SetNumericAttributeBase stat growth** — progression sets base values, GE buffs/debuffs stack on top
9. **StateTree AI** — UE 5.7 modern approach, replaces behavior trees
10. **Server-authoritative** — all state mutations gated by `HasAuthority()`

## Documentation

- **README.md** — comprehensive architecture guide with detailed Blueprint setup instructions for every system
- **FEATURES.md** — feature roadmap organized by priority tier (Tier 1: Core, Tier 2: Depth, Tier 3: Polish)
- **OUTRIDERS-DEMO.md** — step-by-step demo setup guide
