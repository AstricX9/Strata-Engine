/**************************************************************************/
/*  test_strata_window_frame.cpp                                          */
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

TEST_FORCE_LINK(test_strata_window_frame)

#ifdef WINDOWS_ENABLED

#include "platform/windows/strata_window_frame.h"

namespace TestStrataWindowFrame {

TEST_CASE("[Strata][WindowFrame] The top edge still resizes the window") {
	const Vector2i size(1000, 800);
	CHECK(StrataWindowFrame::hit_test(Vector2i(500, 2), size, 6, false) == StrataWindowFrame::HIT_TOP);
	CHECK(StrataWindowFrame::hit_test(Vector2i(2, 2), size, 6, false) == StrataWindowFrame::HIT_TOP_LEFT);
	CHECK(StrataWindowFrame::hit_test(Vector2i(997, 2), size, 6, false) == StrataWindowFrame::HIT_TOP_RIGHT);
	CHECK(StrataWindowFrame::hit_test(Vector2i(500, 20), size, 6, false) == StrataWindowFrame::HIT_CLIENT);
	// A maximised window cannot be resized.
	CHECK(StrataWindowFrame::hit_test(Vector2i(500, 2), size, 6, true) == StrataWindowFrame::HIT_CLIENT);
}

TEST_CASE("[Strata][WindowFrame] The caption is removed and the other frame edges kept") {
	CHECK(StrataWindowFrame::client_rect(Rect2i(0, 0, 1000, 800), 8, false) == Rect2i(8, 0, 984, 792));
}

TEST_CASE("[Strata][WindowFrame] A maximised window does not hang off the screen") {
	// Windows places a maximised window's frame outside the monitor; the client rect must stay on it.
	CHECK(StrataWindowFrame::client_rect(Rect2i(-8, -8, 1936, 1056), 8, true) == Rect2i(0, 0, 1920, 1040));
}

} // namespace TestStrataWindowFrame

#endif // WINDOWS_ENABLED
