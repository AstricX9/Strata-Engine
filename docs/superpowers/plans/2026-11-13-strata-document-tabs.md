# Strata Document Tabs Implementation Plan (Milestone 3b)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace Godot's 2D/3D/Script/Game/Asset Store title-bar buttons with UE4-style document tabs: one row with a tab per open level, plus Script, Marketplace and Game tabs that appear when used.

**Architecture:** Godot's scene tab bar (`EditorSceneTabs`) keeps holding the level tabs, untouched, because `EditorNode` treats its tab index as the scene index. A new `StrataDocumentTabs` (a `TabBar` in `editor/strata/strata_document_tabs.*`) holds the tool tabs and sits in the same row, right after the level tabs. Which tool tabs show, in what order, and which main screen a level uses come from pure static functions, so they are unit-tested. Switching still goes through `EditorMainScreen::select`, which gets one new signal so the tabs follow switches made elsewhere (double-clicking a script, keyboard shortcuts, the toolbar).

**Tech Stack:** C++17, Godot 4.7 editor, SCons, doctest (`tests=yes`).

**Spec:** `docs/superpowers/specs/2026-11-12-ue4-editor-shell-design.md` (section 3, document-tab half).

## Global Constraints

- Look and work like UE4.26 without copying Epic's assets.
- Engine class names and the scripting API keep Godot names. Only UI strings change.
- Underlying editor plugins and scene handling are unchanged; only the switching UI is new.
- Godot's removed pieces are deleted or bypassed at the plug-in point, not left reachable: the title-bar main-screen buttons are hidden.
- The editor must build (`python -m SCons platform=windows target=editor tests=yes -j16`) and launch after every task. Test runs use self-contained mode (`bin/._sc_`).

## Decisions made here

- **Two tab bars in one row.** Levels stay in Godot's `TabBar` (its index is the scene index throughout `EditorNode`); tools live in a second `TabBar` after it. When a tool tab is active, the current level tab is drawn in the unselected style rather than deselected, so no scene index changes.
- **Plugin main screens** (third-party plugins with `_has_main_screen`) appear as permanent tool tabs after Game. UE4 has no equivalent, and hiding them would make them unreachable.
- **Closing the Script tab** only hides the tab; the scripts stay open. It reappears when a script is opened or switched to.
- **Ctrl+F1…F5** keep working. They switch screens through `EditorMainScreen`, and the tabs follow.

## Review Focus

1. **Scene index safety:** closing, reordering or switching levels while a tool tab is active must open the right level. Covered by never deselecting or reindexing the level `TabBar`.
2. **Script opened from elsewhere** (double-clicking a script in the Content Browser, an error in Output Log): the Script tab must appear and become active. Covered by the `screen_changed` signal and checked in Task 2 Step 5.
3. **Playing with the game embedded:** the Game tab appears and activates on Play and closes on Stop. Checked in Task 2 Step 5.
4. **A 2D level** (root is a `Node2D` or `Control`) opens in the 2D editor, and a 3D one in the 3D editor, when switching back from a tool tab. Pinned by Task 1's `level_screen` test.
5. **Feature profile with Script or Asset Library disabled:** those tabs never appear. Pinned by Task 1's visibility test.

---

### Task 1: Tab model

**Files:**
- Create: `editor/strata/strata_document_tabs.h`, `editor/strata/strata_document_tabs.cpp`
- Test: `tests/strata/test_strata_document_tabs.cpp`

**Interfaces:**
- Produces:

```cpp
class StrataDocumentTabs : public TabBar {
	GDCLASS(StrataDocumentTabs, TabBar);
public:
	enum ToolTab { TOOL_SCRIPT, TOOL_MARKETPLACE, TOOL_GAME, TOOL_PLUGIN };
	struct ToolState {
		bool script_enabled = true; // Feature profile.
		bool scripts_open = false;
		bool script_dismissed = false;
		bool marketplace_enabled = true;
		bool marketplace_open = false;
		bool playing = false;
		int plugin_screens = 0;
	};
	struct TabEntry { ToolTab tool; int plugin_index; }; // plugin_index is -1 unless tool == TOOL_PLUGIN.
	static Vector<TabEntry> get_visible_tools(const ToolState &p_state); // Order: Script, Marketplace, Game, plugins.
	static int level_screen(const Node *p_root); // EditorMainScreen::EDITOR_2D for CanvasItem roots, else EDITOR_3D.
};
```

- [ ] **Step 1: Write the failing tests** (`[Strata][DocumentTabs]`):
  - Default `ToolState` (nothing open, not playing, no plugins): no tool tabs.
  - Scripts open, Marketplace open, playing, 2 plugin screens: Script, Marketplace, Game, Plugin 0, Plugin 1, in that order.
  - Scripts open but `script_dismissed`: no Script tab. `script_enabled` false: no Script tab even when open. `marketplace_enabled` false: no Marketplace tab.
  - `level_screen`: `Node3D` → 3D, `Node2D` → 2D, `Control` → 2D, plain `Node` → 3D, `nullptr` → 3D.

- [ ] **Step 2: Compile the test object; confirm it fails** with `strata_document_tabs.h` not found.

- [ ] **Step 3: Implement** the two static functions; the class body is otherwise empty for now.

- [ ] **Step 4: Build and run the Strata tests.** Expected: all `*Strata*` tests pass.

- [ ] **Step 5: Commit** `Strata document tabs: tab model`

---

### Task 2: Tabs in the editor

**Files:**
- Modify: `editor/strata/strata_document_tabs.h/.cpp` (widget behaviour)
- Modify: `editor/editor_main_screen.h/.cpp` (signal `screen_changed(index)` emitted at the end of `select`)
- Modify: `editor/scene/editor_scene_tabs.h/.cpp` (`TabBar *get_tab_bar() const;` and `void add_after_tabs(Control *p_control);`, which inserts right after the level `TabBar`)
- Modify: `editor/editor_node.cpp` (create the tool tabs, hide `main_editor_button_hb`)

**Interfaces:**
- Consumes: `get_visible_tools`, `level_screen` (Task 1); `EditorRunBar` signals `play_pressed`/`stop_pressed`; `ScriptEditor` signals `editor_script_changed`/`script_close`.
- Produces: `void StrataDocumentTabs::setup(EditorSceneTabs *p_scene_tabs, EditorMainScreen *p_main_screen);`

- [ ] **Step 1: Add the live check** to the test project's snapshot plugin (`testproj/addons/snap`). With `STRATA_SNAP` set, it opens its own script (`res://addons/snap/snap.gd`) with `EditorInterface.edit_script`, waits, and prints:
  - `TABS: main_buttons_visible=<bool> tool_tabs=<titles> current=<title>`
  - `TABS: PASS` when the title-bar buttons are hidden, a "Script" tool tab exists and it is current.
  - `TABS: FAIL` otherwise.
  Run it on the current build. Expected: `TABS: FAIL` (no tool tabs exist yet).

- [ ] **Step 2: Implement.**
  - `setup` calls `add_after_tabs(this)`.
  - Rebuild tabs from `get_visible_tools` on: `screen_changed`, `editor_script_changed`, `script_close`, `play_pressed`, `stop_pressed`, and feature-profile changes. A Script, Marketplace or Game switch arriving through `screen_changed` also marks that tool open and clears `script_dismissed`.
  - Tool tabs are closable (`TabBar::CLOSE_BUTTON_SHOW_ACTIVE_ONLY`), except Game (closed by Stop) and plugin tabs.
  - Closing Script sets `script_dismissed`. Closing Marketplace clears `marketplace_open`. Either then switches back to the current level.
  - Selecting a tool tab calls `EditorMainScreen::select`. Clicking a level tab (`tab_clicked` on the level `TabBar`) while a tool is active selects `level_screen(edited scene root)`.
  - While a tool tab is active, override the level `TabBar`'s `tab_selected` style with its `tab_unselected` style; remove the override when a level is active. When a level is active, this tab bar has `current_tab` −1 (`set_deselect_enabled(true)`).
  - In `EditorNode`, hide `main_editor_button_hb` (it stays as `EditorMainScreen`'s button container) and call `setup` after both scene tabs and main screen exist.

- [ ] **Step 3: Build, run the Strata tests and the full suite.** Expected: all pass.

- [ ] **Step 4: Run the live check.** Expected: `TABS: PASS`.

- [ ] **Step 5: Snapshot check.** View the snapshot. Confirm:
  - one tab row with the level tab, then a Script tab;
  - no 2D/3D/Script/Game/Asset Store buttons in the title bar.
  Then ask the user to check, in the open editor:
  - Play shows a Game tab, and Stop removes it;
  - Marketplace on the toolbar opens a closable Marketplace tab;
  - clicking the level tab returns to the 3D view.

- [ ] **Step 6: Commit** `Strata document tabs: one UE4-style tab row for levels and tools`

---

## Next plans

- Milestone 4: World Outliner (actors only, collapsed by default, Type column, folders, actor count) and Details (component list, Add Component, World Settings tab).
- Milestones 5–9 as listed in the theme-and-layout plan.
