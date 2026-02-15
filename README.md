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
  Inventory/
    OutlawItemDefinition.h/.cpp                Item data asset (name, rarity, type, grid size, abilities)
    OutlawInventoryTypes.h                     Core structs (entry, list, equipment slot, save data)
    OutlawInventoryComponent.h/.cpp            Inventory component (flat + grid modes, equipment, GAS)
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
7. **Dual-mode inventory** — A single component supports both flat slots (Destiny/Outriders) and spatial grid (Path of Exile 2). Mode is selected via config, not code changes. All features (equipment, GAS, save/load, replication) work in both modes.
8. **Server-authoritative inventory** — All mutations check `HasAuthority()`. Clients receive updates via `FFastArraySerializer` delta replication.
