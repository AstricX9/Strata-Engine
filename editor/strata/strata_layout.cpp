/**************************************************************************/
/*  strata_layout.cpp                                                     */
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

#include "strata_layout.h"

#include "core/variant/array.h"
#include "core/variant/dictionary.h"

void StrataLayout::fill_default_layout(const Ref<ConfigFile> &p_layout, const StrataDockKeys &p_keys, int p_vsplit_count) {
	const String section = "docks";
	// Dock numbers are DockSlot enum value + 1: 5 = RIGHT_UL, 6 = RIGHT_BL.
	// Closed docks are listed too (where they reopen): the loader only closes docks it finds in a slot key.
	p_layout->set_value(section, "dock_5", String(",").join({ p_keys.outliner, p_keys.import, p_keys.history }));
	p_layout->set_value(section, "dock_5_selected_tab_idx", 0);
	p_layout->set_value(section, "dock_6", String(",").join({ p_keys.details, p_keys.signals, p_keys.groups }));
	p_layout->set_value(section, "dock_6_selected_tab_idx", 0);

	Array closed;
	closed.push_back(p_keys.import);
	closed.push_back(p_keys.signals);
	closed.push_back(p_keys.groups);
	closed.push_back(p_keys.history);
	p_layout->set_value(section, "dock_closed", closed);

	// The Content Browser leads the bottom dock order (see get_bottom_dock_order), so open tab 0 like UE4.
	p_layout->set_value(section, "dock_9_selected_tab_idx", 0);
	Dictionary offsets;
	offsets[KEY_CONTENT_BROWSER] = -300;
	offsets[KEY_OUTPUT_LOG] = -270;
	offsets["Audio"] = -450;
	p_layout->set_value("EditorNode", "bottom_panel_offsets", offsets);
	// UE4's Content Browser is a grid of thumbnails (0 = FileSystemDock::FILE_LIST_DISPLAY_THUMBNAILS).
	p_layout->set_value(String("docks/") + KEY_CONTENT_BROWSER, "file_list_display_mode", 0);

	const int hsplits[] = { 0, 0, -RIGHT_COLUMN_WIDTH, 0 };
	for (int i = 0; i < 4; i++) {
		p_layout->set_value(section, "dock_hsplit_" + itos(i + 1), hsplits[i]);
	}
	for (int i = 0; i < p_vsplit_count; i++) {
		p_layout->set_value(section, "dock_split_" + itos(i + 1), 0);
	}
}

String StrataLayout::get_bottom_dock_order(const PackedStringArray &p_bottom_docks) {
	PackedStringArray ordered;
	for (const char *first : { KEY_CONTENT_BROWSER, KEY_OUTPUT_LOG }) {
		if (p_bottom_docks.has(first)) {
			ordered.push_back(first);
		}
	}
	for (const String &key : p_bottom_docks) {
		if (!ordered.has(key)) {
			ordered.push_back(key);
		}
	}
	return String(",").join(ordered);
}

bool StrataLayout::needs_strata_layout(const Ref<ConfigFile> &p_layout) {
	return int(p_layout->get_value("docks", LAYOUT_VERSION_KEY, 0)) < LAYOUT_VERSION;
}

void StrataLayout::mark_strata_layout(const Ref<ConfigFile> &p_layout) {
	p_layout->set_value("docks", LAYOUT_VERSION_KEY, LAYOUT_VERSION);
}
