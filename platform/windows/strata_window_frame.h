/**************************************************************************/
/*  strata_window_frame.h                                                 */
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

#include "core/math/rect2i.h"
#include "core/math/vector2i.h"

// Strata: window frame maths for "extend to title" on Windows, kept free of Win32 so it can be unit-tested.
// The editor draws its own title row; Windows keeps the side and bottom frame (native resize, shadow, snap).
namespace StrataWindowFrame {

enum Hit {
	HIT_CLIENT,
	HIT_TOP,
	HIT_TOP_LEFT,
	HIT_TOP_RIGHT,
};

// p_point is in window coordinates (0,0 = top-left of the window rect); p_border is the DPI-scaled resize border.
// The top edge has no native frame left, so it is resized from here; the other edges stay native.
inline Hit hit_test(const Vector2i &p_point, const Vector2i &p_window_size, int p_border, bool p_maximized) {
	if (p_maximized || p_point.y >= p_border) {
		return HIT_CLIENT;
	}
	if (p_point.x < p_border) {
		return HIT_TOP_LEFT;
	}
	if (p_point.x >= p_window_size.x - p_border) {
		return HIT_TOP_RIGHT;
	}
	return HIT_TOP;
}

// The client rect for a window rect: the native caption (and top frame) removed, side and bottom frame kept.
// A maximised window's frame lies outside the monitor, so it is inset on every side.
inline Rect2i client_rect(const Rect2i &p_window_rect, int p_frame, bool p_maximized) {
	if (p_maximized) {
		return p_window_rect.grow(-p_frame);
	}
	return Rect2i(p_window_rect.position.x + p_frame, p_window_rect.position.y, p_window_rect.size.x - p_frame * 2, p_window_rect.size.y - p_frame);
}

} // namespace StrataWindowFrame
