# Outriders-Style Demo — Step-by-Step Build Guide

> **Goal**: Build a playable third-person shooter demo using the Outlaw C++ infrastructure.  
> **Gameplay loop**: Player shoots enemies → enemies fight back with AI → enemies die (ragdoll + dissolve) → loot drops → player picks up loot → player gains XP and levels up.  
> **Time estimate**: ~2–3 hours following this guide.

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Register Gameplay Tags](#2-register-gameplay-tags)
3. [Create Gameplay Effects](#3-create-gameplay-effects)
4. [Set Up the Player Blueprint](#4-set-up-the-player-blueprint)
5. [Create a Damage Number Widget](#5-create-a-damage-number-widget)
6. [Create a Death Screen Widget](#6-create-a-death-screen-widget)
7. [Set Up the Enemy Blueprint](#7-set-up-the-enemy-blueprint)
8. [Create Weapon Data Assets](#8-create-weapon-data-assets)
9. [Create Item Definitions for Loot](#9-create-item-definitions-for-loot)
10. [Create a Loot Table](#10-create-a-loot-table)
11. [Build the AI StateTree](#11-build-the-ai-statetree)
12. [Set Up Animation Blueprints](#12-set-up-animation-blueprints)
13. [Create the Projectile Blueprint](#13-create-the-projectile-blueprint)
14. [Place Enemy Spawners in the Level](#14-place-enemy-spawners-in-the-level)
15. [Wire Up the Game Mode](#15-wire-up-the-game-mode)
16. [Assemble the Demo Map](#16-assemble-the-demo-map)
17. [Testing Checklist](#17-testing-checklist)
18. [Troubleshooting](#18-troubleshooting)

---

## 1. Prerequisites

Before starting, verify the following:

- **Unreal Engine 5.5** (or the version your project targets).
- **Project compiles with zero errors**. Open `Outlaw.sln`, build in `Development Editor`.
- **Existing content assets** are present:
  - `Content/Characters/BP_OutlawPlayerCharacter`
  - `Content/Characters/BP_OutlawEnemyCharacter`
  - `Content/Players/BP_OutlawPlayerstate`
  - `Content/Game/BP_OutlawGameMode`
  - `Content/NewFolder/DA_DefaultAbilitySet`
  - `Content/NewFolder/GE_DefaultAttributes`
  - `Content/Input/IMC_Default`
  - `Content/Maps/Demo`
- **Third-person character meshes** are assigned (Mannequin or custom skeletal mesh with a compatible animation set).

> **Tip**: If you don't have character meshes yet, use the UE5 Manny/Quinn mannequins from the Third Person template.

---

## 2. Register Gameplay Tags

All the C++ systems reference specific Gameplay Tags. They must be registered before anything works.

### Option A: Project Settings (Recommended)

**Project Settings → GameplayTags → Add New Gameplay Tag** — add each tag below:

#### Combat Tags
| Tag | Purpose |
|-----|---------|
| `Combat.CriticalHit` | Applied when a hit crits (used by damage execution) |
| `Combat.DamageWindowActive` | Anim notify window for melee damage |
| `Combat.Targetable` | Actor can be targeted by lock-on |

#### Status Tags
| Tag | Purpose |
|-----|---------|
| `Status.DoT` | Damage-over-time active |
| `Status.CC.Stun` | Crowd control — stun |
| `Status.CC.Slow` | Crowd control — slow |
| `Status.CC.Root` | Crowd control — root |

#### SetByCaller Tags (Damage Pipeline)
| Tag | Purpose |
|-----|---------|
| `SetByCaller.WeaponType` | 1.0 = shooter weapon (used by damage execution) |
| `SetByCaller.TargetLevel` | Enemy level for armor scaling formula |
| `SetByCaller.StrengthScaling` | Strength attribute scaling factor |

#### State Tags
| Tag | Purpose |
|-----|---------|
| `State.Dead` | Character is dead |
| `State.Staggered` | Character is staggered (hit reaction) |

#### AI Tags
| Tag | Purpose |
|-----|---------|
| `AI.Behavior.Patrol` | AI patrol behavior active |
| `AI.Behavior.Chase` | AI chase behavior active |
| `AI.Behavior.Attack` | AI attack behavior active |
| `AI.Behavior.Flee` | AI flee behavior active |
| `AI.Behavior.Search` | AI search behavior active |
| `AI.State.HasTarget` | AI has a valid target |
| `AI.State.LowHealth` | AI health is low |

#### Animation Tags
| Tag | Purpose |
|-----|---------|
| `Anim.HitReaction.Light` | Light hit reaction playing |
| `Anim.HitReaction.Medium` | Medium hit reaction playing |
| `Anim.HitReaction.Heavy` | Heavy hit reaction playing |

### Option B: DefaultGameplayTags.ini

Add this to `Config/DefaultGameplayTags.ini`:

```ini
[/Script/GameplayTags.GameplayTagsSettings]
+GameplayTagList=(Tag="Combat.CriticalHit",DevComment="")
+GameplayTagList=(Tag="Combat.DamageWindowActive",DevComment="")
+GameplayTagList=(Tag="Combat.Targetable",DevComment="")
+GameplayTagList=(Tag="Status.DoT",DevComment="")
+GameplayTagList=(Tag="Status.CC.Stun",DevComment="")
+GameplayTagList=(Tag="Status.CC.Slow",DevComment="")
+GameplayTagList=(Tag="Status.CC.Root",DevComment="")
+GameplayTagList=(Tag="SetByCaller.WeaponType",DevComment="")
+GameplayTagList=(Tag="SetByCaller.TargetLevel",DevComment="")
+GameplayTagList=(Tag="SetByCaller.StrengthScaling",DevComment="")
+GameplayTagList=(Tag="State.Dead",DevComment="")
+GameplayTagList=(Tag="State.Staggered",DevComment="")
+GameplayTagList=(Tag="AI.Behavior.Patrol",DevComment="")
+GameplayTagList=(Tag="AI.Behavior.Chase",DevComment="")
+GameplayTagList=(Tag="AI.Behavior.Attack",DevComment="")
+GameplayTagList=(Tag="AI.Behavior.Flee",DevComment="")
+GameplayTagList=(Tag="AI.Behavior.Search",DevComment="")
+GameplayTagList=(Tag="AI.State.HasTarget",DevComment="")
+GameplayTagList=(Tag="AI.State.LowHealth",DevComment="")
+GameplayTagList=(Tag="Anim.HitReaction.Light",DevComment="")
+GameplayTagList=(Tag="Anim.HitReaction.Medium",DevComment="")
+GameplayTagList=(Tag="Anim.HitReaction.Heavy",DevComment="")
```

---

## 3. Create Gameplay Effects

### 3a. GE_Damage (Core Damage Effect)

This is the gameplay effect used by every damage source. It feeds into `UOutlawDamageExecution`.

1. **Content Browser → Right-click → Blueprint Class → GameplayEffect** → Name: `GE_Damage`
2. Save in `Content/AbilitySystem/Effects/GE_Damage`

**Configure:**

| Property | Value |
|----------|-------|
| Duration Policy | `Instant` |
| Executions → Add Element | |
| → Calculation Class | `OutlawDamageExecution` |

**SetByCaller Magnitudes** — These are set at runtime by `UOutlawCombatLibrary::ApplyDamageToTarget`. No manual magnitude setup needed in the effect itself; the execution reads them via `SetByCaller` tags.

> **How damage flows**: Weapon fires → calls `ApplyDamageToTarget(SourceASC, TargetASC, GE_Damage, Level, SetByCallerMagnitudes)` → `GE_Damage` triggers `OutlawDamageExecution` → execution reads base damage from `IncomingDamage`, applies armor formula `Reduction = Armor / (Armor + 50 + 10*Level)`, applies crit/strength scaling → modifies target's `Health`.

### 3b. GE_DefaultAttributes (Enemy Defaults)

If `Content/NewFolder/GE_DefaultAttributes` already exists, verify it sets these attributes. Otherwise create it:

1. **Content Browser → Right-click → Blueprint Class → GameplayEffect** → Name: `GE_DefaultAttributes`
2. Save in `Content/AbilitySystem/Effects/GE_DefaultAttributes`

**Configure:**

| Property | Value |
|----------|-------|
| Duration Policy | `Instant` |
| Modifiers | (add one per attribute below) |

| Attribute | Modifier Op | Magnitude Type | Scalable Float Value |
|-----------|-------------|----------------|---------------------|
| `OutlawAttributeSet.Health` | `Override` | `Scalable Float` | `500.0` |
| `OutlawAttributeSet.MaxHealth` | `Override` | `Scalable Float` | `500.0` |
| `OutlawAttributeSet.Stamina` | `Override` | `Scalable Float` | `100.0` |
| `OutlawAttributeSet.MaxStamina` | `Override` | `Scalable Float` | `100.0` |
| `OutlawAttributeSet.Strength` | `Override` | `Scalable Float` | `20.0` |
| `OutlawAttributeSet.MaxStrength` | `Override` | `Scalable Float` | `100.0` |
| `OutlawAttributeSet.Armor` | `Override` | `Scalable Float` | `30.0` |
| `OutlawAttributeSet.MaxArmor` | `Override` | `Scalable Float` | `100.0` |

> **Tip**: Use the level-based `Scalable Float Magnitude` with a curve table if you want enemies to scale with level. For a first demo, flat values work fine.

### 3c. GE_PlayerDefaults (Player Defaults)

Same structure as `GE_DefaultAttributes`, but with player-appropriate values:

| Attribute | Value |
|-----------|-------|
| `Health` / `MaxHealth` | `1000.0` |
| `Stamina` / `MaxStamina` | `200.0` |
| `Strength` / `MaxStrength` | `50.0` |
| `Armor` / `MaxArmor` | `50.0` |

---

## 4. Set Up the Player Blueprint

Open `Content/Characters/BP_OutlawPlayerCharacter`.

### 4a. Add Components

In the Components panel, click **Add Component** and add each:

| Component | Class | Notes |
|-----------|-------|-------|
| Camera | `OutlawCameraComponent` | Replaces default camera. Provides OTS + Isometric modes, ADS, recoil, screen shake. |
| LockOn | `OutlawLockOnComponent` | Attach as child of root. Handles target lock cycling. |
| DeathComponent | `OutlawDeathComponent` | Core death state machine. |
| PlayerDeathHandler | `OutlawPlayerDeathHandler` | Checkpoint respawn, death screen. |
| CombatLog | `OutlawCombatLogComponent` | Optional, useful for debugging. |
| DamageNumbers | `OutlawDamageNumberComponent` | Floating damage text. |
| HitReaction | `OutlawHitReactionComponent` | Plays hit reaction montages. |
| StatusEffects | `OutlawStatusEffectComponent` | Tracks active status effects. |
| WeaponManager | `OutlawWeaponManagerComponent` | Should already exist. Verify it's present. |
| Inventory | `OutlawInventoryComponent` | Should already exist. Verify it's present. |
| Progression | `OutlawProgressionComponent` | Should already exist. Verify it's present. |

### 4b. Configure the Camera

Select the **OutlawCameraComponent**:

| Property | Value |
|----------|-------|
| OTS Config → Arm Length | `300.0` |
| OTS Config → Socket Offset | `(X=0, Y=60, Z=60)` |
| OTS Config → Field of View | `90.0` |
| OTS Config → Use Pawn Control Rotation | `true` |
| ADS Field of View | `60.0` |
| Recoil Recovery Speed | `5.0` |
| Camera Mode Blend Speed | `5.0` |

> **Note**: The camera defaults are already reasonable. Tweak `SocketOffset.Y` to taste — higher values push the camera further right for a more Outriders-like over-the-shoulder feel.

### 4c. Configure Lock-On

Select **OutlawLockOnComponent**:

| Property | Value |
|----------|-------|
| Lock On Range | `1500.0` |
| Lock On FOV | `45.0` |

### 4d. Configure Death Handling

Select **OutlawDeathComponent**:

| Property | Value |
|----------|-------|
| Allow Down State | `false` (for basic demo; enable for DBNO mechanics) |

Select **OutlawPlayerDeathHandler**:

| Property | Value |
|----------|-------|
| Respawn Delay | `3.0` |
| Death Screen Widget Class | Your death screen widget (see [Step 6](#6-create-a-death-screen-widget)) |

### 4e. Configure Damage Numbers

Select **OutlawDamageNumberComponent**:

| Property | Value |
|----------|-------|
| Damage Number Widget Class | Your damage number widget (see [Step 5](#5-create-a-damage-number-widget)) |
| Spawn Offset | `(X=0, Y=0, Z=80)` |

### 4f. Configure Hit Reactions

Select **OutlawHitReactionComponent**:

| Property | Value |
|----------|-------|
| Light Hit Threshold Percent | `10.0` (% of max health) |
| Medium Hit Threshold Percent | `30.0` |
| Can Be Staggered | `true` |
| Hit Reaction Montages | Add montage entries (see [Step 12](#12-set-up-animation-blueprints)) |

### 4g. Wire Up Events (Event Graph)

In the **Event Graph**, add the following bindings:

#### On Death Started
```
Event BeginPlay
  → DeathComponent → Bind Event to OnDeathStarted
    → Delegate: Custom Event "HandleDeathStarted"

HandleDeathStarted:
  → Disable Input
  → PlayerDeathHandler → StartDeath (is called automatically via the component)
```

#### On Loot Pickup (Optional auto-loot)
The `OutlawLootPickup` actor has `bAutoLoot = true` by default with `AutoLootRadius = 200`. Walking near loot auto-collects it. For manual pickup, set `bAutoLoot = false` and bind an input action to call `PickUp` on overlap.

#### On XP Gained
```
Progression Component → OnXPGained event
  → (Optional) Play UI feedback, level-up VFX
```

### 4h. Apply Default Attributes

In `Event BeginPlay` (or via the AbilitySystemComponent initialization):

```
Get Ability System Component
  → Apply Gameplay Effect to Self: GE_PlayerDefaults
```

Or configure the ASC's `DefaultStartingData` array to include `GE_PlayerDefaults`.

---

## 5. Create a Damage Number Widget

1. **Content Browser → Right-click → User Interface → Widget Blueprint**
2. Parent Class: `  `
3. Name: `WBP_DamageNumber`
4. Save in `Content/UI/WBP_DamageNumber`

**Design the widget:**

- Add a **Text Block** named `DamageText` — this displays the number.
- Add a **Float Up + Fade Out** animation:
  - Create a Widget Animation (name it `FadeAnim`).
  - Animate the `Render Transform → Translation Y` from `0` to `-80` over 1 second.
  - Animate `Render Opacity` from `1.0` to `0.0` over 1 second.
- In the Widget's **Event Graph**:
  - Add the event **Event On Damage Number Init** (this is a `BlueprintImplementableEvent` from the parent class — it appears as a red event node when you right-click → "Add Event" → search `OnDamageNumberInit`, or find it under the "Damage Number" category). It provides two pins: `Amount` (float) and `bIsCrit` (bool).
  - From `Amount` → **Format Text** or **To Text (Float)** → plug into `DamageText` → `SetText`.
  - **Branch** on `bIsCrit`:
    - **True**: Set `DamageText` → `SetColorAndOpacity` to **Yellow**, set `RenderTransform → Scale` to `(1.5, 1.5)`.
    - **False**: Set color to **White**, scale to `(1.0, 1.0)`.
  - **Play Animation** → select `FadeAnim`.
  - Bind **On Animation Finished (FadeAnim)** → `Remove From Parent`.

---

## 6. Create a Death Screen Widget

1. **Content Browser → Right-click → User Interface → Widget Blueprint**
2. Parent Class: `CommonUserWidget` (no custom death screen base class — the handler takes a plain `UUserWidget`)
3. Name: `WBP_DeathScreen`
4. Save in `Content/UI/WBP_DeathScreen`

**Design the widget:**

- Full-screen dark overlay (black, 0.6 opacity).
- Center text: **"YOU DIED"** (large, red).
- Subtitle: **"Respawning in 3..."** with a countdown.
- The `OutlawPlayerDeathHandler` automatically handles respawn after `RespawnDelay`. The widget is cosmetic.

Set this as the `DeathScreenWidgetClass` on the `OutlawPlayerDeathHandler` component (Step 4d).

---

## 7. Set Up the Enemy Blueprint

Open `Content/Characters/BP_OutlawEnemyCharacter`.

### 7a. Add Components

| Component | Class | Notes |
|-----------|-------|-------|
| DeathComponent | `OutlawDeathComponent` | Core death state. |
| EnemyDeathHandler | `OutlawEnemyDeathHandler` | Ragdoll → dissolve → loot drop → XP award. |
| HitReaction | `OutlawHitReactionComponent` | Hit flinch animations. |
| StatusEffects | `OutlawStatusEffectComponent` | Status effect tracking. |
| DamageNumbers | `OutlawDamageNumberComponent` | Shows damage dealt to this enemy. |

### 7b. Configure Enemy Death Handler

Select **OutlawEnemyDeathHandler**:

| Property | Value |
|----------|-------|
| Ragdoll Duration | `2.0` |
| Dissolve Duration | `1.5` |
| Death Impulse | `1000.0` |
| Base XP Reward | `50` |
| Loot Table | Your loot table asset (see [Step 10](#10-create-a-loot-table)) |
| Num Loot Drops | `2` |
| Rarity Bonus | `0.0` (increase for elite enemies) |

> **How enemy death works**: Health reaches 0 → `DeathComponent` fires `OnDeathStarted` → `EnemyDeathHandler` listens → enables ragdoll physics for `RagdollDuration` → starts dissolve material effect for `DissolveDuration` → calls `OutlawLootSubsystem::SpawnLoot` at death location → awards XP to killer → destroys actor.

### 7c. Configure Hit Reactions

Select **OutlawHitReactionComponent**:

| Property | Value |
|----------|-------|
| Light Hit Threshold Percent | `10.0` |
| Medium Hit Threshold Percent | `30.0` |
| Can Be Staggered | `true` |
| Hit Reaction Montages | Add entries (see [Step 12](#12-set-up-animation-blueprints)) |

### 7d. Verify AI Controller

The `AOutlawEnemyCharacter` C++ class already sets `AIControllerClass = AOutlawAIController::StaticClass()`. Verify in the Blueprint that:

| Property | Value |
|----------|-------|
| AI Controller Class | `OutlawAIController` (should be auto-set) |
| Auto Possess AI | `Placed in World or Spawned` |

### 7e. Configure the AI Controller Defaults

Create or open a Blueprint of `OutlawAIController` if you need to override defaults. Otherwise, the C++ defaults are:

| Property | Default |
|----------|---------|
| Sight Radius | `1500.0` |
| Lose Sight Radius | `2000.0` |
| Peripheral Vision Angle | `90.0°` |
| Hearing Range | `1000.0` |

To assign a **StateTree**, set the `StateTreeAsset` property on the `UStateTreeComponent` inside the AI controller (see [Step 11](#11-build-the-ai-statetree)).

### 7f. Apply Default Attributes

Same as player — in `Event BeginPlay`:

```
Get Ability System Component
  → Apply Gameplay Effect to Self: GE_DefaultAttributes
```

Or use the ASC `DefaultStartingData` property.

### 7g. Add Gameplay Tag: Targetable

In the enemy's ASC or via an initialization GE, grant the tag `Combat.Targetable` so the lock-on system can find this enemy.

---

## 8. Create Weapon Data Assets

### 8a. Shooter Weapon — Assault Rifle

1. **Content Browser → Right-click → Miscellaneous → Data Asset**
2. Pick class: `OutlawShooterWeaponData`
3. Name: `DA_AssaultRifle`
4. Save in `Content/Weapons/DA_AssaultRifle`

**Configure:**

| Property | Suggested Value |
|----------|-----------------|
| Weapon Name | `Assault Rifle` |
| Base Damage | `25.0` |
| Fire Rate | `10.0` (rounds/sec) |
| Magazine Size | `30` |
| Reload Time | `2.0` |
| Range | `5000.0` |
| Spread Angle | `2.0` |
| Weapon Mesh | Assign a skeletal or static mesh |

### 8b. Create an Item Definition for the Weapon

1. **Content Browser → Right-click → Miscellaneous → Data Asset**
2. Pick class: `OutlawItemDefinition`
3. Name: `ID_AssaultRifle`
4. Save in `Content/Items/ID_AssaultRifle`

**Configure:**

| Property | Value |
|----------|-------|
| Display Name | `Assault Rifle` |
| Item Type | `Weapon` |
| Rarity | `Common` |
| Max Stack Size | `1` |
| Weapon Data | `DA_AssaultRifle` |
| Icon | Assign a texture/icon |

### 8c. Give the Player a Starting Weapon

In `BP_OutlawPlayerCharacter` → `Event BeginPlay`:

```
WeaponManager → AddWeapon(ID_AssaultRifle)
WeaponManager → EquipWeapon(0)
```

---

## 9. Create Item Definitions for Loot

Create a few item definitions that enemies can drop. Minimum for the demo:

### Health Pack
| Property | Value |
|----------|-------|
| Name: `ID_HealthPack` | |
| Display Name | `Health Pack` |
| Item Type | `Consumable` |
| Rarity | `Common` |
| Max Stack Size | `10` |

### Ammo Pack
| Property | Value |
|----------|-------|
| Name: `ID_AmmoPack` | |
| Display Name | `Ammo Pack` |
| Item Type | `Consumable` |
| Rarity | `Common` |
| Max Stack Size | `50` |

### Rare Weapon Drop
| Property | Value |
|----------|-------|
| Name: `ID_RareRifle` | |
| Display Name | `Modified Assault Rifle` |
| Item Type | `Weapon` |
| Rarity | `Rare` |
| Max Stack Size | `1` |
| Weapon Data | Create a `DA_RareAssaultRifle` with higher base damage (e.g., `40.0`) |

---

## 10. Create a Loot Table

1. **Content Browser → Right-click → Miscellaneous → Data Asset**
2. Pick class: `OutlawLootTable`
3. Name: `LT_BasicEnemy`
4. Save in `Content/Loot/LT_BasicEnemy`

**Configure the `Entries` array:**

| Entry | Item Definition | Weight | Min Level | Max Level | Min Qty | Max Qty |
|-------|----------------|--------|-----------|-----------|---------|---------|
| 0 | `ID_HealthPack` | `50.0` | `1` | `99` | `1` | `3` |
| 1 | `ID_AmmoPack` | `40.0` | `1` | `99` | `5` | `20` |
| 2 | `ID_RareRifle` | `10.0` | `3` | `99` | `1` | `1` |

> **How loot works**: `EnemyDeathHandler` calls `OutlawLootSubsystem::SpawnLoot(DeathLocation, LootTable, EnemyLevel, RarityBonus, NumDrops)` → subsystem calls `LootTable::RollLoot` → rolls weighted random entries → spawns `OutlawLootPickup` actors scattered around the death location (`ScatterRadius = 150`).

Set `LT_BasicEnemy` as the **Loot Table** on each enemy's `OutlawEnemyDeathHandler` component (Step 7b).

---

## 11. Build the AI StateTree

The AI system uses UE5 **StateTree** for behavior. The C++ provides ready-made tasks and conditions.

### 11a. Create the StateTree Asset

1. **Content Browser → Right-click → AI → StateTree**
2. Schema: `StateTreeAIComponentSchema` 
3. Name: `ST_BasicEnemy`
4. Save in `Content/AI/ST_BasicEnemy`

### 11b. Available C++ Tasks

| Task Class | What It Does |
|------------|--------------|
| `OutlawSTTask_Patrol` | Moves between patrol points or wanders randomly |
| `OutlawSTTask_ChaseTarget` | Moves toward the current target actor |
| `OutlawSTTask_AttackTarget` | Performs an attack (plays montage, applies damage) |
| `OutlawSTTask_FleeFromTarget` | Moves away from threat |
| `OutlawSTTask_SearchForTarget` | Investigates last known position |

### 11c. Available C++ Conditions

| Condition Class | What It Checks |
|-----------------|----------------|
| `OutlawSTCondition_HasTarget` | True if AI controller has a valid target actor |
| `OutlawSTCondition_IsInRange` | True if target is within configurable range |
| `OutlawSTCondition_IsLowHealth` | True if health is below configurable threshold (default: 20%) |

### 11d. Recommended StateTree Structure

```
Root
├── State: Combat (Condition: HasTarget = true)
│   ├── State: Flee (Condition: IsLowHealth = true)
│   │   └── Task: FleeFromTarget
│   ├── State: Attack (Condition: IsInRange, Range = 800)
│   │   └── Task: AttackTarget
│   └── State: Chase (default/fallback)
│       └── Task: ChaseTarget
│
└── State: Idle (default — no target)
    └── Task: Patrol
```

**To build this in the StateTree editor:**

1. Open `ST_BasicEnemy`.
2. Add a **Root State** called `Root`.
3. Add child state `Combat` — add Enter Condition: `OutlawSTCondition_HasTarget` (Invert = false).
4. Inside `Combat`, add child state `Flee` — add Enter Condition: `OutlawSTCondition_IsLowHealth`.
5. Add `OutlawSTTask_FleeFromTarget` as the task for `Flee`.
6. Add sibling state `Attack` — add Enter Condition: `OutlawSTCondition_IsInRange` (set `Range = 800`).
7. Add `OutlawSTTask_AttackTarget` as the task.
8. Add sibling state `Chase` (no conditions — fallback).
9. Add `OutlawSTTask_ChaseTarget` as the task.
10. Back at root level, add sibling state `Idle` (no conditions — fallback when no target).
11. Add `OutlawSTTask_Patrol` as the task.

### 11e. Assign to AI Controller

Open the AI Controller Blueprint (or create one inheriting from `OutlawAIController`):
- Find the **StateTreeComponent** → set **StateTree Asset** = `ST_BasicEnemy`.

If using the C++ class directly (no Blueprint), set it in `DefaultSubobjectSetup` or via a property in the spawner.

---

## 12. Set Up Animation Blueprints

### 12a. Player Animation Blueprint

1. **Content Browser → Right-click → Animation → Animation Blueprint**
2. Parent Class: `OutlawAnimInstance`
3. Skeleton: Your player character's skeleton
4. Name: `ABP_Player`
5. Save in `Content/Characters/Animations/ABP_Player`

**In the AnimGraph:**

The `OutlawAnimInstance` exposes these properties (updated automatically):

| Property | Type | Source |
|----------|------|--------|
| `Speed` | float | Character movement velocity |
| `Direction` | float | Movement direction relative to facing |
| `bIsInAir` | bool | Jump/fall state |
| `AimPitch` | float | Look up/down |
| `AimYaw` | float | Look left/right |
| `bIsDead` | bool | Death state |
| `bIsStaggered` | bool | Stagger state |

Build a basic locomotion state machine:

```
Idle/Run (Blendspace using Speed + Direction)
  → Transition to JumpStart (bIsInAir = true)
  → Transition to Death (bIsDead = true)

JumpStart → JumpLoop → JumpEnd → Idle/Run
```

Use **Aim Offset** with `AimPitch` and `AimYaw` for upper body aiming.

### 12b. Enemy Animation Blueprint

Same structure as player, parent class `OutlawAnimInstance`.

Name: `ABP_Enemy`, save in `Content/Characters/Animations/ABP_Enemy`.

### 12c. Hit Reaction Montages

Create **Animation Montages** for hit reactions:

| Montage | Slot | Notes |
|---------|------|-------|
| `AM_HitReaction_Light_Front` | `DefaultSlot` | Small flinch forward |
| `AM_HitReaction_Medium_Front` | `DefaultSlot` | Bigger stagger |
| `AM_HitReaction_Heavy_Front` | `DefaultSlot` | Large knockback |

These can be simple 0.3–0.5 second additive animations. Assign them to the `HitReactionMontages` array on the `OutlawHitReactionComponent` (Steps 4f and 7c).

The `FOutlawHitReactionConfig` entries need:
| Property | Value |
|----------|-------|
| Reaction Type | `Light` / `Medium` / `Heavy` |
| Hit Direction | `Front` (or `Back`, `Left`, `Right`) |
| Montage | The corresponding montage |

### 12d. Attack Montages (for AI)

Create at least one attack montage for enemies:

| Montage | Notes |
|---------|-------|
| `AM_Enemy_Attack` | Melee swing or ranged fire animation |

Add an **Anim Notify** of type `OutlawAnimNotify_DamageWindow` at the frame where damage should apply. This sets the `Combat.DamageWindowActive` tag.

### 12e. Assign Animation Blueprints

- `BP_OutlawPlayerCharacter` → Mesh → Anim Class = `ABP_Player`
- `BP_OutlawEnemyCharacter` → Mesh → Anim Class = `ABP_Enemy`

---

## 13. Create the Projectile Blueprint

For hitscan weapons (the assault rifle), you don't need a projectile actor — `OutlawHitscanLibrary::FireHitscan` handles instant traces. But if you want visible bullet projectiles:

### 13a. Bullet Projectile (Optional Visual)

1. **Content Browser → Right-click → Blueprint Class → OutlawBulletProjectile**
2. Name: `BP_BulletProjectile`
3. Save in `Content/Projectiles/BP_BulletProjectile`

**Configure:**

| Property | Value |
|----------|-------|
| Speed | `10000.0` |
| Penetration Count | `0` (no penetration for basic demo) |
| Projectile Mesh / Particle | Add a small sphere mesh or trail particle |

### 13b. Spell Projectile (For Future Abilities)

1. **Content Browser → Right-click → Blueprint Class → OutlawSpellProjectile**
2. Name: `BP_SpellProjectile`

| Property | Value |
|----------|-------|
| Speed | `2000.0` |
| Splash Radius | `200.0` |
| Penetration Count | `0` |

### 13c. Pre-Warm the Projectile Pool

In your **Game Mode** or **Level Blueprint** `Event BeginPlay`:

```
Get OutlawProjectilePoolSubsystem
  → PreWarmPool(BP_BulletProjectile, 20)
```

This pre-spawns 20 bullet actors in the pool for instant reuse.

---

## 14. Place Enemy Spawners in the Level

### 14a. Create a Spawner Blueprint

1. **Content Browser → Right-click → Blueprint Class → OutlawEnemySpawner**
2. Name: `BP_EnemySpawner`
3. Save in `Content/AI/BP_EnemySpawner`

### 14b. Configure the Spawner

| Property | Value |
|----------|-------|
| Spawner Mode | `WaveBased` (for a structured demo) |
| Max Active Enemies | `10` |
| Spawn Radius | `500.0` |
| Enemy Class | `BP_OutlawEnemyCharacter` |

### 14c. Set Up Waves

Add entries to the **Waves** array (`TArray<FOutlawWaveSpawnData>`):

| Wave | Num Enemies | Delay Before Wave |
|------|-------------|-------------------|
| Wave 1 | `3` | `0.0` (immediate) |
| Wave 2 | `5` | `5.0` seconds |
| Wave 3 | `8` | `5.0` seconds |

### 14d. Place in Level

1. Open `Content/Maps/Demo`.
2. Drag `BP_EnemySpawner` into the level.
3. Position it in an open area where the player can engage.
4. Optionally place multiple spawners at different locations for variety.

### 14e. Trigger Spawning

In the **Level Blueprint** or via a trigger volume:

```
Event BeginPlay (or On Overlap with trigger)
  → Get reference to BP_EnemySpawner
  → Call StartSpawning()
```

For `Triggered` mode instead, call `TriggerSpawn(Count)` when the player enters an area.

---

## 15. Wire Up the Game Mode

Open `Content/Game/BP_OutlawGameMode`.

Verify these settings:

| Property | Value |
|----------|-------|
| Default Pawn Class | `BP_OutlawPlayerCharacter` |
| Player Controller Class | Default (or custom if you have one) |
| Player State Class | `BP_OutlawPlayerstate` |
| HUD Class | Default (or custom HUD) |

### 15a. Set Default Map

**Project Settings → Maps & Modes:**

| Property | Value |
|----------|-------|
| Default GameMode | `BP_OutlawGameMode` |
| Editor Startup Map | `Demo` |
| Game Default Map | `Demo` |

---

## 16. Assemble the Demo Map

Open `Content/Maps/Demo` and set up the level:

### 16a. Level Layout Checklist

- [ ] **Floor / terrain** — Flat plane or basic landscape
- [ ] **Walls / cover** — BSP or static mesh blocks for the player to use as cover
- [ ] **Player Start** — Place a `PlayerStart` actor
- [ ] **Lighting** — At minimum, a `DirectionalLight` + `SkyLight` + `SkyAtmosphere`
- [ ] **Nav Mesh** — Place a `NavMeshBoundsVolume` covering the playable area (required for AI movement)
- [ ] **Enemy Spawners** — 1–3 `BP_EnemySpawner` actors placed in combat areas
- [ ] **Checkpoint** (optional) — Call `PlayerDeathHandler->SetCheckpoint(Location, Rotation)` at key points

### 16b. Nav Mesh Setup (Critical for AI)

1. Place a `NavMeshBoundsVolume` actor.
2. Scale it to cover the entire playable area.
3. Press **P** to visualize the nav mesh (green = walkable).
4. Ensure all enemy spawner locations and combat areas are covered.

### 16c. Player Start

Place a `PlayerStart` actor at the desired spawn point. The player will spawn here (and respawn here unless a checkpoint is set).

---

## 17. Testing Checklist

Play the level in PIE (Play In Editor) and verify each system:

### Core Loop
- [ ] **Player spawns** with correct health (1000), stamina, attributes
- [ ] **Player can shoot** — weapon fires, hitscan traces work
- [ ] **Enemies spawn** from the spawner in waves
- [ ] **Enemies take damage** — damage numbers appear above their heads
- [ ] **Enemies have AI** — they detect the player, chase, and attack
- [ ] **Hit reactions play** — enemies flinch when shot
- [ ] **Enemies die** — ragdoll activates, then dissolve starts
- [ ] **Loot drops** — pickup actors spawn at enemy death location
- [ ] **Loot auto-collects** — walking near pickups adds items to inventory
- [ ] **XP is awarded** — check progression component after killing enemies
- [ ] **Player can level up** — kill enough enemies, XP reaches threshold

### Combat
- [ ] **Armor reduces damage** — enemies with higher armor take less damage
- [ ] **Critical hits** — damage numbers show crits (if crit logic is configured)
- [ ] **Combat log** — entries appear for each damage event

### Camera
- [ ] **OTS camera** — camera is positioned over the shoulder
- [ ] **ADS** — aiming zooms in (if bound to input)
- [ ] **Lock-on** — targeting enemies snaps aim (if bound to input)

### Death / Respawn
- [ ] **Player death** — health reaches 0, death screen shows, respawns after 3 seconds
- [ ] **Checkpoint** — if set, respawns at checkpoint location

### AI
- [ ] **Patrol** — enemies without a target wander/idle
- [ ] **Chase** — enemies move toward detected player
- [ ] **Attack** — enemies attack when in range
- [ ] **Flee** — low-health enemies retreat

### Performance
- [ ] **Projectile pooling** — no hitches during rapid fire (`stat game` to check)
- [ ] **Enemy count** — 10 concurrent enemies run smoothly
- [ ] **Loot pickups** — no issues with many pickups on screen

---

## 18. Troubleshooting

### Enemies don't move
- **Check Nav Mesh**: Press **P** in editor. Green overlay must cover enemy areas.
- **Check AI Controller**: Verify `AIControllerClass` is `OutlawAIController` on the enemy Blueprint.
- **Check StateTree**: Ensure the StateTree asset is assigned to the `StateTreeComponent`.

### No damage is dealt
- **Check GE_Damage**: Ensure `OutlawDamageExecution` is set as the execution class.
- **Check Gameplay Tags**: All `SetByCaller` tags must be registered.
- **Check ASC**: Both player and enemy must have Ability System Components with `OutlawAttributeSet`.

### Enemies don't die
- **Check attributes**: Enemy must have `Health` attribute initialized (via `GE_DefaultAttributes`).
- **Check DeathComponent**: Must be present on the enemy Blueprint.
- **Check IncomingDamage flow**: The damage execution writes to `Health` directly.

### No loot drops
- **Check LootTable**: Must be assigned on `OutlawEnemyDeathHandler`.
- **Check LootSubsystem**: `OutlawLootSubsystem` is a world subsystem — it initializes automatically. No setup needed.
- **Check Item Definitions**: The `TSoftObjectPtr` paths in loot table entries must be valid.

### Lock-on doesn't work
- **Check `Combat.Targetable` tag**: Enemies must have this tag granted.
- **Check range**: Player must be within `LockOnRange` (default 1500).

### Animations don't play
- **Check AnimBP assignment**: Mesh component must have the correct Anim Blueprint Class set.
- **Check parent class**: AnimBP must inherit from `OutlawAnimInstance`.
- **Check montage slots**: Hit reaction montages must use a slot that exists in your AnimBP.

### Crash on damage application
- **Check `ApplyDamageToTarget` parameters**: `SourceASC` and `TargetASC` must be non-null. `DamageEffectClass` must be set (typically `GE_Damage`).
- **Check SetByCaller magnitudes**: If the execution expects `SetByCaller.WeaponType` but it's not set, you'll get a warning. Ensure all magnitudes are passed in the TMap.

---

## Quick Reference: Key C++ Classes

| System | Class | Header |
|--------|-------|--------|
| Damage Pipeline | `UOutlawDamageExecution` | `Combat/OutlawDamageExecution.h` |
| Combat Library | `UOutlawCombatLibrary` | `Combat/OutlawCombatLibrary.h` |
| Death (Core) | `UOutlawDeathComponent` | `Combat/OutlawDeathComponent.h` |
| Death (Player) | `UOutlawPlayerDeathHandler` | `Combat/OutlawPlayerDeathHandler.h` |
| Death (Enemy) | `UOutlawEnemyDeathHandler` | `Combat/OutlawEnemyDeathHandler.h` |
| AI Controller | `AOutlawAIController` | `AI/OutlawAIController.h` |
| Enemy Spawner | `AOutlawEnemySpawner` | `AI/OutlawEnemySpawner.h` |
| Loot Table | `UOutlawLootTable` | `Loot/OutlawLootTable.h` |
| Loot Pickup | `AOutlawLootPickup` | `Loot/OutlawLootPickup.h` |
| Loot Subsystem | `UOutlawLootSubsystem` | `Loot/OutlawLootSubsystem.h` |
| Projectile Base | `AOutlawProjectileBase` | `Projectile/OutlawProjectileBase.h` |
| Hitscan | `UOutlawHitscanLibrary` | `Projectile/OutlawHitscanLibrary.h` |
| Pool Subsystem | `UOutlawProjectilePoolSubsystem` | `Projectile/OutlawProjectilePoolSubsystem.h` |
| Camera | `UOutlawCameraComponent` | `Camera/OutlawCameraComponent.h` |
| Lock-On | `UOutlawLockOnComponent` | `Camera/OutlawLockOnComponent.h` |
| Anim Instance | `UOutlawAnimInstance` | `Animation/OutlawAnimInstance.h` |
| Hit Reactions | `UOutlawHitReactionComponent` | `Animation/OutlawHitReactionComponent.h` |
| Status Effects | `UOutlawStatusEffectComponent` | `Combat/OutlawStatusEffectComponent.h` |
| Combat Log | `UOutlawCombatLogComponent` | `Combat/OutlawCombatLogComponent.h` |
| Damage Numbers | `UOutlawDamageNumberComponent` | `Combat/OutlawDamageNumberComponent.h` |

---

## What's NOT in This Demo (Tier 2+)

These features are on the roadmap but **not yet implemented**:

- **Abilities / Skills** — No GAS abilities, skill trees are framework-only
- **Multiplayer** — Single player only; authority checks exist but no replication
- **Status Effects** — Framework only; no burn/bleed/freeze/shock implementations
- **Save / Load** — No persistence between sessions
- **Inventory UI** — No visual inventory screen (items are in the component, no widget)
- **Skill Tree UI** — Framework exists, no visual interface
- **Polished VFX / SFX** — Placeholder only; no particle effects or sound
- **World Tiers / Difficulty Scaling** — `OutlawDifficultyConfig` exists but no UI or dynamic scaling

See `FEATURES.md` for the full tier roadmap.
