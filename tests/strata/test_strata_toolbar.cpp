/**************************************************************************/
/*  test_strata_toolbar.cpp                                               */
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

TEST_FORCE_LINK(test_strata_toolbar)

#ifdef TOOLS_ENABLED

#include "core/config/engine.h"
#include "editor/run/editor_run_bar.h"
#include "editor/strata/strata_toolbar.h"
#include "scene/resources/dpi_texture.h"

namespace TestStrataToolbar {

TEST_CASE("[Strata][Toolbar] Buttons follow UE4's level editor toolbar") {
	const Vector<StrataToolbar::ButtonInfo> buttons = StrataToolbar::get_buttons();
	const char *labels[] = { "Save Current", "Source Control", "Content", "Marketplace", "Settings", "Blueprints", "Cinematics", "Build", "Play", "Launch" };
	const char *icons[] = { "StrataSave", "StrataSourceControl", "StrataContent", "StrataMarketplace", "StrataSettings", "StrataBlueprints", "StrataCinematics", "StrataBuild", "StrataPlay", "StrataLaunch" };
	const bool dropdowns[] = { false, true, false, false, true, true, true, true, true, true };
	REQUIRE(buttons.size() == 10);
	for (int i = 0; i < 10; i++) {
		CHECK(buttons[i].id == i);
		CHECK(String(buttons[i].label) == labels[i]);
		CHECK(String(buttons[i].icon) == icons[i]);
		CHECK(buttons[i].has_dropdown == dropdowns[i]);
	}
}

TEST_CASE("[Strata][Toolbar] Separators split the toolbar into UE4's groups") {
	// Save | Source Control | Content, Marketplace | Settings | Blueprints, Cinematics | Build | Play | Launch
	const bool group_starts[] = { false, true, true, false, true, true, false, true, true, true };
	const Vector<StrataToolbar::ButtonInfo> buttons = StrataToolbar::get_buttons();
	for (int i = 0; i < buttons.size(); i++) {
		CHECK_MESSAGE(buttons[i].group_start == group_starts[i], buttons[i].label);
	}
}

TEST_CASE("[Strata][Toolbar] Toolbar icons are sharp copies at twice the editor icon size") {
	const String svg = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\"><rect width=\"16\" height=\"16\" fill=\"#fff\"/></svg>";
	Ref<DPITexture> icon = DPITexture::create_from_string(svg, 1.0, 0.5);
	Ref<DPITexture> big = StrataToolbar::make_toolbar_icon(icon);
	REQUIRE(big.is_valid());
	CHECK(big != icon); // The shared theme icon is left alone.
	CHECK(big->get_width() == icon->get_width() * StrataToolbar::ICON_SCALE);
	CHECK(big->get_saturation() == doctest::Approx(0.5)); // Theme recolouring is kept.
}

TEST_CASE("[Editor][Strata][Toolbar] Run bar in Recovery Mode can hide its play buttons") {
	// Recovery Mode builds the run bar without play buttons; the toolbar must not crash the editor there.
	Engine::get_singleton()->set_recovery_mode_hint(true);
	EditorRunBar *run_bar = memnew(EditorRunBar);
	run_bar->set_play_buttons_visible(false);
	memdelete(run_bar);
	Engine::get_singleton()->set_recovery_mode_hint(false);
}

} // namespace TestStrataToolbar

#endif // TOOLS_ENABLED
