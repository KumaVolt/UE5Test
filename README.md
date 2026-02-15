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
    OutlawWeaponAttributeSet.h/.cpp            Weapon stats as GAS attributes (shooter + ARPG)
  Characters/
    OutlawCharacterBase.h/.cpp                 Abstract base (IAbilitySystemInterface, default ability set)
    OutlawPlayerCharacter.h/.cpp               Player character (Enhanced Input, ASC from PlayerState)
    OutlawEnemyCharacter.h/.cpp                Enemy character (owns its own ASC)
  Inventory/
    OutlawItemDefinition.h/.cpp                Item data asset (name, rarity, type, grid size, weapon data)
    OutlawItemInstance.h/.cpp                  Per-item mutable state (ammo, affixes, gems, mods, quality)
    OutlawInventoryTypes.h                     Core structs (entry, list, equipment slot, save data)
    OutlawInventoryComponent.h/.cpp            Inventory component (flat + grid modes, equipment, GAS)
  Player/
    OutlawPlayerState.h/.cpp                   Owns ASC + AttributeSet for players
    OutlawPlayerController.h/.cpp              Creates and manages the HUD widget
  UI/
    OutlawHUDLayout.h/.cpp                     Root HUD widget (CommonActivatableWidget)
    OutlawStatBar.h/.cpp                       Reusable attribute-bound stat bar (CommonUserWidget)
  Progression/
    OutlawProgressionTypes.h                   Enums, structs, delegates for leveling & class system
    OutlawLevelingConfig.h/.cpp                Data-driven XP table (thresholds, skill points per level)
    OutlawSkillTreeNodeDefinition.h/.cpp       Skill tree node (manual/auto-unlock, multi-rank, prerequisites)
    OutlawClassDefinition.h/.cpp               Class definition (fixed or ascendancy, stat growth, skill tree)
    OutlawProgressionComponent.h/.cpp          XP, leveling, class selection, skill allocation, save/load
  Weapon/
    OutlawWeaponTypes.h                        Shared enums (weapon types, affix slot) & structs (affix, socket)
    OutlawShooterWeaponData.h/.cpp             Shooter weapon data asset (firepower, RPM, magazine)
    OutlawARPGWeaponData.h/.cpp                ARPG weapon data asset (damage range, sockets, affix pool)
    OutlawAffixDefinition.h/.cpp               Single affix definition (SetByCaller GE, value range, weight)
    OutlawAffixPoolDefinition.h/.cpp           Affix pool with weighted random rolling
    OutlawSkillGemDefinition.h/.cpp            Skill gem (socket compatibility, granted abilities)
    OutlawWeaponModDefinition.h/.cpp           Shooter weapon mod (tier slot, granted abilities)
    OutlawWeaponManagerComponent.h/.cpp        Active weapon management, cycling, swap, ammo
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

## Inventory System

### Architecture

The inventory system is a single `UOutlawInventoryComponent` that supports two modes:

| Mode | Config | Style | Use Case |
|---|---|---|---|
| **Flat slots** | `InventoryGridWidth = 0` | Destiny / Outriders | Fixed number of slots, items are 1x1 |
| **Spatial grid** | `InventoryGridWidth > 0, InventoryGridHeight > 0` | Path of Exile 2 | 2D grid, items have varying sizes |

Both modes share the same equipment, stacking, weight, GAS integration, sorting, and save/load systems.

### Item Definitions (`UOutlawItemDefinition`)

A `UPrimaryDataAsset` that defines everything about an item type:

| Property | Purpose |
|---|---|
| `DisplayName` / `Description` | UI text |
| `Icon` | `TSoftObjectPtr<UTexture2D>` for UI |
| `Rarity` | Common, Uncommon, Rare, Epic, Legendary |
| `ItemType` | Weapon, Shield, Helmet, Chest, Gloves, Boots, Ring, Amulet, Consumable, Material, Quest |
| `MaxStackSize` | 1 = non-stackable |
| `Weight` | Per-unit weight |
| `GridWidth` / `GridHeight` | Size in grid cells (grid mode only, default 1x1) |
| `ItemTags` | Gameplay tags for filtering |
| `bCanBeEquipped` / `EquipmentSlotTag` | Equipment slot mapping |
| `GrantedAbilitySet` | Ability set granted when equipped |
| `UseAbility` | Ability activated on "Use Item" |

### Blueprint Setup — Destiny / Outriders Style (Flat Slots)

**Step 1: Create Item Definitions**

1. In Content Browser, right-click > Miscellaneous > Data Asset > select `OutlawItemDefinition`
2. Name it (e.g. `DA_IronSword`, `DA_HealthPotion`, `DA_SteelBoots`)
3. Configure each item:
   - `DA_IronSword`: DisplayName = "Iron Sword", ItemType = Weapon, Rarity = Common, Weight = 3.0, MaxStackSize = 1, bCanBeEquipped = true, EquipmentSlotTag = `Equipment.Slot.MainHand`
   - `DA_HealthPotion`: DisplayName = "Health Potion", ItemType = Consumable, Rarity = Common, Weight = 0.5, MaxStackSize = 20, UseAbility = your potion ability class
   - `DA_SteelBoots`: DisplayName = "Steel Boots", ItemType = Boots, Rarity = Uncommon, Weight = 2.0, bCanBeEquipped = true, EquipmentSlotTag = `Equipment.Slot.Boots`

**Step 2: Add the Component to Your Character**

1. Open your player Blueprint (e.g. `BP_OutlawPlayerCharacter`)
2. Add Component > search `Outlaw Inventory Component`
3. In the Details panel, configure:
   - `MaxSlots` = 20 (or however many inventory slots you want)
   - `MaxWeight` = 100.0
   - `InventoryGridWidth` = 0 (leave at 0 for flat mode)
   - `EquipmentSlots` — click `+` to add entries:
     - Index 0: SlotTag = `Equipment.Slot.MainHand`
     - Index 1: SlotTag = `Equipment.Slot.OffHand`
     - Index 2: SlotTag = `Equipment.Slot.Helmet`
     - Index 3: SlotTag = `Equipment.Slot.Chest`
     - Index 4: SlotTag = `Equipment.Slot.Gloves`
     - Index 5: SlotTag = `Equipment.Slot.Boots`
     - Index 6: SlotTag = `Equipment.Slot.Ring`
     - Index 7: SlotTag = `Equipment.Slot.Amulet`

**Step 3: Add Items (Blueprint or C++)**

In any Blueprint (e.g. on pickup overlap):

```
Get Outlaw Inventory Component → Add Item (ItemDef = DA_IronSword, Count = 1)
```

The return value tells you how many were actually added (0 if inventory is full or overweight).

**Step 4: Build the Equipment Screen UI**

For each equipment slot in your UI:

```
1. Get Equipped Item (SlotTag = Equipment.Slot.Boots) → shows current boots (or null)
2. Find Items For Slot (SlotTag = Equipment.Slot.Boots) → returns all boots in inventory
3. When player selects a replacement → Equip Item (InstanceId from the selected entry)
   - The old item is auto-unequipped, ability sets swap automatically
```

**Step 5: Filtering and Sorting**

```
Find Items By Type (ItemType = Weapon)     → all weapons in inventory
Find Items By Rarity (Rarity = Legendary)  → all legendaries
Find Items By Tag (Tag = Item.Consumable)  → all consumables
Sort Inventory (SortMode = ByRarity, bDescending = true)  → Legendary first
```

**Step 6: Using Consumables**

```
Use Item (InstanceId) → activates the item's UseAbility, consumes 1 from stack
```

**Step 7: Listen for Changes (UI Refresh)**

In your inventory UI widget:
1. Get a reference to the inventory component
2. Bind to `On Inventory Changed` — refresh the entire inventory grid
3. Bind to `On Item Equipped` / `On Item Unequipped` — update equipment slot visuals
4. Bind to `On Item Used` — play consume VFX / feedback

**Step 8: Save/Load**

```
Save:  Save Data = Save Inventory()   → store this struct in your save game object
Load:  Load Inventory (Save Data)     → restores all items + re-equips equipped items
```

---

### Blueprint Setup — Path of Exile 2 Style (Spatial Grid)

**Step 1: Create Item Definitions with Grid Sizes**

Same as above, but also set the grid dimensions:

| Item | GridWidth | GridHeight | Visual |
|---|---|---|---|
| `DA_GreatSword` | 1 | 4 | Tall, narrow |
| `DA_ChestArmor` | 2 | 3 | Large rectangle |
| `DA_Shield` | 2 | 2 | Square |
| `DA_Boots` | 2 | 2 | Square |
| `DA_Ring` | 1 | 1 | Single cell |
| `DA_Amulet` | 1 | 1 | Single cell |
| `DA_HealthPotion` | 1 | 1 | Single cell, MaxStackSize = 10 |
| `DA_Helmet` | 2 | 2 | Square |

**Step 2: Add the Component with Grid Config**

1. Open your player Blueprint
2. Add Component > `Outlaw Inventory Component`
3. Configure:
   - `InventoryGridWidth` = 12
   - `InventoryGridHeight` = 5
   - `MaxSlots` is ignored in grid mode (total capacity is grid area)
   - `MaxWeight` = 100.0
   - `EquipmentSlots` — same as Destiny setup above

**Step 3: Adding Items**

Auto-placement (loot pickup, quest reward — no coordinates needed):

```
Add Item (ItemDef = DA_GreatSword, Count = 1)
  → internally calls FindFreeSpace to find the first open 1x4 rectangle
  → returns 1 if placed, 0 if no space
```

Manual placement (player drags from stash to a specific cell):

```
Can Place Item At (ItemDef = DA_ChestArmor, X = 3, Y = 0)  → true/false
Add Item At Position (ItemDef = DA_ChestArmor, X = 3, Y = 0, Count = 1)
```

**Step 4: Building the Grid UI**

Create a grid of `InventoryGridWidth` x `InventoryGridHeight` cells in your widget:

```
For each cell (X, Y):
  Get Item At Grid Position (X, Y) → returns the entry occupying that cell
    - If empty (InstanceId == -1): show empty cell
    - If occupied: show the item's icon spanning GridWidth x GridHeight cells
      (only render the icon on the entry's GridX, GridY — the top-left cell)
```

**Step 5: Drag-and-Drop**

When the player picks up an item and drops it on a new cell:

```
1. Can Place Item At Ignoring (ItemDef, NewX, NewY, InstanceId)
   → checks if the new rectangle is free, ignoring the item being moved
   → use this for the hover/preview highlight (green = valid, red = blocked)

2. Move Item (InstanceId, NewX, NewY)
   → moves the item, updates occupancy grid, replicates to clients
```

**Step 6: Everything Else Works the Same**

Equipment, use, sorting, filtering, save/load, GAS integration, and delegates all work identically in both modes. Grid positions are automatically saved/loaded.

---

### GAS Integration

When you equip an item that has a `GrantedAbilitySet`:
- The set's abilities, effects, and attribute sets are granted to the ASC (found via `IAbilitySystemInterface` on the owning actor or its PlayerState)
- When unequipped, everything is atomically revoked via `FOutlawAbilitySetGrantedHandles::RevokeFromASC()`

Example: equipping `DA_IronSword` with a `GrantedAbilitySet` containing a `GA_SwordSlash` ability makes that ability available. Unequipping removes it.

### Replication

- Inventory entries replicate via `FFastArraySerializer` (per-entry delta replication)
- Equipment slots replicate via standard `DOREPLIFETIME`
- All mutations are server-authoritative (`HasAuthority()` checks)
- The occupancy grid (grid mode) is not replicated — it's rebuilt on clients from the replicated entries

## Weapon System

The weapon system supports two game styles through a unified architecture:

| Style | Inspired By | Features |
|---|---|---|
| **Shooter** | Outriders | Ammo, RPM, magazine, reload, weapon mods, 3-slot cycling |
| **ARPG** | Path of Exile 2 | Skill gem sockets, weapon swap sets, random affix rolling, quality |

Both styles share the same inventory, equipment, and GAS infrastructure. A single item definition can have one or both weapon data assets attached (composition, not inheritance).

### Architecture

```
Layer 4: UOutlawWeaponManagerComponent   (active weapon, swap, cycle, GAS coordination)
Layer 3: Data Assets                      (ShooterWeaponData, ARPGWeaponData, Affixes, Gems, Mods)
Layer 2: UOutlawItemInstance              (per-item mutable state: ammo, affixes, gems, quality)
Layer 1: Existing Inventory + GAS         (UOutlawInventoryComponent, UOutlawAbilitySet, ASC)
```

### File Overview

```
Source/Outlaw/
  Weapon/
    OutlawWeaponTypes.h                  Shared enums & structs (weapon types, affix slot, socket slot)
    OutlawShooterWeaponData.h/.cpp       Shooter weapon data asset (firepower, RPM, magazine, abilities)
    OutlawARPGWeaponData.h/.cpp          ARPG weapon data asset (damage, sockets, affix pool)
    OutlawAffixDefinition.h/.cpp         Single affix definition (SetByCaller GE, value range, weight)
    OutlawAffixPoolDefinition.h/.cpp     Pool of affixes with weighted random rolling
    OutlawSkillGemDefinition.h/.cpp      Skill gem with socket compatibility + granted abilities
    OutlawWeaponModDefinition.h/.cpp     Shooter mod with tier slot + granted abilities
    OutlawWeaponManagerComponent.h/.cpp  Active weapon management, cycling, swap, ammo
  Inventory/
    OutlawItemInstance.h/.cpp            Per-item mutable state (ammo, affixes, gems, mods, quality)
  AbilitySystem/
    OutlawWeaponAttributeSet.h/.cpp      Weapon stats as GAS attributes (10 replicated attributes)
```

### Key Concepts

**Item Instance (`UOutlawItemInstance`)** — The inventory stores shared `UOutlawItemDefinition` assets, but weapons need per-item mutable state (ammo count, rolled affixes, socketed gems). When a weapon is added to the inventory, a `UOutlawItemInstance` UObject is automatically created and attached to the inventory entry. This instance holds all runtime state.

**Weapon Attribute Set (`UOutlawWeaponAttributeSet`)** — Weapon stats are exposed as GAS attributes so abilities and gameplay effects can read and modify them through standard GAS channels. When a weapon is activated, the weapon manager pushes the weapon's base stats into the attribute set. Buffs/debuffs modify these attributes like any other GAS attribute.

| Shooter Attributes | ARPG Attributes |
|---|---|
| Firepower | PhysicalDamageMin |
| RPM | PhysicalDamageMax |
| Accuracy | AttackSpeed |
| Stability | CriticalStrikeChance |
| CritMultiplier | |
| WeaponRange | |

**SetByCaller Affixes** — Each affix uses a generic `UGameplayEffect` with `SetByCaller` magnitude. The rolled value is passed into the GE via a gameplay tag, so no custom code is needed per affix type. One GE class + one data asset per affix type handles everything.

---

### Blueprint Setup — Outriders-Style Shooter

**Step 1: Create Shooter Weapon Data Assets**

1. Content Browser > right-click > Miscellaneous > Data Asset > select `OutlawShooterWeaponData`
2. Name it (e.g. `DA_ShooterData_AssaultRifle`)
3. Configure:
   - `WeaponType` = AssaultRifle
   - `Firepower` = 120, `RPM` = 650, `MagazineSize` = 30
   - `ReloadTime` = 2.0, `Range` = 3000, `Accuracy` = 80, `Stability` = 70
   - `CritMultiplier` = 1.5
   - `AmmoTypeTag` = `Ammo.Rifle`
   - `FireAbilitySet` = your fire ability set (containing GA_Fire, etc.)
   - `ReloadAbilitySet` = your reload ability set
   - `WeaponMesh` = your weapon skeletal mesh

**Step 2: Create Item Definitions with Weapon Data**

1. Create `DA_AssaultRifle` as an `OutlawItemDefinition`:
   - `DisplayName` = "Assault Rifle", `ItemType` = Weapon, `Rarity` = Rare
   - `bCanBeEquipped` = true, `EquipmentSlotTag` = `Equipment.Slot.Primary1`
   - `ShooterWeaponData` = `DA_ShooterData_AssaultRifle` (the asset from Step 1)
   - `GrantedAbilitySet` = a base ability set for the weapon (optional, for passive effects)

**Step 3: Create Weapon Mods (Optional)**

1. Content Browser > Data Asset > select `OutlawWeaponModDefinition`
2. Name it (e.g. `DA_Mod_Vampiric`)
3. Configure:
   - `DisplayName` = "Vampiric Rounds"
   - `AllowedTier` = 1 (Tier 1 only), or 0 for any tier
   - `GrantedAbilitySet` = set with abilities/effects granted while installed

**Step 4: Add Weapon Manager to Your Character**

1. Open your player Blueprint
2. Add Component > search `Outlaw Weapon Manager Component`
3. Configure:
   - `ShooterWeaponSlotOrder` — add 3 entries:
     - `Weapon.Slot.Primary1`
     - `Weapon.Slot.Primary2`
     - `Weapon.Slot.Sidearm`

4. Make sure your inventory component has matching equipment slots:
   - `Equipment.Slot.Primary1`
   - `Equipment.Slot.Primary2`
   - `Equipment.Slot.Sidearm`

**Step 5: Weapon Cycling (Blueprint)**

```
// Cycle to next weapon (Primary1 → Primary2 → Sidearm → Primary1)
Get Weapon Manager Component → Cycle Weapon

// Or switch directly
Get Weapon Manager Component → Switch To Weapon Slot (SlotTag = Weapon.Slot.Primary2)

// Get current weapon info
Get Weapon Manager Component → Get Active Weapon → returns UOutlawItemInstance*
  → CurrentAmmo (int32)
  → ItemDef → ShooterWeaponData → MagazineSize, RPM, etc.
```

**Step 6: Ammo Management (Blueprint)**

```
// Add ammo pickup
Get Weapon Manager Component → Add Reserve Ammo (AmmoTypeTag = Ammo.Rifle, Amount = 60)

// Read reserves (for UI)
Get Weapon Manager Component → Get Reserve Ammo (AmmoTypeTag = Ammo.Rifle) → int32

// Consume ammo (called by fire ability)
Get Weapon Manager Component → Consume Reserve Ammo (AmmoTypeTag, Amount = 1) → actual consumed

// Magazine ammo is on the item instance directly
Get Active Weapon → CurrentAmmo
```

**Step 7: Installing Mods (Blueprint)**

```
// Get the active weapon instance
Get Active Weapon → Instance

// Install a mod into Tier 1
Instance → Install Mod (ModDef = DA_Mod_Vampiric, Tier = 1, ASC = your ASC)
  → grants the mod's ability set automatically

// Remove a mod
Instance → Remove Mod (Tier = 1, ASC = your ASC)
  → revokes the mod's abilities
```

**Step 8: UI Delegates**

```
// Bind to weapon change events in your HUD
Get Weapon Manager Component → Bind Event to On Active Weapon Changed
  → fires when cycling/switching weapons, passes new UOutlawItemInstance*
  → update weapon icon, ammo display, mod slots
```

---

### Blueprint Setup — Path of Exile 2-Style ARPG

**Step 1: Create Affix Definitions**

1. Content Browser > Data Asset > select `OutlawAffixDefinition`
2. Create multiple affixes, e.g.:
   - `DA_Affix_FlatPhysDamage`: DisplayName = "Sharp", AffixSlot = Prefix, ValueMin = 5, ValueMax = 25, RequiredItemLevel = 1, Weight = 100, AffixGroupTag = `Affix.Group.FlatPhys`, AffixEffect = `GE_AffixFlatPhysDamage`, SetByCallerValueTag = `SetByCaller.FlatPhysDamage`
   - `DA_Affix_IncreasedAttackSpeed`: DisplayName = "of Speed", AffixSlot = Suffix, ValueMin = 5, ValueMax = 15, RequiredItemLevel = 10, Weight = 80

**Step 2: Create an Affix Pool**

1. Data Asset > select `OutlawAffixPoolDefinition`
2. Name it `DA_AffixPool_Swords`
3. Add all sword-eligible affixes to `PossibleAffixes`
4. Set `MaxPrefixes` = 3, `MaxSuffixes` = 3

**Step 3: Create Skill Gem Definitions**

1. Data Asset > select `OutlawSkillGemDefinition`
2. Create gems, e.g.:
   - `DA_Gem_Fireball`: DisplayName = "Fireball", bIsSupportGem = false, RequiredSocketTypeTag = `Socket.Red`, GrantedAbilitySet = set containing GA_Fireball
   - `DA_Gem_MultipleProjectiles`: DisplayName = "Multiple Projectiles Support", bIsSupportGem = true, RequiredSocketTypeTag = `Socket.Green`, GrantedAbilitySet = set with GE that modifies projectile count

**Step 4: Create ARPG Weapon Data Assets**

1. Data Asset > select `OutlawARPGWeaponData`
2. Name it `DA_ARPGData_Sword`:
   - `WeaponType` = Sword
   - `PhysicalDamageMin` = 10, `PhysicalDamageMax` = 20
   - `AttackSpeed` = 1.4, `CriticalStrikeChance` = 0.05
   - `MaxSocketCount` = 3
   - `DefaultSocketLayout` — add 3 entries:
     - Index 0: SocketTypeTag = `Socket.Red`, bLinkedToNext = true
     - Index 1: SocketTypeTag = `Socket.Green`, bLinkedToNext = true
     - Index 2: SocketTypeTag = `Socket.Blue`, bLinkedToNext = false
   - `bTwoHanded` = false
   - `AffixPool` = `DA_AffixPool_Swords`
   - `DefaultAttackAbilitySet` = your basic melee attack ability set

**Step 5: Create the Item Definition**

1. Create `DA_RuneSword` as an `OutlawItemDefinition`:
   - `ItemType` = Weapon, `Rarity` = Rare
   - `bCanBeEquipped` = true, `EquipmentSlotTag` = `Equipment.Slot.WeaponSetI.MainHand`
   - `ARPGWeaponData` = `DA_ARPGData_Sword`

**Step 6: Add Weapon Manager to Your Character**

1. Add `Outlaw Weapon Manager Component` to your player Blueprint
2. Configure ARPG weapon sets:
   - `ARPGWeaponSetI`:
     - `Equipment.Slot.WeaponSetI.MainHand`
     - `Equipment.Slot.WeaponSetI.OffHand`
   - `ARPGWeaponSetII`:
     - `Equipment.Slot.WeaponSetII.MainHand`
     - `Equipment.Slot.WeaponSetII.OffHand`

3. Add matching equipment slots to the inventory component.

**Step 7: Rolling Affixes (Blueprint)**

```
// After adding a weapon to inventory, roll affixes on it
Get Item Instance By Id (InstanceId) → Instance

// Roll affixes based on item level (higher = more affixes, better pool)
Instance → Roll Affixes (ItemLevel = 50)

// Read rolled affixes (for tooltip display)
Instance → Affixes (TArray<FOutlawItemAffix>)
  → for each: AffixDef → DisplayName, RolledValue, Slot (Prefix/Suffix)

// Apply affix effects to the player (do this when weapon is equipped)
Instance → Grant Affix Effects (ASC)

// Remove affix effects (when weapon is unequipped)
Instance → Revoke Affix Effects (ASC)
```

**Step 8: Socketing Gems (Blueprint)**

```
// Socket a gem into a specific socket
Instance → Socket Gem (GemDef = DA_Gem_Fireball, SocketIndex = 0)
  → returns true if compatible, false if wrong socket color

// Remove a gem
Instance → Unsocket Gem (SocketIndex = 0)
  → returns the removed gem definition

// Read socket layout (for UI)
Instance → SocketSlots (TArray<FOutlawSocketSlot>)
  → for each: SocketTypeTag, SocketedGem (or null), bLinkedToNext

// Gem abilities are automatically granted/revoked by the weapon manager
// when the weapon becomes active/inactive
```

**Step 9: Weapon Set Swap (Blueprint)**

```
// Swap between Set I and Set II (like pressing X in PoE 2)
Get Weapon Manager Component → Swap Weapon Set
  → revokes all gem abilities from old set
  → grants all gem abilities from new set
  → fires On Weapon Set Swapped delegate with new index (0 or 1)

// Read current set
Get Weapon Manager Component → Get Active Weapon Set Index → 0 or 1

// Get weapons in a specific set
Get Weapon Manager Component → Get Weapons In Set (SetIndex = 0) → TArray<UOutlawItemInstance*>
```

**Step 10: UI Delegates**

```
// Weapon swap
Bind Event to On Weapon Set Swapped → update ability bar to show new set's gem abilities

// Active weapon changed
Bind Event to On Active Weapon Changed → update weapon tooltip, socket display
```

---

### Blueprint Setup — Affix Gameplay Effects

Each affix needs a Gameplay Effect that uses `SetByCaller` for its magnitude:

1. Create a new Blueprint class inheriting from `GameplayEffect`
2. Name it `GE_AffixFlatPhysDamage`
3. Configure:
   - Duration Policy = Infinite
   - Modifiers: Add one modifier
     - Attribute = `OutlawWeaponAttributeSet.PhysicalDamageMin` (or whichever attribute this affix modifies)
     - Modifier Op = Additive
     - Modifier Magnitude > Magnitude Calculation Type = **Set By Caller**
     - Set By Caller Data Tag = `SetByCaller.FlatPhysDamage` (must match the affix definition's `SetByCallerValueTag`)

The item instance automatically passes the rolled value when calling `GrantAffixEffects()`.

---

### Weapon Attribute Set Integration

The weapon manager pushes base weapon stats into `UOutlawWeaponAttributeSet` when a weapon becomes active. Your abilities should read from these attributes rather than directly from the weapon data:

```
// In your fire ability (GA_Fire):
// Read RPM from the attribute set, not from the weapon data directly
Get Gameplay Attribute Value (Attribute = OutlawWeaponAttributeSet.RPM)
  → this value reflects the base stat + any buffs/debuffs from effects
```

To make the weapon attribute set available, include it in your character's default ability set:
1. Open `DA_DefaultAbilitySet`
2. In the `AttributeSets` array, add an entry with `OutlawWeaponAttributeSet`

This ensures the attribute set is granted when the character initializes.

---

### Save/Load

Weapon instance state (ammo, affixes, socketed gems, mods, quality) is automatically saved and restored by the inventory component's `SaveInventory()` / `LoadInventory()` — no extra Blueprint work needed. The save data includes:

- Current magazine ammo
- Quality value
- All rolled affixes (definition path + rolled value + prefix/suffix)
- All socketed gems per socket index
- Installed mod definitions (Tier 1 and Tier 2)

---

### Complete Example — Shooter Weapon Flow

```
1. Designer creates data assets:
   DA_ShooterData_AssaultRifle (UOutlawShooterWeaponData)
   DA_AssaultRifle (UOutlawItemDefinition, ShooterWeaponData = above)
   DA_Mod_Vampiric (UOutlawWeaponModDefinition)

2. Player picks up weapon:
   InventoryComponent → Add Item (DA_AssaultRifle)
   → ItemInstance auto-created, CurrentAmmo = MagazineSize (30)

3. Player equips weapon:
   InventoryComponent → Equip Item (InstanceId)
   → GrantedAbilitySet granted to ASC
   → WeaponManager notified → activates weapon
   → Fire/Reload ability sets granted
   → Weapon stats pushed to WeaponAttributeSet

4. Player fires:
   GA_Fire reads RPM from WeaponAttributeSet
   GA_Fire decrements Instance.CurrentAmmo
   GA_Fire consumes reserve ammo if magazine empty → triggers reload

5. Player installs mod:
   Instance → Install Mod (DA_Mod_Vampiric, Tier 1, ASC)
   → mod's ability set granted (e.g. life steal on hit)

6. Player cycles weapon:
   WeaponManager → Cycle Weapon
   → old weapon deactivated (abilities revoked, stats cleared)
   → new weapon activated (abilities granted, stats pushed)

7. Save game:
   InventoryComponent → Save Inventory
   → ammo, mods saved with weapon instance
```

---

### Complete Example — ARPG Weapon Flow

```
1. Designer creates data assets:
   DA_Affix_FlatPhys, DA_Affix_AttackSpeed, ... (UOutlawAffixDefinition)
   DA_AffixPool_Swords (UOutlawAffixPoolDefinition)
   DA_Gem_Fireball (UOutlawSkillGemDefinition)
   DA_ARPGData_Sword (UOutlawARPGWeaponData, AffixPool = above)
   DA_RuneSword (UOutlawItemDefinition, ARPGWeaponData = above)

2. Weapon drops:
   InventoryComponent → Add Item (DA_RuneSword)
   → ItemInstance auto-created, SocketSlots copied from DefaultSocketLayout

3. Roll affixes (at drop time or via crafting):
   Instance → Roll Affixes (ItemLevel = 50)
   → weighted random selection, group tag dedup
   → e.g. rolls: "Sharp" +15 phys, "of Speed" +8% attack speed

4. Socket gems:
   Instance → Socket Gem (DA_Gem_Fireball, SocketIndex = 0) → true

5. Equip to weapon set:
   InventoryComponent → Equip Item (InstanceId)
   → WeaponManager → OnWeaponEquipped
   → gem abilities granted (Fireball available on skill bar)
   → affix effects applied via GrantAffixEffects

6. Swap weapon set:
   WeaponManager → Swap Weapon Set
   → Set I gems revoked, Set II gems granted
   → skill bar updates via On Weapon Set Swapped delegate

7. Save game:
   → affixes (def path + rolled value), gems, quality all persisted
```

## Leveling & Class System

The progression system supports two class styles through a unified data-driven architecture:

| Style | Inspired By | Features |
|---|---|---|
| **Fixed Classes** | Outriders | Predefined class with skill tree + auto-unlock abilities per level |
| **Ascendancy Classes** | Path of Exile 2 | Base class → ascendancy specialization with node-based skill tree |

Both styles share the same GAS infrastructure, using `UOutlawAbilitySet` for granting abilities and `SetNumericAttributeBase()` for stat scaling.

### Architecture

```
Layer 3: UOutlawProgressionComponent     (XP, leveling, skill allocation, class selection, save/load)
Layer 2: Data Assets                      (LevelingConfig, ClassDefinition, SkillTreeNodeDefinition)
Layer 1: Existing GAS + Character         (ASC, UOutlawAttributeSet, UOutlawAbilitySet)
```

### Key Concepts

**Leveling Config (`UOutlawLevelingConfig`)** — A data asset that defines the XP table. Each entry maps to a level (index 0 = level 1) and specifies the total XP required and skill points awarded. Classes can override the default config.

**Class Definition (`UOutlawClassDefinition`)** — A data asset that defines a character class. Contains an `EOutlawClassMode` flag:
- `FixedClass` — Outriders-style: flat skill tree with auto-unlock nodes at specific levels
- `AscendancyClass` — PoE 2-style: base class with ascendancy sub-classes and prerequisite chains

Each class has a stat growth table (e.g. MaxHealth +15/level), a class ability set granted on selection, and a list of skill tree nodes.

**Skill Tree Node (`UOutlawSkillTreeNodeDefinition`)** — A data asset for a single allocable point on the skill tree. Two unlock types:
- `Manual` — player spends skill points, must meet prerequisites and level requirement
- `AutoOnLevel` — auto-granted when the character reaches a specific level (free, no points consumed)

Nodes support multi-rank allocation (e.g. +5% damage per rank, up to 5 ranks) with per-rank ability sets and cumulative stat bonuses.

**Progression Component (`UOutlawProgressionComponent`)** — The main component added to the character Blueprint. Manages XP, level-ups, class selection, ascendancy, skill tree allocation, and save/load. Accesses the ASC via `IAbilitySystemInterface` (same pattern as the weapon manager).

**Stat Growth** — Class stat scaling and node stat bonuses use `SetNumericAttributeBase()` on the existing `UOutlawAttributeSet` attributes. This means GE-based buffs/debuffs stack properly on top of the base values set by the progression system.

---

### Blueprint Setup — Outriders-Style (Fixed Classes)

**Step 1: Create a Leveling Config**

1. Content Browser > right-click > Miscellaneous > Data Asset > select `OutlawLevelingConfig`
2. Name it `DA_LevelingConfig_Standard`
3. Configure `LevelTable` — click `+` to add entries (one per level):
   - Index 0 (Level 1): RequiredXP = 0, SkillPointsAwarded = 0
   - Index 1 (Level 2): RequiredXP = 100, SkillPointsAwarded = 1
   - Index 2 (Level 3): RequiredXP = 300, SkillPointsAwarded = 1
   - Index 3 (Level 4): RequiredXP = 600, SkillPointsAwarded = 1
   - ...continue up to your max level (e.g. 50 entries)
4. Set `DefaultSkillPointsPerLevel` = 1 (used when an entry's SkillPointsAwarded is 0)

**Step 2: Create Skill Tree Nodes**

1. Data Asset > select `OutlawSkillTreeNodeDefinition`
2. Create manual nodes (player-chosen):
   - `DA_Node_HeavySlam`: NodeTag = `Skill.Devastator.HeavySlam`, UnlockType = Manual, MaxRank = 1, PointCostPerRank = 1, RequiredLevel = 1, GrantedAbilitySet = your slam ability set
   - `DA_Node_ArmorBoost`: NodeTag = `Skill.Devastator.ArmorBoost`, UnlockType = Manual, MaxRank = 5, PointCostPerRank = 1, RequiredLevel = 5, StatBonusesPerRank = [{Attribute = MaxHealth, ValuePerLevel = 10}]
3. Create auto-unlock nodes (granted at specific levels):
   - `DA_Node_GravityLeap`: NodeTag = `Skill.Devastator.GravityLeap`, UnlockType = AutoOnLevel, AutoUnlockLevel = 10, GrantedAbilitySet = your gravity leap set
4. Set up prerequisites (node B requires node A):
   - On `DA_Node_ArmorBoost`: Prerequisites = [{RequiredNodeTag = `Skill.Devastator.HeavySlam`, RequiredRank = 1}]

**Step 3: Create Class Definitions**

1. Data Asset > select `OutlawClassDefinition`
2. Create `DA_Class_Devastator`:
   - ClassTag = `Class.Devastator`, ClassMode = FixedClass, bIsBaseClass = true
   - StatGrowthTable: [{Attribute = MaxHealth, ValuePerLevel = 15}, {Attribute = Strength, ValuePerLevel = 3}]
   - ClassAbilitySet = a set with passive effects granted on class selection
   - SkillTreeNodes = [DA_Node_HeavySlam, DA_Node_ArmorBoost, DA_Node_GravityLeap]
   - LevelingConfig = DA_LevelingConfig_Standard (or leave null to use component default)
3. Create more classes (DA_Class_Trickster, DA_Class_Technomancer, etc.)

**Step 4: Add Progression Component to Your Character**

1. Open your player Blueprint (e.g. `BP_OutlawPlayerCharacter`)
2. Add Component > search `Outlaw Progression Component`
3. Configure:
   - `DefaultLevelingConfig` = DA_LevelingConfig_Standard
   - `AvailableClasses` = [DA_Class_Devastator, DA_Class_Trickster, ...]

**Step 5: Class Selection (Blueprint)**

```
// During character creation or class selection screen
Get Progression Component → Select Class (ClassTag = Class.Devastator)
  → grants class ability set
  → recalculates base stats (MaxHealth = 15 * level, Strength = 3 * level)
  → processes auto-unlock nodes for current level
  → fires On Class Changed delegate
```

**Step 6: Awarding XP (Blueprint)**

```
// On enemy kill, quest completion, etc.
Get Progression Component → Award XP (Amount = 50)
  → fires On XP Changed (NewXP, DeltaXP)
  → if enough XP: levels up, awards skill points, fires On Player Leveled Up
  → auto-unlock nodes trigger at their designated levels
  → base stats recalculated for new level
```

**Step 7: Skill Tree Allocation (Blueprint)**

```
// Player opens skill tree UI, clicks a node
Get Progression Component → Can Allocate Node (NodeTag = Skill.Devastator.ArmorBoost)
  → returns true if: enough points, level met, prerequisites met, not at max rank

Get Progression Component → Allocate Skill Node (NodeTag = Skill.Devastator.ArmorBoost)
  → deducts skill points, grants abilities/stat bonuses, fires On Skill Node Allocated

// Read current state for UI
Get Progression Component → Get Allocated Rank (NodeTag) → int32
Get Progression Component → Get Available Skill Points → int32
Get Progression Component → Get Allocable Nodes → TArray<FGameplayTag> (all currently eligible)
```

**Step 8: Deallocating and Respec (Blueprint)**

```
// Remove a single rank (only if no other nodes depend on this rank)
Get Progression Component → Deallocate Skill Node (NodeTag)
  → refunds points, revokes abilities, fires On Skill Node Deallocated

// Full respec — refund everything
Get Progression Component → Respec All Nodes
  → revokes all node abilities, refunds all manual skill points
  → auto-unlock nodes are re-applied automatically
```

**Step 9: UI Delegates**

```
// In your HUD / skill tree widget:
Bind Event to On Player Leveled Up      → show level-up VFX, update level display
Bind Event to On XP Changed             → update XP bar (use Get XP Progress for 0-1 float)
Bind Event to On Skill Node Allocated   → refresh skill tree node visual state
Bind Event to On Skill Node Deallocated → refresh skill tree node visual state
Bind Event to On Class Changed          → update class icon, rebuild skill tree UI
```

**Step 10: Save/Load**

```
Save:  Save Data = Get Progression Component → Save Progression
         → stores level, XP, skill points, class, ascendancy, all allocated nodes
         → store this struct in your save game object

Load:  Get Progression Component → Load Progression (Save Data)
         → restores level, class, ascendancy, re-allocates all nodes
         → recalculates stats, re-grants all abilities
```

---

### Blueprint Setup — Path of Exile 2-Style (Ascendancy Classes)

**Step 1: Create Base Class Definitions**

1. Data Asset > select `OutlawClassDefinition`
2. Create `DA_Class_Warrior`:
   - ClassTag = `Class.Warrior`, ClassMode = AscendancyClass, bIsBaseClass = true
   - StatGrowthTable: [{Attribute = MaxHealth, ValuePerLevel = 12}, {Attribute = Strength, ValuePerLevel = 4}]
   - ClassAbilitySet = base warrior passives
   - SkillTreeNodes = base warrior skill tree nodes (all Manual, with prerequisite chains)
   - AscendancyRequiredLevel = 30

**Step 2: Create Ascendancy Sub-Classes**

1. Create `DA_Ascendancy_Slayer`:
   - ClassTag = `Ascendancy.Slayer`, ClassMode = AscendancyClass, bIsBaseClass = false
   - StatGrowthTable: [{Attribute = Strength, ValuePerLevel = 2}] (stacks on top of base class)
   - ClassAbilitySet = slayer passives
   - SkillTreeNodes = slayer-specific nodes with prerequisites
2. Create `DA_Ascendancy_Berserker` similarly

**Step 3: Link Ascendancies to Base Class**

On `DA_Class_Warrior`:
- AvailableAscendancies = [DA_Ascendancy_Slayer, DA_Ascendancy_Berserker]
- AscendancyRequiredLevel = 30

**Step 4: Create Prerequisite Chains**

Skill tree nodes for ascendancy classes use prerequisite chains to form a tree:

```
DA_Node_BrutalFervour (root node, no prerequisites)
  └─ DA_Node_Headsman (requires BrutalFervour at rank 1)
       └─ DA_Node_OverwhelmingSalve (requires Headsman at rank 1)
```

On `DA_Node_Headsman`:
- Prerequisites = [{RequiredNodeTag = Skill.Slayer.BrutalFervour, RequiredRank = 1}]

**Step 5: Add Progression Component**

Same as the Outriders setup — add the component, set DefaultLevelingConfig and AvailableClasses.

**Step 6: Game Flow (Blueprint)**

```
// At game start — select base class
Get Progression Component → Select Class (ClassTag = Class.Warrior)

// Award XP through gameplay...
Get Progression Component → Award XP (Amount)

// When player reaches level 30 and completes the ascendancy trial:
Get Progression Component → Select Ascendancy (AscendancyTag = Ascendancy.Slayer)
  → validates level >= AscendancyRequiredLevel
  → grants ascendancy ability set
  → slayer skill tree nodes become available
  → fires On Ascendancy Selected delegate

// Allocate nodes from both base class tree AND ascendancy tree
Get Progression Component → Allocate Skill Node (NodeTag = Skill.Slayer.BrutalFervour)
```

**Step 7: Reading Class Info (Blueprint)**

```
// Get the active class (returns ascendancy if selected, else base class)
Get Progression Component → Get Active Class Definition → DisplayName, ClassIcon, etc.

// Get specific class info
Get Progression Component → Get Selected Class       → base class definition
Get Progression Component → Get Selected Ascendancy   → ascendancy definition (or null)
```

---

### Replication

- All progression state replicates via standard `DOREPLIFETIME` (level, XP, skill points, class tags, allocated nodes)
- All mutations are server-authoritative (`HasAuthority()` checks)
- Ability grant/revoke handles are server-only (clients see the effects through standard GAS replication)

---

### Complete Example — Fixed Class Flow

```
1. Designer creates data assets:
   DA_LevelingConfig_Standard (50 levels, increasing XP thresholds)
   DA_Node_HeavySlam, DA_Node_ArmorBoost, DA_Node_GravityLeap (skill nodes)
   DA_Class_Devastator (ClassMode = FixedClass, nodes = above)

2. Player creates character:
   ProgressionComponent → SelectClass(Class.Devastator)
   → class passives granted, base stats set for level 1

3. Player kills enemies, gains XP:
   ProgressionComponent → AwardXP(50)
   → accumulates XP, eventually hits level thresholds
   → level 2: +1 skill point, MaxHealth base = 30, Strength base = 6
   → level 10: GravityLeap auto-unlocked (AutoOnLevel node)

4. Player opens skill tree, allocates:
   AllocateSkillNode(Skill.Devastator.HeavySlam) → rank 1, slam ability granted
   AllocateSkillNode(Skill.Devastator.ArmorBoost) → rank 1, +10 MaxHealth
   AllocateSkillNode(Skill.Devastator.ArmorBoost) → rank 2, +20 MaxHealth total

5. Player respecs:
   RespecAllNodes → all manual node abilities revoked, points refunded
   GravityLeap stays (auto-unlock, re-applied automatically)

6. Save game:
   SaveProgression → stores level 10, XP, 8 skill points, Class.Devastator,
                      [HeavySlam rank 1, ArmorBoost rank 2, GravityLeap rank 1]
```

---

### Complete Example — Ascendancy Flow

```
1. Designer creates data assets:
   DA_Class_Warrior (base class, AscendancyRequiredLevel = 30)
   DA_Ascendancy_Slayer (bIsBaseClass = false, skill tree with prerequisites)
   Link: Warrior.AvailableAscendancies = [Slayer, Berserker]

2. Player starts as Warrior:
   SelectClass(Class.Warrior) → base warrior passives + stats

3. Player levels to 30, completes ascendancy trial:
   SelectAscendancy(Ascendancy.Slayer)
   → slayer passives granted
   → slayer skill tree nodes now allocable
   → stat growth stacks: base MaxHealth +12/level + slayer Strength +2/level

4. Player allocates nodes from BOTH trees:
   AllocateSkillNode(Skill.Warrior.IronSkin)        → base class node
   AllocateSkillNode(Skill.Slayer.BrutalFervour)    → ascendancy node
   AllocateSkillNode(Skill.Slayer.Headsman)          → requires BrutalFervour rank 1 ✓

5. Save/load preserves everything:
   class, ascendancy, all allocated nodes from both trees
```

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

**Public**: Core, CoreUObject, Engine, InputCore, EnhancedInput, GameplayAbilities, GameplayTags, GameplayTasks, UMG, CommonUI, CommonInput, NetCore

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
7. **Dual-mode inventory** — A single component supports both flat slots (Destiny/Outriders) and spatial grid (Path of Exile 2). Mode is selected via config, not code changes. All features (equipment, GAS, save/load, replication) work in both modes.
8. **Server-authoritative inventory** — All mutations check `HasAuthority()`. Clients receive updates via `FFastArraySerializer` delta replication.
9. **Composition-based weapons** — `UOutlawItemDefinition` gets optional `ShooterWeaponData` / `ARPGWeaponData` pointers rather than subclasses. Keeps a single item type flowing through the inventory system.
10. **UObject item instances** — `UOutlawItemInstance` enables per-item mutable state (ammo, affixes, gems) while the shared `UOutlawItemDefinition` remains immutable and reusable.
11. **Weapon attribute set** — Abilities read weapon stats from GAS attributes, not from weapon data directly. This lets buffs/debuffs modify weapon stats through standard GAS effects.
12. **SetByCaller affix effects** — Each affix uses a GE with SetByCaller magnitude, passing the rolled value via tag. No custom code per affix type — one GE class per stat modification.
13. **Self-contained progression system** — The `UOutlawProgressionComponent` requires zero modifications to existing files. It accesses the ASC via `IAbilitySystemInterface` (same helper pattern as the weapon manager).
14. **Dual-mode class system** — `EOutlawClassMode` on `UOutlawClassDefinition` determines behavior. FixedClass = Outriders-style (flat skill tree with auto-unlocks). AscendancyClass = PoE 2-style (base class → specialization with node prerequisites).
15. **SetNumericAttributeBase for stat growth** — Class stat scaling and node stat bonuses use `SetNumericAttributeBase()` on existing attributes. GE-based buffs/debuffs stack properly on top of the progression-set base values.
16. **Multi-rank skill nodes** — Supports both single-point nodes (grant one ability set) and multi-rank nodes (e.g. +5% damage per rank, up to 5 ranks) via `PerRankAbilitySets` and `StatBonusesPerRank`.
