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

#include "editor/editor_string_names.h"
#include "editor/themes/editor_scale.h"
#include "editor/themes/editor_theme.h"
#include "scene/resources/gradient.h"
#include "scene/resources/gradient_texture.h"
#include "scene/resources/style_box_flat.h"
#include "scene/resources/style_box_texture.h"
#include "scene/scene_string_names.h"

StrataThemePreset StrataTheme::get_default_preset() {
	StrataThemePreset preset;
	// UE4's scheme: neutral grey panels, orange-gold selection, green main actions.
	preset.base_color = Color(0.2, 0.2, 0.2);
	preset.accent_color = Color(0.89, 0.62, 0.11);
	preset.primary_color = Color(0.22, 0.52, 0.14);
	preset.contrast = 0.25;
	preset.icon_saturation = 2.0;
	preset.corner_radius = 2;
	preset.base_spacing = 3;
	preset.extra_spacing = 2;
	preset.dialogs_buttons_min_size = Size2(90, 26);
	preset.main_font_size = 14;
	return preset;
}

Color StrataTheme::get_primary_button_font_color(const Color &p_accent) {
	return p_accent.get_luminance() > 0.5 ? Color(0.08, 0.08, 0.08) : Color(1, 1, 1);
}

Ref<StyleBoxTexture> StrataTheme::make_gradient_style(const Color &p_top, const Color &p_bottom) {
	Ref<Gradient> gradient;
	gradient.instantiate();
	gradient->set_color(0, p_top);
	gradient->set_color(1, p_bottom);

	Ref<GradientTexture2D> texture;
	texture.instantiate();
	texture->set_gradient(gradient);
	texture->set_width(4);
	texture->set_height(64);
	texture->set_fill_from(Vector2(0, 0));
	texture->set_fill_to(Vector2(0, 1));

	Ref<StyleBoxTexture> style;
	style.instantiate();
	style->set_texture(texture);
	return style;
}

// A gradient box with the same content margins as p_like, so swapping it in keeps every control's size.
static Ref<StyleBox> _shaded_like(const Ref<StyleBox> &p_like, const Color &p_top, const Color &p_bottom) {
	Ref<StyleBoxTexture> style = StrataTheme::make_gradient_style(p_top, p_bottom);
	if (p_like.is_valid()) {
		for (const Side side : { SIDE_LEFT, SIDE_TOP, SIDE_RIGHT, SIDE_BOTTOM }) {
			style->set_content_margin(side, p_like->get_content_margin(side));
		}
	}
	return style;
}

static Ref<StyleBox> _existing(const Ref<EditorTheme> &p_theme, const StringName &p_name, const StringName &p_type, const Ref<StyleBox> &p_fallback) {
	return p_theme->has_stylebox(p_name, p_type) ? p_theme->get_stylebox(p_name, p_type) : p_fallback;
}

void StrataTheme::populate_overrides(const Ref<EditorTheme> &p_theme, EditorThemeManager::ThemeConfiguration &p_config) {
	const Color base = p_config.base_color;

	// UE4's depth comes from shading: raised buttons and tabs are lighter at the top.
	for (const StringName type : { StringName("Button"), StringName("OptionButton"), StringName("MenuButton") }) {
		const struct {
			const char *name;
			Color top;
			Color bottom;
		} states[] = {
			{ "normal", base.lightened(0.22), base.lightened(0.02) },
			{ "hover", base.lightened(0.3), base.lightened(0.08) },
			{ "pressed", base.darkened(0.35), base.darkened(0.05) },
		};
		for (const auto &state : states) {
			const Ref<StyleBox> like = _existing(p_theme, state.name, type, p_config.button_style);
			p_theme->set_stylebox(state.name, type, _shaded_like(like, state.top, state.bottom));
		}
	}

	// Selected tabs: raised, no accent underline.
	for (const StringName type : { StringName("TabContainer"), StringName("TabBar") }) {
		const Ref<StyleBox> like = _existing(p_theme, "tab_selected", type, p_config.base_style);
		p_theme->set_stylebox("tab_selected", type, _shaded_like(like, base.lightened(0.22), base));
	}

	// Dark edges around docks and a dark editor background read as UE4's gaps between panels.
	Ref<StyleBoxFlat> dock_panel = _existing(p_theme, SceneStringName(panel), "TabContainer", p_config.tab_container_style);
	dock_panel = (dock_panel.is_valid() ? dock_panel : p_config.base_style)->duplicate();
	dock_panel->set_border_width_all(1);
	dock_panel->set_border_color(base.darkened(0.6));
	p_theme->set_stylebox(SceneStringName(panel), "TabContainer", dock_panel);
	if (p_theme->has_stylebox("Background", EditorStringName(EditorStyles))) {
		Ref<StyleBoxFlat> background = p_theme->get_stylebox("Background", EditorStringName(EditorStyles))->duplicate();
		background->set_bg_color(base.darkened(0.55));
		p_theme->set_stylebox("Background", EditorStringName(EditorStyles), background);
	}

	// Details header bars: a dark gradient, as in UE4.
	if (p_config.draw_extra_borders) {
		// Presets like Black (OLED) rely on this border; without it the bar vanishes into the panel.
		Ref<StyleBoxFlat> category_bg = p_config.base_style->duplicate();
		category_bg->set_bg_color(p_config.surface_lowest_color);
		category_bg->set_content_margin_individual(6 * EDSCALE, 4 * EDSCALE, 6 * EDSCALE, 4 * EDSCALE);
		category_bg->set_border_width_all(1);
		category_bg->set_border_color(p_config.extra_border_color_2);
		p_theme->set_stylebox("bg", "EditorInspectorCategory", category_bg);
	} else {
		Ref<StyleBox> category_bg = make_gradient_style(base.lightened(0.08), base.darkened(0.35));
		category_bg->set_content_margin_individual(6 * EDSCALE, 4 * EDSCALE, 6 * EDSCALE, 4 * EDSCALE);
		p_theme->set_stylebox("bg", "EditorInspectorCategory", category_bg);
	}

	// Level editor toolbar: a raised strip above the viewport.
	p_theme->set_type_variation("StrataToolbar", "PanelContainer");
	Ref<StyleBox> toolbar_panel = make_gradient_style(base.lightened(0.16), base.darkened(0.12));
	toolbar_panel->set_content_margin_individual(4 * EDSCALE, 2 * EDSCALE, 4 * EDSCALE, 2 * EDSCALE);
	p_theme->set_stylebox(SceneStringName(panel), "StrataToolbar", toolbar_panel);

	// Text fields: lighter than their panel and sunken (darker at the top), as in UE4.
	const Ref<StyleBox> field_like = _existing(p_theme, "normal", "LineEdit", p_config.base_style);
	p_theme->set_stylebox("normal", "LineEdit", _shaded_like(field_like, base.lightened(0.1), base.lightened(0.26)));

	// Primary action buttons (Add Component, Add/Import, Open Project) and the Play icon are green, as in UE4.
	const Color primary = get_default_preset().primary_color;
	p_theme->set_color("strata_primary_color", EditorStringName(Editor), primary);
	p_theme->set_type_variation("StrataPrimaryButton", "Button");
	const struct {
		const char *name;
		Color color;
	} states[] = {
		{ "normal", primary.darkened(0.15) },
		{ "hover", primary },
		{ "pressed", primary.darkened(0.3) },
	};
	for (const auto &state : states) {
		Ref<StyleBoxFlat> style = p_config.button_style->duplicate();
		style->set_bg_color(state.color);
		style->set_corner_radius_all(p_config.corner_radius * EDSCALE);
		p_theme->set_stylebox(state.name, "StrataPrimaryButton", style);
	}
	for (const char *font_state : { "font_color", "font_hover_color", "font_pressed_color", "font_focus_color", "font_hover_pressed_color" }) {
		p_theme->set_color(font_state, "StrataPrimaryButton", get_primary_button_font_color(primary));
	}
}
