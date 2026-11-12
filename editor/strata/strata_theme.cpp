/**************************************************************************/
/*  strata_theme.cpp                                                      */
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

#include "strata_theme.h"

#include "editor/themes/editor_scale.h"
#include "editor/themes/editor_theme.h"
#include "scene/resources/style_box_flat.h"
#include "scene/scene_string_names.h"

StrataThemePreset StrataTheme::get_default_preset() {
	StrataThemePreset preset;
	preset.base_color = Color(0.118, 0.122, 0.133);
	// Teal: deliberately not UE4's tan/orange nor Godot's blue.
	preset.accent_color = Color(0.16, 0.74, 0.62);
	preset.contrast = 0.25;
	preset.icon_saturation = 2.0;
	preset.corner_radius = 2;
	preset.base_spacing = 2;
	preset.extra_spacing = 2;
	preset.dialogs_buttons_min_size = Size2(90, 26);
	preset.main_font_size = 12;
	return preset;
}

void StrataTheme::populate_overrides(const Ref<EditorTheme> &p_theme, EditorThemeManager::ThemeConfiguration &p_config) {
	// Selected tabs: lighter background, no accent underline.
	for (const StringName type : { StringName("TabContainer"), StringName("TabBar") }) {
		Ref<StyleBoxFlat> tab_selected = p_theme->get_stylebox("tab_selected", type);
		if (tab_selected.is_valid()) {
			tab_selected = tab_selected->duplicate();
			tab_selected->set_border_width(SIDE_TOP, 0);
			tab_selected->set_bg_color(p_config.surface_high_color);
			p_theme->set_stylebox("tab_selected", type, tab_selected);
		}
	}

	// Inspector categories as darker header bars.
	Ref<StyleBoxFlat> category_bg = p_config.base_style->duplicate();
	category_bg->set_bg_color(p_config.surface_lowest_color);
	category_bg->set_content_margin_individual(6 * EDSCALE, 4 * EDSCALE, 6 * EDSCALE, 4 * EDSCALE);
	p_theme->set_stylebox("bg", "EditorInspectorCategory", category_bg);

	// Primary action buttons (Add Component, Add/Import, Play) use the accent.
	p_theme->set_type_variation("StrataPrimaryButton", "Button");
	const struct {
		const char *name;
		Color color;
	} states[] = {
		{ "normal", p_config.accent_color.darkened(0.2) },
		{ "hover", p_config.accent_color },
		{ "pressed", p_config.accent_color.darkened(0.35) },
	};
	for (const auto &state : states) {
		Ref<StyleBoxFlat> style = p_config.button_style->duplicate();
		style->set_bg_color(state.color);
		style->set_corner_radius_all(p_config.corner_radius * EDSCALE);
		p_theme->set_stylebox(state.name, "StrataPrimaryButton", style);
	}
	for (const char *font_state : { "font_color", "font_hover_color", "font_pressed_color", "font_focus_color", "font_hover_pressed_color" }) {
		p_theme->set_color(font_state, "StrataPrimaryButton", p_config.font_dark_background_color);
	}
}
