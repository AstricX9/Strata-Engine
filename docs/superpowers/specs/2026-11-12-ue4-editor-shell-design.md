# UE4 Editor Shell — Design

Date: 2026-11-12 · Status: approved in conversation, awaiting spec review

## Goal

Make the Strata (Godot 4.7.3-rc fork) editor look and be used like the Unreal Engine 4.26
level editor: same panel arrangement, toolbar, theme feel, panel names and viewport controls.
Reference: the user's UE 4.26 screenshots (Minimal_Default level; Place Actors, Landscape mode,
World Settings). Screenshots are reference only and are **not** committed (Epic's UI).

## Decisions

| Topic | Decision |
|---|---|
| Scope | Full UE4 shell: layout, theme, names, toolbar, Place Actors, Outliner Type column, Details Add Component bar, Content Browser tiles, viewport bar, UE4 navigation |
| Godot look | Replaced entirely. Classic/Modern styles and Godot's default layout are removed |
| Main-screen tabs | Replaced by one UE-style document tab bar |
| Wording | Panel and menu names only. Godot concepts (scene, node) keep their names until the gameplay-framework phase |
| Viewport controls | New "Unreal" navigation scheme, default |
| Code structure | New components in `editor/strata/`; Godot files get only plug-in edits |
| Art | Original. Colours/feel inspired by UE4; no copied icons or assets |
| Out of scope | Project Manager window, full icon restyle, Outliner folders, Blueprints/anim graphs, Scene→Level / Node→Actor renaming |

## Architecture

All new code lives in `editor/strata/`, one `.cpp/.h` pair per component, registered in an
`SCsub` included from `editor/SCsub`.

| File | Responsibility | Plugged in from |
|---|---|---|
| `strata_theme` | Builds the editor theme (colours, styleboxes, fonts sizes, constants) | `themes/editor_theme_manager.cpp` (replaces `ThemeClassic`/`ThemeModern` calls) |
| `strata_layout` | Default dock layout `ConfigFile` and UE4 dock titles | `editor_node.cpp` default-layout function (~line 9202) |
| `strata_toolbar` | Large icon toolbar row under the menu bar | `editor_node.cpp` UI construction (title bar area, ~line 8679) |
| `strata_document_tabs` | Single tab bar: open scenes + tool screens | `editor_node.cpp`, `editor_main_screen`, `scene/editor_scene_tabs` |
| `place_actors_dock` | Place Actors dock + item table | registered as an `EditorDock` from `editor_node.cpp` |
| `content_browser_view` | Content Browser top bar, tile view, footer | `docks/filesystem_dock.cpp` |

Godot's removed pieces (theme styles, `interface/theme/style` setting, main-screen buttons,
run bar, scene tab strip) are deleted or bypassed at the plug-in point, not left half-alive.

## 1. Default layout

| Area | Contents |
|---|---|
| Left | Place Actors dock; mode panels from plugins (e.g. GridMap) as tabs in the same slot |
| Centre | Document tab bar + active editor |
| Right upper | World Outliner (Scene dock) |
| Right lower | Details (Inspector), World Settings tab |
| Bottom | Content Browser (FileSystem dock), Output Log, then Godot's other bottom panels (Debugger, Animation, Shader Editor, …) |
| Hidden | Import, Signals, Groups, History — reachable from Window menu |

Docks remain movable. Window → Reset Layout restores the above.
World Settings shows the main scene/world-related project settings (Physics, Rendering
defaults, Audio) in an inspector, as UE4's World Settings tab does.

## 2. Theme (`strata_theme`)

Single built-in look; accent and base colour remain configurable in Editor Settings.

- Base panel ~`#242424`, dock background ~`#2f2f2f`; inputs/search fields near-black, square.
- Selection: muted tan/orange row highlight; viewport selection outline orange.
- Primary "add" buttons green (Add Component, Add/Import); script buttons blue.
- Inspector: darker section header bars with ▸/▾ and bold title; thin label/value divider.
- Vector3 fields: red/green/blue X/Y/Z tabs on the left of each field.
- Dense: ~9pt UI font, tight margins, square corners.
- Tabs: compact, icon + label + ×; active tab lighter, no accent underline.
- Godot icons kept except new large toolbar icons.

Exact colour values are sampled from the reference screenshots during implementation.

## 3. Toolbar (`strata_toolbar`) and document tabs (`strata_document_tabs`)

Toolbar: large icon-over-label buttons, ▾ for dropdowns.

| Button | Action |
|---|---|
| Save Current | Save active scene |
| Source Control ▾ | Godot VCS menu |
| Modes ▾ | Switch left-dock tab (Place Actors + plugin mode panels) |
| Content | Focus Content Browser |
| Marketplace | Open Asset Library document tab |
| Settings ▾ | Project Settings, Editor Settings, World Settings |
| Blueprints ▾ | Open / new / attach script on selection |
| Cinematics ▾ | Add AnimationPlayer to scene, open Animation panel |
| Build ▾ | Bake all LightmapGI, VoxelGI, NavigationRegion3D in the scene |
| Play ▾ | Play current scene embedded (Game tab); dropdown: main scene, separate window, Movie Maker. While running: Pause / Stop |
| Launch ▾ | Export and one-click deploy |

Godot's top-right run bar is removed; its functions live under Play.

Document tab bar replaces both the 2D/3D/Script/AssetLib buttons and the scene tabs:
- Each open scene is a tab; shows the 3D or 2D editor depending on the scene root.
- Script Editor tab appears when a script is opened; closable.
- Marketplace (Asset Library) tab; closable.
- Game tab appears and focuses while playing; closes on Stop.

Underlying editor plugins and scene handling are unchanged; only the switching UI is new.

## 4. Docks

### Place Actors (`place_actors_dock`)
Category list (Recently Placed, Basic, Lights, Cinematic, Visual Effects, Geometry, Volumes,
All Classes), search field, icon + name item list.

Items are one static table `{category, label, icon, builder}`; the builder returns a node tree.

| Item | Node tree |
|---|---|
| Empty Actor | Node3D |
| Empty Character | CharacterBody3D + capsule MeshInstance3D + capsule CollisionShape3D |
| Empty Pawn | CharacterBody3D |
| Player Start | Marker3D "PlayerStart", group `player_start` |
| Cube / Sphere / Cylinder / Cone / Plane | StaticBody3D + MeshInstance3D(primitive) + matching CollisionShape3D |
| Box / Sphere Trigger | Area3D + matching CollisionShape3D |
| Lights | DirectionalLight3D, OmniLight3D (Point), SpotLight3D, WorldEnvironment+Sky (Sky Light), ReflectionProbe |
| Cinematic | Camera3D, AnimationPlayer, Path3D |
| Visual Effects | GPUParticles3D, Decal, FogVolume |
| Geometry | CSGBox3D, CSGSphere3D, CSGCylinder3D, CSGCombiner3D |
| Volumes | NavigationRegion3D, VoxelGI, Area3D |
| All Classes | Every instantiable Node3D-derived class, searchable |

Drag into viewport: placed at cursor surface hit (reuse Node3D editor drop handling).
Double-click: added in front of the editor camera. Placement is one undo action.
Recently Placed is stored in editor project metadata.

### World Outliner (Scene dock)
Eye visibility toggle in a left column; Type column with node class; footer
"N nodes (M selected)".

### Details (Inspector)
Header: node name field. Bar: green "+ Add Component ▾" (Create Node dialog, adds child),
blue "Script / Add Script". Components tree: selected node + direct children, click selects.
"Search Details" filter. Section headers per theme.

### Content Browser (FileSystem dock, bottom)
Top bar: green Add/Import ▾ (folder, scene, script, resource, import), Save All, back/forward,
breadcrumb "Content ▸ …" (`Content` = `res://`). Filters button + search. Large tiles, name
below. Collapsible folder tree on the left. Footer: "N items", View Options.

## 5. Viewport and navigation

Viewport bar: top-left ▾ menu, Perspective (view/ortho), Lit (display modes), Show
(visibility toggles). Top-right inline: move/rotate/scale tools, world/local, surface snap,
grid snap (default 10), rotation snap (10°), scale snap (0.25), camera speed, maximise.

New `editors/3d/navigation/navigation_scheme` value "Unreal", default:
- RMB hold + WASD fly, Q/E down/up, wheel while flying = speed
- Alt+LMB orbit, Alt+RMB dolly, MMB pan, wheel zoom
- F focus selection, End snap to floor, W/E/R move/rotate/scale

Other schemes remain selectable.

## Error handling

- Place Actors builders only use core node types; CSG and navigation items are hidden if their
  modules are compiled out (`#ifdef MODULE_CSG_ENABLED` etc.).
- Toolbar actions that need an open scene are disabled when none is open (same rule Godot's
  menus use).
- Build ▾ reports per-bake failures in Output Log and continues with the remaining bakes.
- Layout loading falls back to the Strata default if a saved layout references missing docks
  (existing Godot behaviour).

## Testing

- Doctest (`scons tests=yes`, `tests/editor/`):
  - each Place Actors item builds a non-null tree whose root is the expected class;
  - `strata_layout` default config places each dock in its intended slot.
- Visual check per milestone: launch editor, compare against reference screenshots.

## Build order (one commit each; editor must build and run after each)

1. Theme
2. Default layout + UE4 names
3. Toolbar + document tabs
4. World Outliner + Details
5. Content Browser
6. Place Actors
7. Viewport bar + Unreal navigation

## Risks

- Replacing Godot's look entirely and editing `editor_node.cpp` increases conflicts when
  pulling upstream Godot; mitigated by keeping edits to plug-in points.
- Document tabs touch scene-tab and main-screen logic, the most entangled part of
  `editor_node.cpp`; budget extra time for milestone 3.
