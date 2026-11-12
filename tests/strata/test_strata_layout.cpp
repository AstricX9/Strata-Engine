/**************************************************************************/
/*  test_strata_layout.cpp                                                */
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

TEST_FORCE_LINK(test_strata_layout)

#ifdef TOOLS_ENABLED

#include "core/io/config_file.h"
#include "editor/strata/strata_layout.h"

namespace TestStrataLayout {

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
	// Closed docks must also be listed in a slot: the loader only closes docks it finds in slot keys.
	CHECK(cfg->get_value("docks", "dock_5") == Variant("Scene,Import,History")); // RIGHT_UL
	CHECK(cfg->get_value("docks", "dock_6") == Variant("Inspector,Signals,Groups")); // RIGHT_BL
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

TEST_CASE("[Strata][Layout] Content Browser starts open with a usable height") {
	Ref<ConfigFile> cfg;
	cfg.instantiate();
	StrataLayout::fill_default_layout(cfg, { "Scene", "Inspector", "Import", "Signals", "Groups", "History" }, 4);
	// Bottom dock order always starts with the Content Browser, so tab 0 is the Content Browser.
	CHECK(int(cfg->get_value("docks", "dock_9_selected_tab_idx", -1)) == 0);
	Dictionary offsets = cfg->get_value("EditorNode", "bottom_panel_offsets", Dictionary());
	CHECK(int(offsets.get("FileSystem", 0)) == -300);
	CHECK(int(offsets.get("Output", 0)) == -270);
	CHECK(int(offsets.get("Audio", 0)) == -450);
}

} // namespace TestStrataLayout

#endif // TOOLS_ENABLED
