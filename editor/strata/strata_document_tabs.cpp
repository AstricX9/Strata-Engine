/**************************************************************************/
/*  strata_document_tabs.cpp                                              */
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

#include "strata_document_tabs.h"

#include "core/object/callable_mp.h"
#include "editor/editor_main_screen.h"
#include "editor/editor_node.h"
#include "editor/plugins/editor_plugin.h"
#include "editor/run/editor_run_bar.h"
#include "editor/scene/editor_scene_tabs.h"
#include "editor/settings/editor_feature_profile.h"
#include "editor/script/script_editor_plugin.h"
#include "scene/main/canvas_item.h"

Vector<StrataDocumentTabs::TabEntry> StrataDocumentTabs::get_visible_tools(const ToolState &p_state) {
	Vector<TabEntry> tabs;
	if (p_state.script_enabled && p_state.scripts_open && !p_state.script_dismissed) {
		tabs.push_back({ TOOL_SCRIPT, -1 });
	}
	if (p_state.marketplace_enabled && p_state.marketplace_open) {
		tabs.push_back({ TOOL_MARKETPLACE, -1 });
	}
	if (p_state.game_enabled && (p_state.playing || p_state.game_selected)) {
		tabs.push_back({ TOOL_GAME, -1 });
	}
	for (int i = 0; i < p_state.plugin_screens; i++) {
		tabs.push_back({ TOOL_PLUGIN, i });
	}
	return tabs;
}

bool StrataDocumentTabs::is_tool_screen(int p_screen) {
	return p_screen != EditorMainScreen::EDITOR_2D && p_screen != EditorMainScreen::EDITOR_3D;
}

int StrataDocumentTabs::level_screen(const Node *p_root) {
	return Object::cast_to<CanvasItem>(p_root) ? EditorMainScreen::EDITOR_2D : EditorMainScreen::EDITOR_3D;
}

// Plugin main screens come after Godot's five built-in ones (2D, 3D, Script, Game, Asset Library).
static constexpr int BUILTIN_SCREENS = EditorMainScreen::EDITOR_ASSETLIB + 1;

int StrataDocumentTabs::_screen_for(const TabEntry &p_entry) const {
	switch (p_entry.tool) {
		case TOOL_SCRIPT:
			return EditorMainScreen::EDITOR_SCRIPT;
		case TOOL_MARKETPLACE:
			return EditorMainScreen::EDITOR_ASSETLIB;
		case TOOL_GAME:
			return EditorMainScreen::EDITOR_GAME;
		case TOOL_PLUGIN:
			return BUILTIN_SCREENS + p_entry.plugin_index;
	}
	return EditorMainScreen::EDITOR_3D;
}

void StrataDocumentTabs::_queue_rebuild() {
	callable_mp(this, &StrataDocumentTabs::_rebuild).call_deferred();
}

void StrataDocumentTabs::_rebuild() {
	ScriptEditor *script_editor = ScriptEditor::get_singleton();
	const int screen_count = main_screen->get_plugin_count();
	state.script_enabled = screen_count > EditorMainScreen::EDITOR_SCRIPT && main_screen->is_button_enabled(EditorMainScreen::EDITOR_SCRIPT);
	state.scripts_open = (script_editor && !script_editor->get_open_scripts().is_empty()) || main_screen->get_selected_index() == EditorMainScreen::EDITOR_SCRIPT;
	state.marketplace_enabled = screen_count > EditorMainScreen::EDITOR_ASSETLIB && main_screen->is_button_enabled(EditorMainScreen::EDITOR_ASSETLIB);
	state.game_enabled = screen_count > EditorMainScreen::EDITOR_GAME && main_screen->is_button_enabled(EditorMainScreen::EDITOR_GAME);
	state.game_selected = main_screen->get_selected_index() == EditorMainScreen::EDITOR_GAME;
	state.playing = EditorRunBar::get_singleton() && EditorRunBar::get_singleton()->is_playing();
	state.plugin_screens = MAX(0, screen_count - BUILTIN_SCREENS);

	updating = true;
	shown = get_visible_tools(state);
	clear_tabs();
	for (const TabEntry &entry : shown) {
		const int screen = _screen_for(entry);
		ERR_CONTINUE(screen >= screen_count);
		// UE4 calls the asset store the Marketplace.
		const String title = entry.tool == TOOL_MARKETPLACE ? TTR("Marketplace") : main_screen->get_plugin_at(screen)->get_plugin_name();
		add_tab(title, main_screen->get_screen_icon(screen));
	}
	updating = false;
	_sync_selection();
}

void StrataDocumentTabs::_sync_selection() {
	const int screen = main_screen->get_selected_index();
	int tab = -1;
	for (int i = 0; i < shown.size(); i++) {
		if (_screen_for(shown[i]) == screen) {
			tab = i;
		}
	}
	updating = true;
	set_current_tab(tab);
	updating = false;

	// While a tool is active, draw the current level tab as unselected instead of deselecting it:
	// EditorNode reads the level TabBar's current tab as the edited scene index.
	TabBar *levels = scene_tabs->get_tab_bar();
	if (tab >= 0) {
		levels->add_theme_style_override(SNAME("tab_selected"), levels->get_theme_stylebox(SNAME("tab_unselected")));
	} else {
		levels->remove_theme_style_override(SNAME("tab_selected"));
	}
}

void StrataDocumentTabs::_show_level() {
	main_screen->select(level_screen(EditorNode::get_singleton()->get_edited_scene()));
}

void StrataDocumentTabs::_screen_changed(int p_index) {
	if (p_index == EditorMainScreen::EDITOR_SCRIPT) {
		state.script_dismissed = false;
	} else if (p_index == EditorMainScreen::EDITOR_ASSETLIB) {
		state.marketplace_open = true;
	}
	_rebuild();
}

void StrataDocumentTabs::_tab_selected(int p_tab) {
	if (updating) {
		return;
	}
	if (p_tab < 0) {
		// Clicking the active tab deselects it; the tool screen is still showing, so select it again.
		_sync_selection();
		return;
	}
	if (p_tab >= shown.size()) {
		return;
	}
	main_screen->select(_screen_for(shown[p_tab]));
}

void StrataDocumentTabs::_tab_closed(int p_tab) {
	ERR_FAIL_INDEX(p_tab, shown.size());
	switch (shown[p_tab].tool) {
		case TOOL_SCRIPT:
			state.script_dismissed = true; // Scripts stay open; the tab comes back when one is opened.
			break;
		case TOOL_MARKETPLACE:
			state.marketplace_open = false;
			break;
		case TOOL_GAME:
			EditorRunBar::get_singleton()->stop_playing(); // Like closing UE4's play window.
			break;
		case TOOL_PLUGIN:
			return; // Plugin screens have no other way in, so their tabs stay.
	}
	if (get_current_tab() == p_tab) {
		_show_level();
	}
	_rebuild();
}

void StrataDocumentTabs::_level_clicked(int p_tab) {
	if (is_tool_screen(main_screen->get_selected_index())) {
		_show_level();
	}
}

void StrataDocumentTabs::_notification(int p_what) {
	if (p_what != NOTIFICATION_READY) {
		return;
	}
	if (ScriptEditor *script_editor = ScriptEditor::get_singleton()) {
		script_editor->connect("editor_script_changed", callable_mp(this, &StrataDocumentTabs::_queue_rebuild).unbind(1));
		script_editor->connect("script_close", callable_mp(this, &StrataDocumentTabs::_queue_rebuild).unbind(1));
	}
	if (EditorRunBar *run_bar = EditorRunBar::get_singleton()) {
		run_bar->connect("play_pressed", callable_mp(this, &StrataDocumentTabs::_queue_rebuild));
		run_bar->connect("stop_pressed", callable_mp(this, &StrataDocumentTabs::_queue_rebuild));
	}
	// Deferred so it runs after EditorNode applies the profile to the main screen buttons.
	EditorFeatureProfileManager::get_singleton()->connect("current_feature_profile_changed", callable_mp(this, &StrataDocumentTabs::_queue_rebuild));
	_queue_rebuild();
}

void StrataDocumentTabs::setup(EditorSceneTabs *p_scene_tabs, EditorMainScreen *p_main_screen) {
	scene_tabs = p_scene_tabs;
	main_screen = p_main_screen;
	set_name("StrataDocumentTabs");
	set_deselect_enabled(true);
	set_tab_close_display_policy(CLOSE_BUTTON_SHOW_ACTIVE_ONLY);
	set_clip_tabs(false); // Show every tool tab, not one with scroll arrows.
	set_auto_translate_mode(AUTO_TRANSLATE_MODE_DISABLED);
	scene_tabs->add_after_tabs(this);

	connect("tab_changed", callable_mp(this, &StrataDocumentTabs::_tab_selected));
	connect("tab_close_pressed", callable_mp(this, &StrataDocumentTabs::_tab_closed));
	main_screen->connect("screen_changed", callable_mp(this, &StrataDocumentTabs::_screen_changed));
	// Plugins adding or removing a main screen (rebuilt deferred, after the screen list settles).
	main_screen->connect("screens_changed", callable_mp(this, &StrataDocumentTabs::_queue_rebuild));
	scene_tabs->get_tab_bar()->connect("tab_clicked", callable_mp(this, &StrataDocumentTabs::_level_clicked));
}
