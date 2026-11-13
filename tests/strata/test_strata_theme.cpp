/**************************************************************************/
/*  test_strata_theme.cpp                                                 */
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

TEST_FORCE_LINK(test_strata_theme)

#ifdef TOOLS_ENABLED

#include "editor/strata/strata_theme.h"
#include "editor/themes/editor_theme.h"
#include "scene/resources/gradient.h"
#include "scene/resources/gradient_texture.h"
#include "scene/resources/style_box_flat.h"
#include "scene/resources/style_box_texture.h"

namespace TestStrataTheme {

TEST_CASE("[Strata][Theme] Default preset is Strata's own") {
	const StrataThemePreset p = StrataTheme::get_default_preset();
	// UE4's scheme: neutral grey panels, orange-gold accent, green primary actions.
	CHECK(p.base_color.is_equal_approx(Color(0.2, 0.2, 0.2)));
	CHECK(p.accent_color.is_equal_approx(Color(0.89, 0.62, 0.11)));
	CHECK(p.primary_color.is_equal_approx(Color(0.22, 0.52, 0.14)));
	CHECK(p.contrast == doctest::Approx(0.25));
	CHECK(p.icon_saturation == doctest::Approx(2.0));
	CHECK(p.corner_radius == 2);
	CHECK(p.base_spacing == 2);
	CHECK(p.extra_spacing == 2);
	CHECK(p.dialogs_buttons_min_size == Size2(90, 26));
	CHECK(p.main_font_size == 12);
}

TEST_CASE("[Strata][Theme] Inspector category bar keeps extra borders (Black OLED preset)") {
	Ref<EditorTheme> theme;
	theme.instantiate();
	EditorThemeManager::ThemeConfiguration config;
	config.base_style.instantiate();
	config.button_style.instantiate();
	config.draw_extra_borders = true;
	config.extra_border_color_2 = Color(1, 0, 0);
	StrataTheme::populate_overrides(theme, config);
	Ref<StyleBoxFlat> bg = theme->get_stylebox("bg", "EditorInspectorCategory");
	REQUIRE(bg.is_valid());
	CHECK(bg->get_border_width(SIDE_TOP) == 1);
	CHECK(bg->get_border_color() == Color(1, 0, 0));
}

TEST_CASE("[Strata][Theme] Primary button text contrasts with the accent") {
	const Color dark_text(0.08, 0.08, 0.08);
	CHECK(StrataTheme::get_primary_button_font_color(Color(1, 0.9, 0.1)) == dark_text); // Yellow.
	CHECK(StrataTheme::get_primary_button_font_color(StrataTheme::get_default_preset().accent_color) == dark_text); // Orange-gold.
	CHECK(StrataTheme::get_primary_button_font_color(StrataTheme::get_default_preset().primary_color) == Color(1, 1, 1)); // Green.
	CHECK(StrataTheme::get_primary_button_font_color(Color(0.1, 0.15, 0.5)) == Color(1, 1, 1)); // Dark blue.
}

static Ref<EditorTheme> make_theme(EditorThemeManager::ThemeConfiguration &r_config) {
	Ref<EditorTheme> theme;
	theme.instantiate();
	r_config.base_style.instantiate();
	r_config.button_style.instantiate();
	r_config.tab_container_style.instantiate();
	r_config.base_color = Color(0.2, 0.2, 0.2);
	r_config.accent_color = Color(0.89, 0.62, 0.11);
	StrataTheme::populate_overrides(theme, r_config);
	return theme;
}

TEST_CASE("[Strata][Theme] Primary buttons are green like UE4, not the accent") {
	EditorThemeManager::ThemeConfiguration config;
	Ref<EditorTheme> theme = make_theme(config);
	Ref<StyleBoxFlat> hover = theme->get_stylebox("hover", "StrataPrimaryButton");
	REQUIRE(hover.is_valid());
	CHECK(hover->get_bg_color().is_equal_approx(StrataTheme::get_default_preset().primary_color));
	CHECK(theme->get_color("font_color", "StrataPrimaryButton") == Color(1, 1, 1));
}

// Top and bottom colours of a Strata gradient style; both transparent black if it is not one.
static Color gradient_color(const Ref<StyleBox> &p_style, int p_point) {
	const Ref<StyleBoxTexture> style = p_style;
	const Ref<GradientTexture2D> texture = style.is_valid() ? style->get_texture() : Ref<Texture2D>();
	return texture.is_valid() ? texture->get_gradient()->get_color(p_point) : Color(0, 0, 0, 0);
}

TEST_CASE("[Strata][Theme] Gradient styles shade from top to bottom") {
	Ref<StyleBoxTexture> style = StrataTheme::make_gradient_style(Color(0.4, 0.4, 0.4), Color(0.2, 0.2, 0.2));
	REQUIRE(style.is_valid());
	const Ref<GradientTexture2D> texture = style->get_texture();
	REQUIRE(texture.is_valid());
	CHECK(texture->get_fill_to().y > texture->get_fill_from().y); // Vertical.
	CHECK(gradient_color(style, 0) == Color(0.4, 0.4, 0.4));
	CHECK(gradient_color(style, 1) == Color(0.2, 0.2, 0.2));
}

TEST_CASE("[Strata][Theme] Buttons and selected tabs are raised like UE4") {
	EditorThemeManager::ThemeConfiguration config;
	Ref<EditorTheme> theme = make_theme(config);
	for (const char *type : { "Button", "OptionButton", "MenuButton" }) {
		const Ref<StyleBox> normal = theme->get_stylebox("normal", type);
		CHECK_MESSAGE(gradient_color(normal, 0).get_luminance() > gradient_color(normal, 1).get_luminance(), type);
	}
	const Ref<StyleBox> tab = theme->get_stylebox("tab_selected", "TabContainer");
	CHECK(gradient_color(tab, 0).get_luminance() > gradient_color(tab, 1).get_luminance());
}

TEST_CASE("[Strata][Theme] Text fields are sunken and lighter than the panels") {
	EditorThemeManager::ThemeConfiguration config;
	Ref<EditorTheme> theme = make_theme(config);
	const Ref<StyleBox> field = theme->get_stylebox("normal", "LineEdit");
	CHECK(gradient_color(field, 0).get_luminance() < gradient_color(field, 1).get_luminance());
	CHECK(gradient_color(field, 1).get_luminance() > config.base_color.get_luminance() + 0.1);
}

TEST_CASE("[Strata][Theme] Docks have dark edges so panels separate like UE4") {
	EditorThemeManager::ThemeConfiguration config;
	Ref<EditorTheme> theme = make_theme(config);
	Ref<StyleBoxFlat> panel = theme->get_stylebox("panel", "TabContainer");
	REQUIRE(panel.is_valid());
	CHECK(panel->get_border_width(SIDE_TOP) >= 1);
	CHECK(panel->get_border_color().get_luminance() < config.base_color.get_luminance() * 0.6);
}

TEST_CASE("[Strata][Theme] Details header bars have a gradient") {
	EditorThemeManager::ThemeConfiguration config;
	Ref<EditorTheme> theme = make_theme(config);
	const Ref<StyleBox> header = theme->get_stylebox("bg", "EditorInspectorCategory");
	CHECK(gradient_color(header, 0).get_luminance() > gradient_color(header, 1).get_luminance());
}

} // namespace TestStrataTheme

#endif // TOOLS_ENABLED
