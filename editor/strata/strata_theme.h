/**************************************************************************/
/*  strata_theme.h                                                        */
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

#include "editor/themes/editor_theme_manager.h"

// Strata's own default look: preset values feeding Godot's theme generator, plus overrides applied after it.
struct StrataThemePreset {
	Color base_color;
	Color accent_color;
	float contrast = 0.0;
	float icon_saturation = 0.0;
	int corner_radius = 0;
	int base_spacing = 0;
	int extra_spacing = 0;
	Size2 dialogs_buttons_min_size;
	int main_font_size = 0;
};

class StrataTheme {
public:
	static StrataThemePreset get_default_preset();
	// Near-black text on light accents, white on dark ones, so primary buttons stay readable with any custom accent.
	static Color get_primary_button_font_color(const Color &p_accent);
	static void populate_overrides(const Ref<EditorTheme> &p_theme, EditorThemeManager::ThemeConfiguration &p_config);
};
