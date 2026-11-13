# UE4 Editor Shell — Design

Date: 2026-11-12 · Status: approved in conversation, awaiting spec review

## Goal

Make the Strata (Godot 4.7.3-rc fork) editor **work** like the Unreal Engine 4.26 level editor
as closely as possible: same panel arrangement, toolbar, workflows, terminology and viewport
controls. Reference: the user's UE 4.26 screenshots (Minimal_Default level; Place Actors,
Landscape mode, World Settings). Screenshots are reference only and are **not** committed.

Guiding rule: **look and work like UE4.26, without copying Epic's assets.** Layout, workflow,
naming, controls and colour scheme follow UE4 (decided 2026-11-13: the theme matches UE4's
colours). Epic's images, icons and other editor assets are never used: icons are Godot's or
open-licence (Lucide, ISC). Deviate from UE4 behaviour only where Godot has nothing equivalent underneath,
and record the deviation here.

## Decisions

| Topic | Decision |
|---|---|
| Scope | Full UE4 shell: layout, theme, names, toolbar, Place Actors, Outliner Type column + folders, Details Add Component bar, Content Browser, viewport bar, UE4 navigation, UE4 terminology, Project Browser |
| Godot look | Replaced entirely by Strata's own theme. Classic/Modern styles and Godot's default layout are removed |
| Visual identity | UE4-like colour scheme (neutral greys, orange-gold selection, green primary buttons); icons are Godot's or recoloured Lucide (ISC). No Epic assets copied |
| Main-screen tabs | Replaced by one UE-style document tab bar |
| Wording | UE4 terms throughout the editor UI: panel/menu names, scene → Level, node → Actor, child nodes listed in Details → Components. Engine class names and the scripting API keep Godot names |
| Viewport controls | New "Unreal" navigation scheme, default |
| Code structure | New components in `editor/strata/`; Godot files get only plug-in edits |
| Out of scope | Icon redraw, Blueprints/anim graphs (phase 2), gameplay framework classes (phase 3) |

## Architecture

All new code lives in `editor/strata/`, one `.cpp/.h` pair per component, registered in an
`SCsub` included from `editor/SCsub`.

| File | Responsibility | Plugged in from |
|---|---|---|
| `strata_theme` | Builds the editor theme (colours, styleboxes, font sizes, constants) | `themes/editor_theme_manager.cpp` (replaces `ThemeClassic`/`ThemeModern` calls) |
| `strata_layout` | Default dock layout `ConfigFile` and UE4 dock titles | `editor_node.cpp` default-layout function (~line 9202) |
| `strata_toolbar` | Large icon toolbar row under the menu bar | `editor_node.cpp` UI construction (title bar area, ~line 8679) |
| `strata_document_tabs` | Single tab bar: open levels + tool screens | `editor_node.cpp`, `editor_main_screen`, `scene/editor_scene_tabs` |
| `place_actors_dock` | Place Actors dock + item table | registered as an `EditorDock` from `editor_node.cpp` |
| `outliner_folders` | Editor-only folders in the World Outliner | `docks/scene_tree_dock.cpp`, `gui/scene_tree_editor.cpp` |
| `content_browser_view` | Content Browser top bar, tile view, footer | `docks/filesystem_dock.cpp` |
| `strata_terms` | Central UE4 term table used when building UI strings | call sites listed in section 6 |
| `project_browser` | UE4-style Project Browser | `project_manager/` |

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
World Settings shows the main world-related project settings (Physics, Rendering defaults,
Audio) in an inspector, as UE4's World Settings tab does.

## 2. Theme (`strata_theme`)

Looks like UE4: dense, dark, panel-heavy, with UE4's colour scheme. Accent and base colour
remain configurable in Editor Settings.

- Flat neutral greys (no blue tint) for panels, darker gaps between panels.
- Orange-gold accent for selection, focus and highlights.
- Primary actions (Add Component, Add/Import, Open Project) are green buttons; the Play icon
  is green. Secondary actions neutral.
- Text fields lighter than the panels they sit on.
- Inspector: distinct section header bars with ▸/▾ and title; clear label/value divider.
- Vector3 fields: X/Y/Z axis colour coding (a common convention, shared with Godot's gizmos).
- Dense: ~9pt UI font, tight margins.
- Tabs: compact, icon + label + ×.
- Godot's icons kept; toolbar icons are recoloured Lucide (ISC).

## 3. Toolbar (`strata_toolbar`) and document tabs (`strata_document_tabs`)

Toolbar: large icon-over-label buttons, ▾ for dropdowns, same order and behaviour as UE4.

| Button | Action |
|---|---|
| Save Current | Save active level |
| Source Control ▾ | Godot VCS menu |
| Modes ▾ | Switch left-dock tab (Place Actors + plugin mode panels) |
| Content | Focus Content Browser |
| Marketplace | Open Asset Library document tab |
| Settings ▾ | Project Settings, Editor Settings, World Settings |
| Blueprints ▾ | Open / new / attach script on selection |
| Cinematics ▾ | Add AnimationPlayer to level, open Animation panel |
| Build ▾ | Bake all LightmapGI, VoxelGI, NavigationRegion3D in the level |
| Play ▾ | Play current level embedded (Game tab); dropdown: main level, separate window, Movie Maker. While running: Pause / Stop |
| Launch ▾ | Export and one-click deploy |

Godot's top-right run bar is removed; its functions live under Play.

Document tab bar replaces both the 2D/3D/Script/AssetLib buttons and the scene tabs:
- Each open level is a tab; shows the 3D or 2D editor depending on the root.
- Script Editor tab appears when a script is opened; closable.
- Marketplace (Asset Library) tab; closable.
- Game tab appears and focuses while playing; closes on Stop.

Underlying editor plugins and scene handling are unchanged; only the switching UI is new.

### Title bar (milestone 3c, added 2026-11-13)

As in UE4, the editor draws its own title bar and the document tabs live in it:
- Row 1 (title row): Strata logo, document tabs (levels + tool tabs), drag space,
  minimise / maximise / close. Double-click on drag space maximises or restores.
- Row 2: menu bar (Scene, Project, Debug, Editor, Help), then the toolbar as before.
- Windows: Godot's "extend to title" is implemented for Windows (it was macOS-only):
  the client area covers the native caption via `WM_NCCALCSIZE`, while `WM_NCHITTEST`
  keeps native resize edges, so the shadow and Aero Snap stay.
- Other platforms keep their current behaviour; the title row falls back to the native
  title bar where extend-to-title is unavailable.
- Known gap: the Windows 11 snap-layout flyout on hovering maximise is not provided.

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
- Eye visibility toggle in a left column; Type column with node class; footer
  "N actors (M selected)".
- Folders (`outliner_folders`): editor-only folders as in UE4. A node's folder path is stored
  as node metadata (`_strata_outliner_folder`, saved in the `.tscn`); folders are virtual rows
  in the tree and don't change the node hierarchy. Right-click → Move To → New Folder / existing
  folder; drag actors onto folders; rename/delete folder (delete moves actors to root).
  Deviation: folders group top-level actors only, because Godot's hierarchy is the real
  parent/child tree.

### Details (Inspector)
Header: actor name field. Bar: "+ Add Component ▾" (Create Node dialog, adds child),
"Blueprint/Add Script". Components tree: selected actor + direct children, click selects.
"Search Details" filter. Section headers per theme.

### Content Browser (FileSystem dock, bottom)
Top bar: Add/Import ▾ (folder, level, script, resource, import), Save All, back/forward,
breadcrumb "Content ▸ …" (`Content` = `res://`). Filters button + search. Large tiles, name
below. Collapsible folder tree on the left. Footer: "N items", View Options.

## 5. Viewport and navigation

Viewport bar: top-left ▾ menu, Perspective (view/ortho), Lit (display modes), Show
(visibility toggles). Top-right inline: move/rotate/scale tools, world/local, surface snap,
grid snap (default 10), rotation snap (10°), scale snap (0.25), camera speed, maximise.

New `editors/3d/navigation/navigation_scheme` value "Unreal", default:
- RMB hold + WASD fly, Q/E down/up, wheel while flying = speed
- LMB drag (no modifier, empty space) = move forward/back + turn, as UE4
- Alt+LMB orbit, Alt+RMB dolly, MMB pan, wheel zoom
- F focus selection, End snap to floor, W/E/R move/rotate/scale

Other schemes remain selectable.

## 6. Terminology (`strata_terms`)

Editor UI strings use UE4 terms. Code, class names, file formats and the scripting API keep
Godot names, so Godot docs and addons still work.

| Godot UI term | Strata UI term |
|---|---|
| Scene (open/save/new, scene tabs, "Scene" menu) | Level (File → New Level, Open Level, Save Current) |
| Node (in Outliner, menus, dialogs) | Actor |
| Child node (shown in Details components tree) | Component |
| Add Child Node | Add Component |
| Inspector | Details |
| Scene dock | World Outliner |
| FileSystem | Content Browser |
| Output | Output Log |
| Project Manager | Project Browser |
| AssetLib | Marketplace |
| Instantiate Child Scene | Place Level Instance |

Deviation: a saved `.tscn` used as a reusable prefab is still a scene underneath; in the UI it
is shown as a "Level" when opened as the main edited scene and as a "Prefab" asset type in the
Content Browser. (UE4's closest equivalent is a Blueprint class; that mapping is phase 2.)

Strings are changed via the term table at the call sites in `editor_node.cpp`, docks and
dialogs; the translation catalogue keys are updated to match.

## 7. Project Browser (`project_browser`)

Replaces the Project Manager's UI with UE4.26's Project Browser workflow (single page, as in
the user's 4.26 reference screenshot):
- Window titled "Strata Project Browser", page heading **Select or Create New Project**.
- **Recent Projects**: grid of tiles (project thumbnail, name below; selected tile
  highlighted with the Strata accent). A tile shows an engine-version badge in its corner
  when the project was last saved by a different engine version. A blank "Blank" tile comes
  first and starts a new empty project. Double-click opens; **More** opens a file dialog to
  browse for a project. Search field filters tiles.
- **New Project Categories**: list rows (icon, bold title, one-line description). Strata
  ships only **Games** at first; choosing it opens the template step: template tiles
  (Blank, First Person, Third Person, Top Down — shipped as small Strata template projects),
  Next.
- **Project Settings** page (UE4.26 layout): two columns of setting tiles, each a large
  icon button with a ▾ dropdown and a one-line description beside it:
  - **Quality**: Maximum Quality = Forward+, Scalable = Compatibility.
  - **Target Hardware**: Desktop / Console or Mobile / Tablet (Mobile picks the Mobile
    renderer and enables ETC2/ASTC texture import). Description notes it can be changed
    later in Project Settings.
  - **Starter Content**: With / No Starter Content.
  - UE4's Blueprint/C++ tile waits for Phase 2 (Blueprints); its raytracing tile is
    omitted (no Godot equivalent).
  - Below: "Select a location for your project to be stored." with Folder (with … browse)
    and Name fields; footer **Back**, **Create Project** (primary), **Cancel**.
- Footer: **Open Project** (primary button) and **Cancel**.
- Existing Godot Project Manager features (import, scan, rename, remove, tags, version
  control init) stay available under a "More ▾" menu.

## Error handling

- Place Actors builders only use core node types; CSG and navigation items are hidden if their
  modules are compiled out (`#ifdef MODULE_CSG_ENABLED` etc.).
- Toolbar actions that need an open level are disabled when none is open.
- Build ▾ reports per-bake failures in Output Log and continues with the remaining bakes.
- Layout loading falls back to the Strata default if a saved layout references missing docks.
- Outliner folder metadata pointing to a missing folder recreates the folder on load.
- Project templates missing from disk are hidden from New Project, with a warning in the log.

## Testing

- Doctest (`scons tests=yes`, new `tests/strata/` dir, guarded by `TOOLS_ENABLED` like
  `tests/scene/test_node.cpp`):
  - each Place Actors item builds a non-null tree whose root is the expected class;
  - `strata_layout` default config places each dock in its intended slot;
  - outliner folder metadata round-trips through save/load of a scene;
  - `strata_terms` has an entry for every term in section 6.
- Visual check per milestone: launch the editor and compare *layout and workflow* against the
  reference screenshots.

## Build order (one commit each; editor must build and run after each)

1. Theme
2. Default layout + dock names
3. Toolbar + document tabs (3a toolbar, 3b document tabs, 3c custom title bar)
4. World Outliner (incl. folders) + Details
5. Content Browser
6. Place Actors
7. Viewport bar + Unreal navigation
8. Terminology pass
9. Project Browser

## Risks

- Replacing Godot's look entirely and editing `editor_node.cpp` increases conflicts when
  pulling upstream Godot; mitigated by keeping edits to plug-in points.
- Document tabs touch scene-tab and main-screen logic, the most entangled part of
  `editor_node.cpp`; budget extra time for milestone 3.
- The terminology pass touches many UI strings across the editor; it runs last so earlier
  milestones aren't fighting string conflicts, and Godot tutorials will no longer match UI
  wording.
