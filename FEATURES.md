# Outlaw — Feature Roadmap

## Implemented

### Gameplay Ability System
- [x] ASC on PlayerState (Mixed replication, survives pawn respawn)
- [x] Data-driven ability sets (UOutlawAbilitySet — grant/revoke atomically)
- [x] Input tag routing (abilities found by gameplay tags, not hardcoded bindings)
- [x] Base ability class with activation policies (OnInputTriggered, OnGranted, OnGameplayEvent)
- [x] Core attribute set (Health, MaxHealth, Stamina, MaxStamina, Strength, MaxStrength)
- [x] Weapon attribute set (Firepower, RPM, Accuracy, Stability, CritMultiplier, WeaponRange, PhysicalDamageMin/Max, AttackSpeed, CriticalStrikeChance)

### CommonUI HUD
- [x] Root HUD layout (CommonActivatableWidget, auto-activate, never eats game input)
- [x] Reusable stat bar widget (binds to any GAS attribute pair, delegate-driven updates)
- [x] Controller creates HUD in BeginPlayingState (guarantees valid ASC)

### Inventory System
- [x] Dual-mode: flat slots (Outriders/Destiny) and spatial grid (PoE 2)
- [x] Equipment slots with GAS integration (equip grants abilities, unequip revokes)
- [x] Stacking, weight limits, sorting, filtering by type/rarity/tag
- [x] FFastArraySerializer replication (per-entry delta)
- [x] Save/load (items, equipment, grid positions)

### Weapon System
- [x] Shooter mode: 3-slot cycling, ammo reserves, magazine, fire/reload ability sets, weapon mods
- [x] ARPG mode: weapon set swap (Set I / Set II), skill gem sockets, socket compatibility
- [x] Random affix rolling (weighted, group tag dedup, SetByCaller GE magnitude)
- [x] Weapon stats pushed to GAS attributes (buffs/debuffs stack on top)
- [x] Per-item mutable state via UOutlawItemInstance (ammo, affixes, gems, mods, quality)

### Leveling & Class System
- [x] Data-driven XP table (configurable per class)
- [x] Fixed classes (Outriders-style: skill tree + auto-unlock abilities per level)
- [x] Ascendancy classes (PoE 2-style: base class -> specialization with node prerequisites)
- [x] Multi-rank skill nodes with per-rank ability sets and stat bonuses
- [x] Auto-unlock nodes (granted at specific levels, free)
- [x] Manual nodes (skill point cost, prerequisite validation)
- [x] Respec (full refund, auto-unlock nodes re-applied)
- [x] Class stat growth via SetNumericAttributeBase (GE buffs stack on top)
- [x] Save/load (level, XP, class, ascendancy, all allocated nodes)

### Characters
- [x] Player character (Enhanced Input, ASC from PlayerState, default ability set)
- [x] Enemy character (owns its own ASC, Minimal replication)
- [x] PlayerState (owns ASC + AttributeSet)
- [x] PlayerController (creates and manages HUD)

---

## Tier 1 — Core Gameplay (can't play without these)

### Combat / Damage Pipeline
- [ ] Damage calculation gameplay effect (weapon attributes + attacker stats vs defender armor)
- [ ] Hit detection (line traces for hitscan, overlap for melee/AoE)
- [ ] Critical hit resolution
- [ ] Floating damage numbers
- [ ] Status effects (burn, bleed, freeze, shock) as gameplay effects
- [ ] Damage resistance / armor system in attribute set
- [ ] Friendly fire rules

### AI System
- [ ] Enemy behavior trees (aggro, patrol, chase, attack, flee, search)
- [ ] AI perception (sight, hearing, damage)
- [ ] Enemy ability usage through GAS (enemies use same ability system as player)
- [ ] Spawner actor (wave-based, triggered, ambient)
- [ ] Spawn budgeting (max active enemies, performance scaling)
- [ ] Enemy difficulty scaling (level, stat multipliers)

### Death & Respawn
- [ ] Player death flow (down state or instant death, respawn at checkpoint)
- [ ] Enemy death (ragdoll, dissolve, loot drop trigger)
- [ ] XP-on-kill hookup to progression component
- [ ] Kill feed / combat log

### Loot / Drop System
- [ ] Loot table data asset (per enemy type, per chest tier, per boss)
- [ ] Rarity weighting and item level scaling
- [ ] Auto-roll affixes on drop based on item level
- [ ] Loot pickup actor (world mesh, interact to collect)
- [ ] Auto-loot radius option
- [ ] Loot beam / glow by rarity

### Projectile System
- [ ] Bullet projectile actor (travel speed, gravity, impact)
- [ ] Spell projectile actor (homing, AoE splash on impact)
- [ ] Projectile penetration and chaining
- [ ] Projectile pooling (object pool for performance)
- [ ] Hitscan alternative for high-RPM weapons

### Animation
- [ ] Player AnimBP (locomotion blend space, aim offset)
- [ ] Ability montages (attack, cast, reload, dodge)
- [ ] Anim notifies for damage windows and VFX triggers
- [ ] Weapon equip / holster animations
- [ ] Enemy hit reactions (stagger, knockback, knockdown)
- [ ] Death animations / ragdoll blend

### Camera
- [ ] Third-person over-shoulder camera (Outriders-style)
- [ ] Isometric camera option (PoE 2-style)
- [ ] Aim mode (ADS zoom, sensitivity change)
- [ ] Recoil and screen shake
- [ ] Camera collision avoidance
- [ ] Lock-on targeting (optional, for ARPG melee)

---

## Tier 2 — Depth & Polish (playable but shallow without these)

### Unified Save System
- [ ] USaveGame subclass orchestrating all per-system save data
- [ ] Save slots (multiple characters)
- [ ] Auto-save on checkpoints / zone transitions
- [ ] Async save/load (non-blocking)
- [ ] Save file versioning and migration

### Quest / Objective System
- [ ] Quest definition data asset (objectives, rewards, prerequisites)
- [ ] Objective types: kill count, collect item, reach location, interact with NPC
- [ ] Quest state machine (available, active, completed, failed)
- [ ] Quest log UI
- [ ] Quest waypoint markers on HUD / minimap
- [ ] NPC dialogue triggers
- [ ] Quest rewards (XP, items, currency, class unlock)

### Minimap / World Map
- [ ] Minimap widget (player position, rotation)
- [ ] Enemy markers (with range fade)
- [ ] Quest waypoints and objective markers
- [ ] Points of interest icons
- [ ] Full-screen world map with zoom and pan
- [ ] Fog of war / area discovery

### Full UI Suite
- [ ] Inventory screen (grid rendering, drag-drop, item tooltips with stat comparison)
- [ ] Skill tree visual graph (node connections, allocated/available/locked states)
- [ ] Character sheet (all stats breakdown, equipped gear summary)
- [ ] Class selection screen (class preview, stat comparison)
- [ ] Settings menu (graphics, audio, controls, keybinding)
- [ ] Pause menu
- [ ] Dialogue UI (NPC conversation, branching choices)
- [ ] Confirmation dialogs (respec, discard item, quit)
- [ ] Loading screen with tips

### Interaction System
- [ ] Interact component (doors, chests, NPCs, shrines, crafting stations)
- [ ] Interaction prompt widget (context-sensitive icon + text)
- [ ] Priority handling (closest interactable, prefer quest NPCs)
- [ ] Range check with configurable radius

### Audio
- [ ] SFX: abilities, weapons, impacts, footsteps, UI clicks
- [ ] Ambient soundscapes per area (wind, water, dungeon echoes)
- [ ] Music system (exploration, combat, boss, menu)
- [ ] Audio mix snapshots (combat intensity, paused, cutscene)
- [ ] 3D spatial audio for enemy footsteps and off-screen cues

### VFX
- [ ] Niagara systems: muzzle flash, bullet trails, impacts by surface type
- [ ] Ability VFX (cast, projectile, area effect, buff/debuff auras)
- [ ] Status effect visuals (fire overlay, frost crystals, bleed drip)
- [ ] Level-up burst effect
- [ ] Loot rarity beam / glow
- [ ] Environmental VFX (torches, fog, dust motes, water caustics)

### Crafting
- [ ] Crafting bench interaction
- [ ] Currency items (orbs, essences — PoE 2-style) or material cost (Outriders-style)
- [ ] Affix re-rolling, adding, removing, locking
- [ ] Mod transfer between weapons (Outriders-style)
- [ ] Upgrade tiers / item level scaling
- [ ] Crafting recipe discovery

---

## Tier 3 — Shippable Game (polish, endgame, platform)

### Multiplayer Session Management
- [ ] Lobby system (create, join, invite)
- [ ] Matchmaking (level range, region)
- [ ] Party system (2-4 players)
- [ ] Host migration or dedicated server support
- [ ] Latency compensation (client-side prediction for movement)
- [ ] Session browser with filters

### Endgame Loop
- [ ] Repeatable endgame content (Expeditions / Maps / Rifts)
- [ ] Difficulty tiers with scaling rewards
- [ ] Timer-based reward tiers (Outriders Expeditions)
- [ ] Atlas / map device progression (PoE 2-style)
- [ ] Endgame boss encounters
- [ ] Leaderboards (optional)

### Enemy Variety
- [ ] Melee rusher archetype
- [ ] Ranged attacker archetype
- [ ] Tank / shielder archetype
- [ ] Healer / support archetype
- [ ] Elite / champion enemies (affixes: extra fast, extra life, reflect)
- [ ] Boss encounters (multi-phase, unique mechanics)
- [ ] Distinct visual silhouettes per archetype

### World / Level Design
- [ ] Hub area (town, camp — safe zone with NPCs, stash, crafting)
- [ ] Combat zones (linear missions or open areas)
- [ ] Level streaming and area transitions
- [ ] Loading screens
- [ ] Waypoint / fast travel system
- [ ] Environmental hazards (traps, fire, poison pools)
- [ ] Destructible objects (barrels, crates — potential loot)

### Tutorials / Onboarding
- [ ] First-time-user experience (guided first mission)
- [ ] Contextual tooltip system
- [ ] Control hints (adapt to input device)
- [ ] Gradual mechanic introduction (abilities, then skill tree, then crafting)

### Localization
- [ ] String tables for all user-facing text
- [ ] Localization pipeline (export/import .po files)
- [ ] Right-to-left text support (if targeting Arabic/Hebrew)
- [ ] Localized audio (subtitles at minimum)

### Analytics / Telemetry
- [ ] Play session tracking (time played, deaths, kills)
- [ ] Crash reporting integration
- [ ] Balance data collection (popular classes, skill allocations, item drop rates)
- [ ] Funnel analysis (tutorial completion, endgame reach)

### Platform Requirements
- [ ] Console certification (Xbox GDK, PlayStation SDK)
- [ ] Platform achievements / trophies
- [ ] Cloud saves (Steam Cloud, console cloud)
- [ ] First-party API integration (friends, rich presence)
- [ ] Cross-platform play (optional)

### Anti-cheat / Security
- [ ] Server-side validation (XP amount sanity, item generation bounds)
- [ ] Rate limiting on RPCs (prevent ability spam exploits)
- [ ] Checksums on save data
- [ ] Anti-cheat middleware (EasyAntiCheat / BattlEye, if competitive)

---

## Suggested Build Order

The recommended implementation order to close gameplay loops as fast as possible:

```
1. Combat damage pipeline + hit detection    → things can take damage
2. AI + enemy spawning                       → something to fight
3. Death/respawn + XP-on-kill                → core loop closes (fight → kill → level)
4. Loot drops                                → loot loop closes (kill → drop → equip → stronger)
5. Projectile system                         → shooter mode functional
6. Animation + VFX                           → feels like a game
7. Unified save game                         → can playtest progression across sessions
8. Interaction system                        → chests, NPCs, doors
9. Full UI screens                           → inventory, skill tree, character sheet
10. Quest system                             → directed player experience
11. Crafting                                 → item depth
12. Endgame loop                             → retention
13. Multiplayer                              → co-op play
14. Polish (audio, tutorials, settings)      → ship it
```
