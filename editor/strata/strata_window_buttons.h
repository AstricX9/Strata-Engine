/**************************************************************************/
/*  strata_window_buttons.h                                               */
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
#include "scene/main/window.h"

class Button;

// Minimise / maximise-restore / close for the editor's own title row (UE4 draws its own window chrome).
class StrataWindowButtons : public HBoxContainer {
	GDCLASS(StrataWindowButtons, HBoxContainer);

public:
	enum Action {
		ACTION_MINIMIZE,
		ACTION_MAXIMIZE,
		ACTION_RESTORE,
		ACTION_CLOSE,
	};

private:
	Button *minimize_button = nullptr;
	Button *toggle_button = nullptr;
	Button *close_button = nullptr;

	Button *_make_button(Action p_action, const String &p_tooltip);
	void _pressed(int p_action);
	void _update_toggle();

protected:
	void _notification(int p_what);

public:
	// The middle button maximises a windowed window and restores a maximised one.
	static Action toggle_action_for(Window::Mode p_mode);

	StrataWindowButtons();
};
