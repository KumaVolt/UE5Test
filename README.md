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
