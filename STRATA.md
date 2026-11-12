# Strata Engine

A fork of [Godot Engine](https://github.com/godotengine/godot) whose goal is to make
building a game feel as close as possible to **Unreal Engine 4** — the editor layout,
workflow and authoring tools. Rendering/graphics parity is explicitly **not** a goal.

Based on Godot 4.7.3-rc, upstream commit `8521df165a44db92bcc4c878d4d7e53714652c97`.

## Roadmap

### Phase 1 — Editor interface (current)
- UE4-style default layout: Modes/Place Actors panel left, viewport center,
  World Outliner + Details on the right, Content Browser docked at the bottom.
- UE4-style main toolbar (Save, Content, Settings, Blueprints, Build, Play/Launch).
- UE4-like dark theme, spacing and iconography (original assets, not copies).
- UE4 viewport navigation (RMB fly + WASD, Alt+LMB orbit, F to focus) as the default.
- UE4 terminology in the UI where it maps cleanly (Level, Actor, Details, Content Browser).

### Phase 2 — Authoring workflow
- Visual scripting graph in the style of Blueprints (event graph, construction script).
- Animation graph / state machine editor in the style of AnimBlueprints.
- Material graph editor.

### Phase 3 — Gameplay framework
- GameMode / PlayerController / Pawn / Character style framework classes.

## Building

Same as upstream Godot — see the
[compiling docs](https://docs.godotengine.org/en/latest/contributing/development/compiling/index.html).
On Windows: `scons platform=windows`.

## Tracking upstream

This repo was imported from a source archive, so it does not share git history with
Godot. To pull upstream changes, add the remote and merge with
`--allow-unrelated-histories` (once), or cherry-pick specific fixes.
