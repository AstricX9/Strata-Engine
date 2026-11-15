/**************************************************************************/
/*  test_strata_window_buttons.cpp                                        */
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

TEST_FORCE_LINK(test_strata_window_buttons)

#ifdef TOOLS_ENABLED

#include "editor/strata/strata_window_buttons.h"

namespace TestStrataWindowButtons {

TEST_CASE("[Strata][WindowButtons] The middle button maximises or restores") {
	CHECK(StrataWindowButtons::toggle_action_for(Window::MODE_WINDOWED) == StrataWindowButtons::ACTION_MAXIMIZE);
	CHECK(StrataWindowButtons::toggle_action_for(Window::MODE_MAXIMIZED) == StrataWindowButtons::ACTION_RESTORE);
}

TEST_CASE("[Strata][WindowButtons] The title row only drags a windowed or maximised editor") {
	CHECK(StrataWindowButtons::can_drag(Window::MODE_WINDOWED));
	CHECK(StrataWindowButtons::can_drag(Window::MODE_MAXIMIZED));
	CHECK_FALSE(StrataWindowButtons::can_drag(Window::MODE_FULLSCREEN));
	CHECK_FALSE(StrataWindowButtons::can_drag(Window::MODE_EXCLUSIVE_FULLSCREEN));
	CHECK_FALSE(StrataWindowButtons::can_drag(Window::MODE_MINIMIZED));
}

} // namespace TestStrataWindowButtons

#endif // TOOLS_ENABLED
