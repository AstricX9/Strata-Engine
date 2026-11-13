/**************************************************************************/
/*  test_strata_document_tabs.cpp                                         */
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

#include "tests/test_macros.h"

TEST_FORCE_LINK(test_strata_document_tabs)

#ifdef TOOLS_ENABLED

#include "editor/editor_main_screen.h"
#include "editor/strata/strata_document_tabs.h"
#include "scene/2d/node_2d.h"
#include "scene/3d/node_3d.h"
#include "scene/gui/control.h"

namespace TestStrataDocumentTabs {

using Tabs = StrataDocumentTabs;

TEST_CASE("[Strata][DocumentTabs] Nothing open means no tool tabs") {
	CHECK(Tabs::get_visible_tools(Tabs::ToolState()).is_empty());
}

TEST_CASE("[Strata][DocumentTabs] Tool tabs follow levels in UE4 order") {
	Tabs::ToolState state;
	state.scripts_open = true;
	state.marketplace_open = true;
	state.playing = true;
	state.plugin_screens = 2;
	const Vector<Tabs::TabEntry> tabs = Tabs::get_visible_tools(state);
	REQUIRE(tabs.size() == 5);
	CHECK(tabs[0].tool == Tabs::TOOL_SCRIPT);
	CHECK(tabs[1].tool == Tabs::TOOL_MARKETPLACE);
	CHECK(tabs[2].tool == Tabs::TOOL_GAME);
	CHECK(tabs[3].tool == Tabs::TOOL_PLUGIN);
	CHECK(tabs[3].plugin_index == 0);
	CHECK(tabs[4].plugin_index == 1);
	CHECK(tabs[0].plugin_index == -1);
}

TEST_CASE("[Strata][DocumentTabs] Dismissed or disabled tools stay hidden") {
	Tabs::ToolState state;
	state.scripts_open = true;
	state.script_dismissed = true;
	CHECK(Tabs::get_visible_tools(state).is_empty());

	state.script_dismissed = false;
	state.script_enabled = false; // Feature profile.
	CHECK(Tabs::get_visible_tools(state).is_empty());

	Tabs::ToolState market;
	market.marketplace_open = true;
	market.marketplace_enabled = false;
	CHECK(Tabs::get_visible_tools(market).is_empty());
}

TEST_CASE("[Strata][DocumentTabs] Game tab shows while playing or on the Game screen, unless disabled") {
	Tabs::ToolState state;
	state.game_selected = true; // Ctrl+F4 without playing.
	REQUIRE(Tabs::get_visible_tools(state).size() == 1);
	CHECK(Tabs::get_visible_tools(state)[0].tool == Tabs::TOOL_GAME);

	state.game_selected = false;
	state.playing = true;
	state.game_enabled = false; // Feature profile, or recovery mode where there is no Game screen.
	CHECK(Tabs::get_visible_tools(state).is_empty());
}

TEST_CASE("[Strata][DocumentTabs] Only 2D and 3D are level screens") {
	CHECK_FALSE(Tabs::is_tool_screen(EditorMainScreen::EDITOR_2D));
	CHECK_FALSE(Tabs::is_tool_screen(EditorMainScreen::EDITOR_3D));
	CHECK(Tabs::is_tool_screen(EditorMainScreen::EDITOR_SCRIPT));
	CHECK(Tabs::is_tool_screen(EditorMainScreen::EDITOR_GAME));
	CHECK(Tabs::is_tool_screen(EditorMainScreen::EDITOR_ASSETLIB + 2)); // A plugin screen.
}

TEST_CASE("[SceneTree][Strata][DocumentTabs] Levels open in the 2D or 3D editor by their root") {
	Node3D *node_3d = memnew(Node3D);
	Node2D *node_2d = memnew(Node2D);
	Control *control = memnew(Control);
	Node *node = memnew(Node);
	CHECK(Tabs::level_screen(node_3d) == EditorMainScreen::EDITOR_3D);
	CHECK(Tabs::level_screen(node_2d) == EditorMainScreen::EDITOR_2D);
	CHECK(Tabs::level_screen(control) == EditorMainScreen::EDITOR_2D);
	CHECK(Tabs::level_screen(node) == EditorMainScreen::EDITOR_3D);
	CHECK(Tabs::level_screen(nullptr) == EditorMainScreen::EDITOR_3D);
	memdelete(node_3d);
	memdelete(node_2d);
	memdelete(control);
	memdelete(node);
}

} // namespace TestStrataDocumentTabs

#endif // TOOLS_ENABLED
