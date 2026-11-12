# Strata Toolbar Implementation Plan (Milestone 3a)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add UE4's level-editor toolbar (large icon-over-label buttons, ▾ dropdowns) under the menu bar, and move Godot's top-right run bar into its Play group.

**Architecture:** One new widget `StrataToolbar` (`editor/strata/strata_toolbar.*`) built from a static button table, so order and labels are unit-testable. Each button calls existing public editor APIs (`EditorNode::trigger_menu_option`, `EditorRunBar`, `SceneTreeDock`, `EditorDockManager`, `VersionControlEditorPlugin`). Build ▾ logic is a separate pure helper `StrataBuild` (`editor/strata/strata_build.*`) that finds bake targets in a level. `EditorNode` gets plug-in edits only: create the toolbar and hand it the run bar.

**Tech Stack:** C++17, Godot 4.7 editor, SCons, doctest (`tests=yes`).

**Spec:** `docs/superpowers/specs/2026-11-12-ue4-editor-shell-design.md` (section 3, toolbar half; Error handling). Milestone 3 is split: this plan is the toolbar (3a). Document tabs (3b) get their own plan because the spec's Risks section flags them as the most entangled part of `editor_node.cpp`.

## Global Constraints

- Match UE4.26 in how things work, not how they look. No UE colours, icons or assets. Godot's existing editor icons are used for toolbar buttons.
- Engine class names and the scripting API keep Godot names. Only UI strings change.
- Godot's removed pieces are deleted at the plug-in point, not left reachable: the run bar no longer sits in the title bar, and its Godot play buttons are hidden.
- Toolbar actions that need an open level are disabled when none is open.
- Build ▾ reports per-bake failures in Output Log and continues with the remaining bakes.
- The editor must build (`python -m SCons platform=windows target=editor tests=yes -j16`) and launch after every task. Test runs use self-contained mode (`bin/._sc_` exists).

## Deviations from spec (decided here)

- **Modes ▾** waits for milestone 6: the left slot has no docks until Place Actors exists, so the menu would be empty.
- **Settings ▾ → World Settings** waits for milestone 4, which builds the World Settings tab.
- **Large Strata-style toolbar icons** are not drawn yet; buttons show Godot's icons at toolbar size. Drawing icons is outside this plan.
- **Play ▾** offers Play Current Level, Play Main Level and Movie Maker Mode. UE4 has no "custom scene" play, so Godot's Play Custom Scene button is dropped.
- **Marketplace** and **Blueprints ▾ → Open Script Editor** switch Godot's main screen until milestone 3b replaces it with document tabs.

## Review Focus

1. **Run shortcuts** F5 (Play Main Level), F6 (Play Current Level), F7 (Pause), F8 (Stop) must still work with Godot's play buttons hidden. Covered by putting the F5/F6 shortcuts on Play ▾ items (MenuButton forwards item shortcuts) and checked in Task 3 Step 6.
2. **Debugger pause** must stay in sync. The debugger drives `EditorRunBar::get_pause_button()`, so Pause and Stop stay as the run bar's own buttons, reparented with the bar. Checked in Task 3 Step 6.
3. **No level open** (fresh project, all tabs closed): Save Current, Blueprints ▾ attach items, Cinematics ▾ and Build ▾ are disabled, not crashing. Pinned by the `StrataBuild` null-root test and checked in Task 3 Step 6.
4. **Lightmap with no save path** (unsaved level): Build ▾ logs an error naming the LightmapGI and continues with the other bakes. Pinned by `lightmap_save_path` tests in Task 2.
5. **Light custom accent** (for example yellow): primary button text must stay readable. Pinned by Task 1's luminance test.

---

### Task 1: Readable primary-button text for any accent

**Files:**
- Modify: `editor/strata/strata_theme.h`, `editor/strata/strata_theme.cpp:92-94`
- Test: `tests/strata/test_strata_theme.cpp`

**Interfaces:**
- Produces: `static Color StrataTheme::get_primary_button_font_color(const Color &p_accent);` Returns near-black `Color(0.08, 0.08, 0.08)` when `p_accent.get_luminance() > 0.5`, otherwise white `Color(1, 1, 1)`.

- [ ] **Step 1: Write the failing test**

```cpp
TEST_CASE("[Strata][Theme] Primary button text contrasts with the accent") {
	const Color dark_text(0.08, 0.08, 0.08);
	CHECK(StrataTheme::get_primary_button_font_color(Color(1, 0.9, 0.1)) == dark_text); // Yellow.
	CHECK(StrataTheme::get_primary_button_font_color(StrataTheme::get_default_preset().accent_color) == dark_text); // Teal.
	CHECK(StrataTheme::get_primary_button_font_color(Color(0.1, 0.15, 0.5)) == Color(1, 1, 1)); // Dark blue.
}
```

- [ ] **Step 2: Compile only the test object and confirm it fails**

Run: `python -m SCons platform=windows target=editor tests=yes -j16 bin/obj/tests/strata/test_strata_theme.windows.editor.x86_64.obj`
Expected: compile error, `get_primary_button_font_color` is not a member of `StrataTheme`.

- [ ] **Step 3: Implement** `get_primary_button_font_color`, and use it in `populate_overrides` in place of `p_config.font_dark_background_color` for the `StrataPrimaryButton` font colours.

- [ ] **Step 4: Build and run the Strata tests**

Run: `python -m SCons platform=windows target=editor tests=yes -j16`, then `bin/godot.windows.editor.x86_64.console.exe --test --test-case="*[Strata]*"`
Expected: `test cases: 7 | 7 passed`.

- [ ] **Step 5: Commit** `git commit -m "Strata theme: primary button text follows accent brightness"`

---

### Task 2: Build ▾ helper (`StrataBuild`)

**Files:**
- Create: `editor/strata/strata_build.h`, `editor/strata/strata_build.cpp`
- Test: `tests/strata/test_strata_build.cpp`

**Interfaces:**
- Produces:

```cpp
struct StrataBakeTargets {
	Vector<LightmapGI *> lightmaps;
	Vector<VoxelGI *> voxel_gis;
	Vector<NavigationRegion3D *> nav_regions;
};
namespace StrataBuild {
StrataBakeTargets collect_targets(Node *p_level_root); // Depth-first, root included. Null root returns empty lists.
String lightmap_save_path(const LightmapGI *p_lightmap, const String &p_level_path); // Existing light_data resource path if it is a file path, else <level basename>.lmbake, else "" when the level is unsaved.
void build_all(Node *p_level_root, bool p_lighting, bool p_navigation); // Bakes, logs each failure to Output Log with ERR_PRINT and continues.
}
```

- [ ] **Step 1: Write the failing tests** (`[Strata][Build]`):
  - `collect_targets(nullptr)` returns three empty lists.
  - A tree `Node3D` root → children `VoxelGI`, `Node3D` → grandchild `NavigationRegion3D`, plus a `LightmapGI` child finds exactly one of each, in tree order.
  - `lightmap_save_path(lm, "res://levels/arena.tscn")` with no light data is `"res://levels/arena.lmbake"`.
  - `lightmap_save_path(lm, "")` with no light data is `""`.
  - Free the trees with `memdelete(root)`.

- [ ] **Step 2: Compile the test object; confirm it fails** with `strata_build.h` not found.

- [ ] **Step 3: Implement.** `build_all` bakes VoxelGIs with `VoxelGI::bake()`, nav regions with `bake_navigation_mesh(false)`, and lightmaps with `LightmapGI::bake(root_or_parent, path, LightmapGIEditorPlugin::bake_func_step)` followed by `bake_func_end`, using the same root rule as `LightmapGIEditorPlugin::_bake_select_file`. An empty save path or a non-OK `BakeError` prints `vformat(TTR("Build: %s failed (%s)."), node path, reason)` to Output Log and moves on.

- [ ] **Step 4: Build and run the Strata tests.** Expected: `test cases: 11 | 11 passed`.

- [ ] **Step 5: Commit** `git commit -m "Strata: Build helper for lighting and navigation bakes"`

---

### Task 3: Toolbar widget and run bar move

**Files:**
- Create: `editor/strata/strata_toolbar.h`, `editor/strata/strata_toolbar.cpp`
- Test: `tests/strata/test_strata_toolbar.cpp`
- Modify: `editor/editor_node.h` (member `StrataToolbar *strata_toolbar`), `editor/editor_node.cpp:9087-9091` (run bar construction)
- Modify: `editor/run/editor_run_bar.h/.cpp` (add `void set_play_buttons_visible(bool p_visible);`)

**Interfaces:**
- Consumes: `StrataBuild::build_all`, `StrataBuild::collect_targets` (Task 2); `"StrataPrimaryButton"` theme variation (Task 1).
- Produces:

```cpp
struct StrataToolbarButton { StrataToolbar::ButtonId id; const char *label; const char *icon; bool has_dropdown; };
class StrataToolbar : public HBoxContainer {
	GDCLASS(StrataToolbar, HBoxContainer);
public:
	enum ButtonId { SAVE_CURRENT, SOURCE_CONTROL, CONTENT, MARKETPLACE, SETTINGS, BLUEPRINTS, CINEMATICS, BUILD, PLAY, LAUNCH };
	static Vector<StrataToolbarButton> get_buttons();
	void set_run_bar(EditorRunBar *p_run_bar); // Reparents it right after Play and hides Godot's play buttons.
};
```

- [ ] **Step 1: Write the failing test** (`[Strata][Toolbar]`): `get_buttons()` returns, in order, labels `Save Current, Source Control, Content, Marketplace, Settings, Blueprints, Cinematics, Build, Play, Launch`. `has_dropdown` is true exactly for Source Control, Settings, Blueprints, Cinematics, Build, Play, Launch. Each `icon` is one of `Save, VcsBranches, Folder, AssetStore, Tools, Script, Animation, Bake, Play, PlayRemote`, in that order.

- [ ] **Step 2: Compile the test object; confirm it fails** with `strata_toolbar.h` not found.

- [ ] **Step 3: Implement `StrataToolbar`.** Each button is a flat `Button` or `MenuButton` with `icon_alignment` centered above the label (`set_vertical_icon_alignment(VERTICAL_ALIGNMENT_TOP)`), minimum width `64 * EDSCALE`, and `▾` appended to the label when `has_dropdown`. Play uses `"StrataPrimaryButton"`. Actions:

| Button | Behaviour |
|---|---|
| Save Current | `trigger_menu_option(SCENE_SAVE_SCENE, false)` |
| Source Control ▾ | Version Control Metadata… → `VersionControlEditorPlugin::popup_vcs_metadata_dialog()`; Version Control Settings… → `popup_vcs_set_up_dialog(gui_base)` |
| Content | `EditorDockManager::get_singleton()->focus_dock(FileSystemDock::get_singleton())` |
| Marketplace | `EditorNode::get_editor_main_screen()->select(EditorMainScreen::EDITOR_ASSETLIB)`; disabled when `!is_button_enabled(EDITOR_ASSETLIB)` |
| Settings ▾ | Project Settings… → `PROJECT_OPEN_SETTINGS`; Editor Settings… → `EDITOR_OPEN_SETTINGS` |
| Blueprints ▾ | Open Script Editor → select `EDITOR_SCRIPT`; Attach Script to Selected → `SceneTreeDock::attach_script_to_selected(false)`; Extend Script on Selected → `attach_script_to_selected(true)` |
| Cinematics ▾ | Add AnimationPlayer to Level → add an `AnimationPlayer` child to the level root through `EditorUndoRedoManager`, select it; Open Animation Panel → `focus_dock(AnimationPlayerEditor::get_singleton())` |
| Build ▾ | Build All, Build Lighting Only, Build Navigation Only → `StrataBuild::build_all(root, …)` |
| Play ▾ | Button → `EditorRunBar::play_current_scene()`. Menu: Play Current Level (shortcut `editor/run_current_scene`), Play Main Level (`editor/run_project`), Movie Maker Mode (check item → `set_movie_maker_enabled`) |
| Launch ▾ | Export Project… → `PROJECT_EXPORT`; Pack Project as ZIP… → `PROJECT_PACK_AS_ZIP` |

Refresh the enabled state on each popup's `about_to_popup` and on `EditorNode`'s `scene_changed` signal: needs-level items are disabled when `get_edited_scene()` is null; attach items are also disabled with an empty selection.

Add `EditorRunBar::set_play_buttons_visible(bool)`, which shows or hides `play_button`, `play_scene_button`, `play_custom_scene_button` and `write_movie_panel`. Pause, Stop, the profiler indicator, recovery mode and one-click deploy stay. `set_run_bar` reparents the bar right after the Play button and calls `set_play_buttons_visible(false)`.

In `EditorNode`, replace `title_bar->add_child(project_run_bar)` with creating `strata_toolbar`, adding it to `main_vbox` directly after `title_bar`, and `strata_toolbar->set_run_bar(project_run_bar)`. Keep both `play_pressed`/`stop_pressed` connections.

- [ ] **Step 4: Build and run the Strata tests.** Expected: `test cases: 12 | 12 passed`.

- [ ] **Step 5: Run the full suite.** Run: `bin/godot.windows.editor.x86_64.console.exe --test > <workspace>/full.txt`. Expected: all pass (1386 + 6 new).

- [ ] **Step 6: Visual and behaviour check.** This needs a visible window. Ask the user before launching. Launch `bin/godot.windows.editor.x86_64.exe -e --path <scratchpad>/testproj` and confirm:
  - Toolbar row under the menu bar, in table order. No run bar in the title bar.
  - F6 plays the current level, F5 plays the main level, F7 pauses (the debugger shows a break), F8 stops.
  - With every level tab closed, the level-only items are disabled.
  - Build ▾ → Build All on the test level with one unsaved LightmapGI logs a failure and still bakes the others.

- [ ] **Step 7: Commit** `git commit -m "Strata toolbar: UE4 toolbar row with Play group and Build menu"`

---

## Next plans

- 3b Document tabs: one tab bar for open levels, Script Editor, Marketplace and Game. Removes the 2D/3D/Script/Game/AssetLib buttons and the scene tab strip.
- Milestones 4–9 as listed in the theme-and-layout plan.
