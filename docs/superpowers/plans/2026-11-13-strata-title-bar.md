# Strata Custom Title Bar Implementation Plan (Milestone 3c)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Draw the editor's own title bar on Windows, as UE4 does. The document tabs and the minimise/maximise/close buttons go in the title row, and the menu bar moves to a row below it.

**Architecture:** On Windows, implement `FEATURE_EXTEND_TO_TITLE` / `WINDOW_FLAG_EXTEND_TO_TITLE`:
- **Window frame:** `WM_NCCALCSIZE` removes the native caption but keeps the side and bottom frame. `WM_NCHITTEST` returns native resize codes along the top edge.
- **Hit-test logic:** a pure, header-only helper (`platform/windows/strata_window_frame.h`), so it can be unit-tested.
- **Editor side:** Godot already has a code path for extend-to-title (`can_expand`, written for macOS), which makes `EditorTitleBar` drag the window. Strata adds window buttons (`editor/strata/strata_window_buttons.*`) and a logo, moves the scene tab row into the title bar, and moves the menu bar to its own row underneath.

**Tech Stack:** C++17, Win32 (DWM), Godot 4.7 editor, SCons, doctest (`tests=yes`).

**Spec:** `docs/superpowers/specs/2026-11-12-ue4-editor-shell-design.md` (section 3, "Title bar (milestone 3c)").

## Global Constraints

- Look and work like UE4.26 without copying Epic's assets. The logo is Strata's own: the Godot logo stays until a Strata logo exists, and no UE logo is used.
- Native Windows behaviour is kept: the resize edges, drop shadow, Aero Snap and the system menu (Alt+Space).
- Platforms without extend-to-title (Linux X11/Wayland) keep the native title bar. The tabs stay in their current row there.
- The game window and other editor windows are unaffected. Only `MAIN_WINDOW_ID` uses the flag, and only when the editor requests it.
- The editor must build (`python -m SCons platform=windows target=editor tests=yes -j16`) and launch after every task. Test runs use self-contained mode (`bin/._sc_`).

## Review Focus

1. **Maximised window:** with the caption removed, Windows overhangs a maximised window by the frame thickness. Without compensating in `WM_NCCALCSIZE`, the top row is cut off and the bottom spills onto the next monitor. Pinned by the `client_rect` helper test (Task 1).
2. **Display scale 100% / 150% / 200%** and per-monitor DPI changes: the frame thickness must come from `GetSystemMetricsForDpi`, not fixed pixels. Checked by the user in Task 2 Step 6.
3. **Resize from the top edge** must still work, because the title row sits where the native top border used to be. Pinned by the `hit_test` helper test (Task 1).
4. **Fullscreen editor (Shift+F11)** and Godot's "Distraction Free" mode must not show a gap or a stray frame. Checked by the user in Task 2 Step 6.
5. **Popups and dialogs** (EditorSettings, file dialogs) must keep their native title bars. Covered by applying the flag to the main window only.

---

### Task 1: Windows extend-to-title

**Files:**
- Create: `platform/windows/strata_window_frame.h`
- Modify: `platform/windows/display_server_windows.h/.cpp` (feature flag, window flag storage, `WM_NCCALCSIZE`, `WM_NCHITTEST`, DWM frame margins)
- Test: `tests/strata/test_strata_window_frame.cpp` (Windows-only, `#ifdef WINDOWS_ENABLED`)

**Interfaces:**
- Produces:

```cpp
namespace StrataWindowFrame {
enum Hit { HIT_CLIENT, HIT_TOP, HIT_TOP_LEFT, HIT_TOP_RIGHT };
// p_point is in window coordinates (0,0 = top-left of the window rect); p_border is the DPI-scaled resize border.
Hit hit_test(Vector2i p_point, Vector2i p_window_size, int p_border, bool p_maximized);
// The client rect for a window rect: the native caption is removed, the side and bottom frame kept.
// When maximised, the top is moved in by the frame thickness, so nothing sits off-screen.
Rect2i client_rect(Rect2i p_window_rect, int p_frame, bool p_maximized);
}
```

- [ ] **Step 1: Write the failing tests** (`[Strata][WindowFrame]`):
  - `hit_test((500, 2), (1000, 800), 6, false)` → `HIT_TOP`; `(2, 2)` → `HIT_TOP_LEFT`; `(997, 2)` → `HIT_TOP_RIGHT`; `(500, 20)` → `HIT_CLIENT`.
  - When maximised, `(500, 2)` → `HIT_CLIENT` (no resizing a maximised window).
  - `client_rect((0, 0, 1000, 800), 8, false)` → `(8, 0, 984, 792)`: no caption and no top frame; side and bottom frame kept.
  - `client_rect((-8, -8, 1936, 1056), 8, true)` → `(0, 0, 1920, 1040)`: the maximised overhang is removed on all sides.

- [ ] **Step 2: Compile the test object; confirm it fails** with `strata_window_frame.h` not found.

- [ ] **Step 3: Implement.**
  - Implement the helpers.
  - In `DisplayServerWindows`:
    - `has_feature(FEATURE_EXTEND_TO_TITLE)` returns true.
    - Store `extend_to_title` in `WindowData`. `window_set_flag` / `window_get_flag` handle `WINDOW_FLAG_EXTEND_TO_TITLE`, and setting it triggers `SetWindowPos(SWP_FRAMECHANGED)`.
    - `WM_NCCALCSIZE` (when the flag is set and `wParam` is TRUE) applies `client_rect` with the frame from `GetSystemMetricsForDpi(SM_CXSIZEFRAME) + SM_CXPADDEDBORDER`, and returns 0.
    - `WM_NCHITTEST` (when the flag is set) maps `hit_test` results to `HTTOP` / `HTTOPLEFT` / `HTTOPRIGHT`. Otherwise it uses `DefWindowProc` (sides and bottom stay native).
    - `DwmExtendFrameIntoClientArea` with margins `{0, 0, 1, 0}` keeps the shadow.
    - `window_get_safe_title_margins` returns zero margins (the editor draws its own buttons).

- [ ] **Step 4: Build and run the Strata tests.** Expected: all pass.

- [ ] **Step 5: Commit** `Windows: extend-to-title for the editor window (keeps resize edges, shadow and snap)`

---

### Task 2: Title row with tabs and window buttons

**Files:**
- Create: `editor/strata/strata_window_buttons.h/.cpp`
- Modify: `editor/editor_node.cpp` (title row contents, menu row, `_update_main_menu_type` target)
- Test: `tests/strata/test_strata_window_buttons.cpp`; live check in the test project's snapshot plugin

**Interfaces:**
- Consumes: `FEATURE_EXTEND_TO_TITLE` (Task 1).
- Produces:

```cpp
class StrataWindowButtons : public HBoxContainer {
	GDCLASS(StrataWindowButtons, HBoxContainer);
public:
	enum Action { ACTION_MINIMIZE, ACTION_MAXIMIZE, ACTION_RESTORE, ACTION_CLOSE };
	// Which middle button shows for a window mode (maximise or restore).
	static Action toggle_action_for(Window::Mode p_mode);
};
```

- [ ] **Step 1: Write the failing tests:**
  - `toggle_action_for(MODE_WINDOWED)` → `ACTION_MAXIMIZE`.
  - `toggle_action_for(MODE_MAXIMIZED)` → `ACTION_RESTORE`.
  - Live check (snapshot plugin) prints `TITLE: tabs_in_title_bar=<bool> window_buttons=<bool> extend=<bool>`, then `TITLE: PASS` when all are true. Expected now: compile failure, and `TITLE: FAIL`.

- [ ] **Step 2: Confirm both fail.**

- [ ] **Step 3: Implement.**
  - `StrataWindowButtons`:
    - Three flat buttons using the `Window`-style icons that already exist in Godot (`GuiClose`, plus minimise and maximise drawn as simple 1px-line SVGs in `editor/icons/strata/`).
    - Close triggers `EditorNode::SCENE_QUIT` through `trigger_menu_option`, so unsaved-changes prompts still appear. Minimise and maximise/restore set the window mode.
    - The close button turns red on hover, as Windows' does.
  - In `EditorNode`, when `can_expand`:
    - Add a Strata logo (`TitleBarLogo` icon) at the left of `title_bar`.
    - Reparent `scene_tabs` (EditorSceneTabs) into `title_bar` after the logo, expanding.
    - Add `StrataWindowButtons` at the right end.
    - Hide the old spacers.
    - Create a `menu_row` `HBoxContainer` under `title_bar` in `main_vbox`, and make `_update_main_menu_type` put the menu bar there instead of in `title_bar`.
  - When not `can_expand`, keep everything as it is now.

- [ ] **Step 4: Build; run the Strata tests and the full suite.** Expected: all pass.

- [ ] **Step 5: Live check and snapshot.** Expected: `TITLE: PASS`. In the snapshot:
  - tabs and window buttons sit in the top row;
  - the menu bar is in the row below;
  - there is no Windows title bar.

- [ ] **Step 6: User check** (needs a visible window; ask first). Confirm:
  - drag by the empty title area;
  - double-click to maximise and restore;
  - resizing from every edge, including the top;
  - Aero Snap (drag to a screen edge, Win+arrows);
  - minimise / maximise / close (with unsaved changes it asks first);
  - Shift+F11 fullscreen;
  - a second monitor at a different scale, if available.

- [ ] **Step 7: Commit** `Strata title bar: tabs and window buttons in the title row, menus below (UE4)`

---

## Next plans

- Milestone 4: World Outliner + Details.
