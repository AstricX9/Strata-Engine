/**************************************************************************/
/*  strata_window_buttons.cpp                                             */
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

#include "strata_window_buttons.h"

#include "core/object/callable_mp.h"
#include "editor/editor_node.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/button.h"
#include "scene/resources/style_box_flat.h"

StrataWindowButtons::Action StrataWindowButtons::toggle_action_for(Window::Mode p_mode) {
	return p_mode == Window::MODE_MAXIMIZED ? ACTION_RESTORE : ACTION_MAXIMIZE;
}

Button *StrataWindowButtons::_make_button(Action p_action, const String &p_tooltip) {
	Button *button = memnew(Button);
	button->set_focus_mode(FOCUS_NONE);
	button->set_tooltip_text(p_tooltip);
	button->set_icon_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	button->set_custom_minimum_size(Size2(46, 0) * EDSCALE); // Windows' caption button width.
	button->set_v_size_flags(SIZE_FILL);
	button->connect(SceneStringName(pressed), callable_mp(this, &StrataWindowButtons::_pressed).bind((int)p_action));
	add_child(button);
	return button;
}

void StrataWindowButtons::_pressed(int p_action) {
	Window *window = get_window();
	ERR_FAIL_NULL(window);
	switch (p_action) {
		case ACTION_MINIMIZE:
			window->set_mode(Window::MODE_MINIMIZED);
			break;
		case ACTION_MAXIMIZE:
			window->set_mode(Window::MODE_MAXIMIZED);
			break;
		case ACTION_RESTORE:
			window->set_mode(Window::MODE_WINDOWED);
			break;
		case ACTION_CLOSE:
			// Same path as Scene > Quit, so unsaved changes are still offered for saving.
			EditorNode::get_singleton()->trigger_menu_option(EditorNode::SCENE_QUIT, false);
			break;
	}
}

void StrataWindowButtons::_update_toggle() {
	Window *window = get_window();
	if (!window || !toggle_button) {
		return;
	}
	const Action action = toggle_action_for(window->get_mode());
	toggle_button->disconnect(SceneStringName(pressed), callable_mp(this, &StrataWindowButtons::_pressed));
	toggle_button->connect(SceneStringName(pressed), callable_mp(this, &StrataWindowButtons::_pressed).bind((int)action));
	toggle_button->set_tooltip_text(action == ACTION_RESTORE ? TTR("Restore Down") : TTR("Maximize"));
	toggle_button->set_button_icon(get_editor_theme_icon(action == ACTION_RESTORE ? SNAME("StrataWindowRestore") : SNAME("StrataWindowMaximize")));
}

void StrataWindowButtons::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			get_window()->connect(SNAME("size_changed"), callable_mp(this, &StrataWindowButtons::_update_toggle));
		} break;
		case NOTIFICATION_EXIT_TREE: {
			get_window()->disconnect(SNAME("size_changed"), callable_mp(this, &StrataWindowButtons::_update_toggle));
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			minimize_button->set_button_icon(get_editor_theme_icon(SNAME("StrataWindowMinimize")));
			close_button->set_button_icon(get_editor_theme_icon(SNAME("StrataWindowClose")));

			// Plain until hovered; close turns red on hover, as on Windows.
			Ref<StyleBoxFlat> plain;
			plain.instantiate();
			plain->set_bg_color(Color(0, 0, 0, 0));
			Ref<StyleBoxFlat> hover = plain->duplicate();
			hover->set_bg_color(Color(1, 1, 1, 0.1));
			Ref<StyleBoxFlat> close_hover = plain->duplicate();
			close_hover->set_bg_color(Color(0.77, 0.17, 0.17));
			for (Button *button : { minimize_button, toggle_button, close_button }) {
				for (const char *state : { "normal", "focus", "disabled" }) {
					button->add_theme_style_override(state, plain);
				}
				button->add_theme_style_override("hover", button == close_button ? close_hover : hover);
				button->add_theme_style_override("pressed", button == close_button ? close_hover : hover);
				button->add_theme_style_override("hover_pressed", button == close_button ? close_hover : hover);
			}
			_update_toggle();
		} break;
	}
}

StrataWindowButtons::StrataWindowButtons() {
	set_name("StrataWindowButtons");
	add_theme_constant_override("separation", 0);
	minimize_button = _make_button(ACTION_MINIMIZE, TTR("Minimize"));
	toggle_button = _make_button(ACTION_MAXIMIZE, TTR("Maximize"));
	close_button = _make_button(ACTION_CLOSE, TTR("Close"));
}
