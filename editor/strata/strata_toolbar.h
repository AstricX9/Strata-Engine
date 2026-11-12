/**************************************************************************/
/*  strata_toolbar.h                                                      */
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

#include "scene/gui/box_container.h"

class Button;
class EditorRunBar;
class MenuButton;
class PopupMenu;

// UE4's level editor toolbar: large icon-over-label buttons under the menu bar.
class StrataToolbar : public HBoxContainer {
	GDCLASS(StrataToolbar, HBoxContainer);

public:
	enum ButtonId {
		SAVE_CURRENT,
		SOURCE_CONTROL,
		CONTENT,
		MARKETPLACE,
		SETTINGS,
		BLUEPRINTS,
		CINEMATICS,
		BUILD,
		PLAY,
		LAUNCH,
		BUTTON_MAX,
	};

	struct ButtonInfo {
		ButtonId id;
		const char *label;
		const char *icon;
		bool has_dropdown;
	};

private:
	enum MenuItem {
		VCS_METADATA,
		VCS_SETTINGS,
		SETTINGS_PROJECT,
		SETTINGS_EDITOR,
		BLUEPRINTS_OPEN_SCRIPT_EDITOR,
		BLUEPRINTS_ATTACH,
		BLUEPRINTS_EXTEND,
		CINEMATICS_ADD_ANIMATION_PLAYER,
		CINEMATICS_OPEN_ANIMATION,
		BUILD_ALL,
		BUILD_LIGHTING,
		BUILD_NAVIGATION,
		PLAY_CURRENT,
		PLAY_MAIN,
		PLAY_MOVIE_MAKER,
		LAUNCH_EXPORT,
		LAUNCH_PACK_ZIP,
	};

	Button *buttons[BUTTON_MAX] = {};
	MenuButton *play_options = nullptr;

	MenuButton *_get_menu_button(ButtonId p_id) const;
	PopupMenu *_add_menu(ButtonId p_id);
	void _button_pressed(int p_id);
	void _menu_item_pressed(int p_item);
	void _update_menu(int p_id);
	void _update_enabled();
	void _add_animation_player();

protected:
	void _notification(int p_what);

public:
	static Vector<ButtonInfo> get_buttons();
	// Moves Godot's run bar (Pause, Stop, deploy, indicators) right after Play and hides its own play buttons.
	void set_run_bar(EditorRunBar *p_run_bar);

	StrataToolbar();
};
