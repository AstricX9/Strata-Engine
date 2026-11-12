# Strata Theme and Layout Implementation Plan (Milestones 1–2)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace Godot's editor look with Strata's own theme and arrange the docks in the UE4 level-editor layout with UE4 dock names.

**Architecture:** New code lives in `editor/strata/`. The theme is Godot's Modern theme generator with Strata's own preset values plus a small `StrataTheme::populate_overrides()` pass; the Classic style and the style setting are deleted. The default dock layout comes from a pure function `StrataLayout::fill_default_layout()` that `EditorNode` calls, so it is unit-testable without an editor.

**Tech Stack:** C++17, Godot 4.7 editor, SCons, doctest (`tests=yes`).

**Spec:** `docs/superpowers/specs/2026-11-12-ue4-editor-shell-design.md` (sections Decisions, Architecture, 1, 2; build-order steps 1–2). Later milestones get their own plans.

## Global Constraints

- Match UE4.26 in how things work, not how they look. No UE colours, icons or assets. In particular the accent must not be UE4's tan/orange and buttons must not use UE4's green/blue scheme.
- Engine class names, file formats and the scripting API keep Godot names. Only UI strings change.
- Saved editor layouts from before this change must keep loading: dock layout keys stay `Scene`, `Inspector`, `FileSystem`, `Output`.
- Godot's removed pieces are deleted at the plug-in point, not left reachable.
- The editor must build (`python -m SCons platform=windows target=editor -j16`) and launch after every task.

## Review Focus

1. **Saved layout from plain Godot** (`editor_layout.cfg` with `Scene`/`Inspector` keys): docks must load into their saved slots, not vanish. Pinned by the layout-key constant test in Task 2.
2. **Non-default colour presets** (Light, Black (OLED), Custom): the Strata overrides must stay readable; overrides read colours from `config`, never hard-coded. Checked visually in Task 1 Step 6.
3. **Editor scale 75% and 200%**: compact spacing and 12px font must stay usable; every size in the overrides is multiplied by `EDSCALE`. Checked visually in Task 1 Step 6.
4. **Content Browser in the bottom slot at short heights**: FileSystem dock must switch to its horizontal layout and not clip its toolbar. Checked visually in Task 2 Step 7.
5. **Project Manager Quick Settings** without the style row: the dialog must open and save without errors. Checked in Task 1 Step 6.

---

### Task 1: Strata theme (milestone 1)

**Files:**
- Create: `editor/strata/SCsub`, `editor/strata/strata_theme.h`, `editor/strata/strata_theme.cpp`
- Create: `tests/strata/test_strata_theme.cpp`
- Modify: `editor/SCsub:97` (add `SConscript("strata/SCsub")`)
- Modify: `editor/themes/editor_theme_manager.h` (remove `String style;` from `ThemeConfiguration`; `default_corner_radius` 4 → 2)
- Modify: `editor/themes/editor_theme_manager.cpp:45,61,180-231,245,272-288,359-362,409-413`
- Modify: `editor/settings/editor_settings.cpp:510,605,618`
- Modify (delete Classic branches, keep the Modern branch): `editor/import/dynamic_font_import_settings.cpp:631`, `editor/project_manager/project_manager.cpp:309`, `editor/project_manager/quick_settings_dialog.cpp:67,102,205`, `editor/scene/gui/theme_editor_plugin.cpp:3502,3952`
- Delete: `editor/themes/theme_classic.h`, `editor/themes/theme_classic.cpp`

**Interfaces:**
- Produces:
  - `struct StrataThemePreset { Color base_color; Color accent_color; float contrast; float icon_saturation; int corner_radius; int base_spacing; int extra_spacing; Size2 dialogs_buttons_min_size; int main_font_size; };`
  - `class StrataTheme { public: static StrataThemePreset get_default_preset(); static void populate_overrides(const Ref<EditorTheme> &p_theme, EditorThemeManager::ThemeConfiguration &p_config); };`
  - Theme type variation `"StrataPrimaryButton"` (base `"Button"`), used by later milestones for Add Component, Add/Import and Play.

- [ ] **Step 1: Write the failing test** in `tests/strata/test_strata_theme.cpp` (namespace `TestStrataTheme`, file wrapped in `#ifdef TOOLS_ENABLED`, `#include "tests/test_macros.h"` and `"editor/strata/strata_theme.h"`):

```cpp
TEST_CASE("[Strata][Theme] Default preset is Strata's own") {
	const StrataThemePreset p = StrataTheme::get_default_preset();
	CHECK(p.base_color.is_equal_approx(Color(0.118, 0.122, 0.133)));
	CHECK(p.accent_color.is_equal_approx(Color(0.16, 0.74, 0.62)));
	CHECK(p.contrast == doctest::Approx(0.25));
	CHECK(p.icon_saturation == doctest::Approx(2.0));
	CHECK(p.corner_radius == 2);
	CHECK(p.base_spacing == 2);
	CHECK(p.extra_spacing == 2);
	CHECK(p.dialogs_buttons_min_size == Size2(90, 26));
	CHECK(p.main_font_size == 12);
}
```

- [ ] **Step 2: Build with tests and verify the test fails to compile** (first `tests=yes` build recompiles most of the engine, ~17 min)

Run: `python -m SCons platform=windows target=editor tests=yes -j16`
Expected: compile error, `editor/strata/strata_theme.h` not found.

- [ ] **Step 3: Implement `editor/strata/`**
  - `SCsub`: same pattern as `editor/themes/SCsub` minus the fonts block: `env.add_source_files(env.editor_sources, "*.cpp")`.
  - `get_default_preset()` returns exactly the test's values.
  - `populate_overrides()` runs after the Modern styles and only reads colours from `p_config`. It sets:
    - `TabContainer`/`TabBar` `tab_selected`: copy the existing stylebox, set `border_width_top` to 0 (no accent underline) and `bg_color = p_config.surface_high_color`.
    - `EditorInspectorCategory` `bg`: `StyleBoxFlat`, `bg_color = p_config.surface_lowest_color`, content margins `4 * EDSCALE` vertical and `6 * EDSCALE` horizontal.
    - `"StrataPrimaryButton"`: type variation of `"Button"`. `normal` uses `p_config.accent_color` darkened by 0.2, `hover` uses `accent_color`, `pressed` uses `accent_color` darkened by 0.35, corner radius `p_config.corner_radius * EDSCALE`. `font_color` is `p_config.font_dark_background_color` for every state.

- [ ] **Step 4: Plug it in and delete Classic**
  - `editor_theme_manager.cpp`:
    - Drop the `theme_classic.h` include and the `is_default_style` branches: always call `ThemeModern::populate_*`, then `StrataTheme::populate_overrides(theme, config)` right after `ThemeModern::populate_editor_styles`.
    - Remove `config.style` reads and its hash line.
    - In the theme-style block (lines ~272–288), keep only the `// Default` arm.
    - The `// Default` colour-preset arm uses `StrataTheme::get_default_preset()` for accent, base, contrast and icon saturation.
    - The `// Default` spacing arm uses its spacing values.
  - `editor_settings.cpp`:
    - Delete the `interface/theme/style` setting (line 605).
    - `main_font_size` default 14 → `StrataTheme::get_default_preset().main_font_size`.
    - `corner_radius` default 4 → 2.
    - `base_color` and `accent_color` defaults → the preset's colours.
  - In each listed file, remove the code path taken when the style is `"Classic"` and keep the other path. In `quick_settings_dialog.cpp`, remove the style option row entirely.
  - Delete `theme_classic.h/.cpp`.

- [ ] **Step 5: Build and run the test**

Run: `python -m SCons platform=windows target=editor tests=yes -j16`, then `bin/godot.windows.editor.x86_64.console.exe --test --test-case="*[Strata]*"`
Expected: build succeeds, with no remaining references to `ThemeClassic` or `interface/theme/style` (`git grep -n "ThemeClassic\|theme/style\""` prints nothing). Then `[doctest] test cases: 1 | 1 passed`.

- [ ] **Step 6: Visual check**

Launch `bin/godot.windows.editor.x86_64.exe -e --path <any 3D test project>`. Confirm:
- dark neutral base with teal accent and 12px font
- selected tabs have no accent underline
- Inspector categories show as darker header bars

Switch Editor Settings → Theme → Color Preset to Light, then back to Default; text must stay readable in both. Set display scale to 75% and then 200% and restart each time; nothing may overlap or clip. Open the Project Manager (`-p`) → Quick Settings: it opens with no Style row and saves without errors.

- [ ] **Step 7: Commit**

```bash
git add -A editor/strata tests/strata editor/SCsub editor/themes editor/settings/editor_settings.cpp editor/import/dynamic_font_import_settings.cpp editor/project_manager editor/scene/gui/theme_editor_plugin.cpp
git commit -m "Strata theme: own preset and overrides, remove Classic style"
```

---

### Task 2: UE4 default layout and dock names (milestone 2)

**Files:**
- Create: `editor/strata/strata_layout.h`, `editor/strata/strata_layout.cpp`
- Create: `tests/strata/test_strata_layout.cpp`
- Modify: `editor/editor_node.cpp:999-1007` (bottom dock order), `editor/editor_node.cpp:9193-9236` (default layout), `editor/editor_node.cpp:6642` (layout menu label)
- Modify: `editor/docks/scene_tree_dock.cpp:4997-5000`, `editor/docks/inspector_dock.cpp:707-711`, `editor/docks/filesystem_dock.cpp:4431-4435`, `editor/editor_log.cpp:488-491`

**Interfaces:**
- Consumes: nothing from Task 1.
- Produces (in `strata_layout.h`):
  - `namespace StrataLayout { inline constexpr const char *KEY_OUTLINER = "Scene"; KEY_DETAILS = "Inspector"; KEY_CONTENT_BROWSER = "FileSystem"; KEY_OUTPUT_LOG = "Output"; constexpr int RIGHT_COLUMN_WIDTH = 380; }`
  - `struct StrataDockKeys { String outliner, details, import, signals, groups, history; };`
  - `void StrataLayout::fill_default_layout(const Ref<ConfigFile> &p_layout, const StrataDockKeys &p_keys, int p_vsplit_count);`
  - `String StrataLayout::get_bottom_dock_order(const PackedStringArray &p_bottom_docks);` puts `KEY_CONTENT_BROWSER` first and `KEY_OUTPUT_LOG` second, keeps every other key in its original order, and returns them comma-joined.

- [ ] **Step 1: Write the failing tests** in `tests/strata/test_strata_layout.cpp` (same wrapper as Task 1):

```cpp
TEST_CASE("[Strata][Layout] Layout keys match Godot's so old layouts load") {
	CHECK(String(StrataLayout::KEY_OUTLINER) == "Scene");
	CHECK(String(StrataLayout::KEY_DETAILS) == "Inspector");
	CHECK(String(StrataLayout::KEY_CONTENT_BROWSER) == "FileSystem");
	CHECK(String(StrataLayout::KEY_OUTPUT_LOG) == "Output");
}

TEST_CASE("[Strata][Layout] Default layout places docks like UE4") {
	Ref<ConfigFile> cfg;
	cfg.instantiate();
	StrataLayout::fill_default_layout(cfg, { "Scene", "Inspector", "Import", "Signals", "Groups", "History" }, 4);
	CHECK(cfg->get_value("docks", "dock_5") == Variant("Scene")); // RIGHT_UL
	CHECK(cfg->get_value("docks", "dock_6") == Variant("Inspector")); // RIGHT_BL
	CHECK_FALSE(cfg->has_section_key("docks", "dock_3"));
	CHECK_FALSE(cfg->has_section_key("docks", "dock_4"));
	Array closed = cfg->get_value("docks", "dock_closed");
	CHECK(closed.size() == 4);
	CHECK(closed.has("Import"));
	CHECK(closed.has("Signals"));
	CHECK(closed.has("Groups"));
	CHECK(closed.has("History"));
	CHECK(int(cfg->get_value("docks", "dock_hsplit_3")) == -StrataLayout::RIGHT_COLUMN_WIDTH);
}

TEST_CASE("[Strata][Layout] Bottom order is Content Browser, Output Log, then the rest") {
	CHECK(StrataLayout::get_bottom_dock_order({ "Output", "Debugger", "FileSystem", "Audio" }) == "FileSystem,Output,Debugger,Audio");
	CHECK(StrataLayout::get_bottom_dock_order({ "Debugger" }) == "Debugger");
}
```

- [ ] **Step 2: Build and verify the tests fail**

Run: `python -m SCons platform=windows target=editor tests=yes -j16`
Expected: compile error, `editor/strata/strata_layout.h` not found.

- [ ] **Step 3: Implement `strata_layout.cpp`**
  - `fill_default_layout` writes into section `"docks"`:
    - `dock_5` = outliner and `dock_6` = details, each with `_selected_tab_idx` 0.
    - `dock_closed` = Array of the four other keys.
    - `dock_hsplit_1..4` = `{ 0, 0, -RIGHT_COLUMN_WIDTH, 0 }`.
    - `dock_split_1..p_vsplit_count` = 0.
  - It does not write `dock_9`; that is set at READY via `get_bottom_dock_order`.

- [ ] **Step 4: Plug into the docks and EditorNode**
  - Default slots and names, each keeping its old layout key:
    - SceneTreeDock: `set_name(TTRC("World Outliner"))`, `set_layout_key(StrataLayout::KEY_OUTLINER)`, shortcut text "Open World Outliner Dock", slot `DOCK_SLOT_RIGHT_UL`.
    - InspectorDock: "Details", `KEY_DETAILS`, "Open Details Dock", `DOCK_SLOT_RIGHT_BL`.
    - FileSystemDock: "Content Browser", `KEY_CONTENT_BROWSER`, "Open Content Browser Dock", `DOCK_SLOT_BOTTOM`.
    - EditorLog: "Output Log", `KEY_OUTPUT_LOG`, "Toggle Output Log Dock".
  - `editor_node.cpp` default layout:
    - Replace the three `dock_3/4/5` blocks and the hsplit/vsplit loops with one `StrataLayout::fill_default_layout(default_layout, keys, editor_dock_manager->get_vsplit_count())` call. Fill the keys from each dock's `get_effective_layout_key()`.
    - Set `main_hsplit` split offsets to `{ 0, -RIGHT_COLUMN_WIDTH * EDSCALE }`.
    - Keep the `bottom_panel_offsets` block.
  - At NOTIFICATION_READY, set `dock_9` to `StrataLayout::get_bottom_dock_order(bottom_docks)` instead of the plain join.
  - Layout menu item text "Default" → "Reset Layout" ("Reset Layout (Overridden)" when overridden). Milestone 8 moves it into a Window menu.

- [ ] **Step 5: Build and run the tests**

Run: `python -m SCons platform=windows target=editor tests=yes -j16`, then `bin/godot.windows.editor.x86_64.console.exe --test --test-case="*[Strata]*"`
Expected: `test cases: 4 | 4 passed`.

- [ ] **Step 6: Check that old layouts still load**

Copy any project, add a plain-Godot `.godot/editor/editor_layout.cfg` with `dock_3="Scene,Import"` and `dock_5="Inspector,Signals,Groups"`, and open it. Scene/Inspector must appear in those saved slots under their new titles. Then Editor → Editor Layout → Reset Layout must move them to the UE4 arrangement.

- [ ] **Step 7: Visual check against the reference screenshots**

Start with a fresh editor settings folder (launch with `--editor-settings-path` pointing to an empty temp dir, or delete `editor_layout.cfg` in the test project). Confirm:
- World Outliner sits top right and Details bottom right, about 380px wide.
- Content Browser is the first bottom tab and Output Log the second.
- No left dock is shown.
- Import, Signals, Groups and History are closed but reopenable from the dock menu.

Drag the bottom panel down to its minimum height: the Content Browser must use its horizontal layout without clipping its toolbar.

- [ ] **Step 8: Commit**

```bash
git add editor/strata tests/strata editor/editor_node.cpp editor/docks/scene_tree_dock.cpp editor/docks/inspector_dock.cpp editor/docks/filesystem_dock.cpp editor/editor_log.cpp
git commit -m "Strata layout: UE4 dock arrangement and dock names"
```

---

## Next plans

One plan per remaining milestone, each written once the previous one has landed:
3. Toolbar + document tabs
4. World Outliner (incl. folders) + Details + World Settings tab (spec section 1)
5. Content Browser
6. Place Actors (fills the empty left column; plugin mode panels join it as tabs)
7. Viewport bar + Unreal navigation
8. Terminology pass (also adds the Window menu)
9. Project Browser
