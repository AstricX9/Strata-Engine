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

#include "editor/editor_main_screen.h"
#include "scene/main/canvas_item.h"

Vector<StrataDocumentTabs::TabEntry> StrataDocumentTabs::get_visible_tools(const ToolState &p_state) {
	Vector<TabEntry> tabs;
	if (p_state.script_enabled && p_state.scripts_open && !p_state.script_dismissed) {
		tabs.push_back({ TOOL_SCRIPT, -1 });
	}
	if (p_state.marketplace_enabled && p_state.marketplace_open) {
		tabs.push_back({ TOOL_MARKETPLACE, -1 });
	}
	if (p_state.playing) {
		tabs.push_back({ TOOL_GAME, -1 });
	}
	for (int i = 0; i < p_state.plugin_screens; i++) {
		tabs.push_back({ TOOL_PLUGIN, i });
	}
	return tabs;
}

int StrataDocumentTabs::level_screen(const Node *p_root) {
	return Object::cast_to<CanvasItem>(p_root) ? EditorMainScreen::EDITOR_2D : EditorMainScreen::EDITOR_3D;
}
