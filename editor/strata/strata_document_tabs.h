/**************************************************************************/
/*  strata_document_tabs.h                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "scene/gui/tab_bar.h"

class EditorMainScreen;
class EditorSceneTabs;

// UE4-style document tabs: the tool tabs (Script, Marketplace, Game, plugin screens) shown in the same row as the level tabs.
class StrataDocumentTabs : public TabBar {
	GDCLASS(StrataDocumentTabs, TabBar);

public:
	enum ToolTab {
		TOOL_SCRIPT,
		TOOL_MARKETPLACE,
		TOOL_GAME,
		TOOL_PLUGIN,
	};

	struct ToolState {
		bool script_enabled = true; // Feature profile.
		bool scripts_open = false;
		bool script_dismissed = false;
		bool marketplace_enabled = true;
		bool marketplace_open = false;
		bool game_enabled = true; // Feature profile; false in recovery mode, which has no Game screen.
		bool game_selected = false; // Game screen opened without playing (Ctrl+F4).
		bool playing = false;
		int plugin_screens = 0;
	};

	struct TabEntry {
		ToolTab tool;
		int plugin_index; // -1 unless tool == TOOL_PLUGIN.
	};

private:
	EditorSceneTabs *scene_tabs = nullptr;
	EditorMainScreen *main_screen = nullptr;
	ToolState state;
	Vector<TabEntry> shown;
	bool updating = false;

	int _screen_for(const TabEntry &p_entry) const;
	void _rebuild();
	void _queue_rebuild();
	void _sync_selection();
	void _show_level();
	void _screen_changed(int p_index);
	void _tab_selected(int p_tab);
	void _tab_closed(int p_tab);
	void _level_clicked(int p_tab);

protected:
	void _notification(int p_what);

public:
	// Order: Script, Marketplace, Game, then plugin screens.
	static Vector<TabEntry> get_visible_tools(const ToolState &p_state);
	// EditorMainScreen::EDITOR_2D for CanvasItem roots, EDITOR_3D otherwise.
	static int level_screen(const Node *p_root);
	// Every main screen except 2D and 3D is shown as a tool tab.
	static bool is_tool_screen(int p_screen);

	// Puts the tool tabs in the level tab row and keeps them in sync with the main screen.
	void setup(EditorSceneTabs *p_scene_tabs, EditorMainScreen *p_main_screen);
};
