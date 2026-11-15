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

// Monitor edges that have an auto-hide taskbar (bitmask).
enum Edge {
	EDGE_LEFT = 1,
	EDGE_TOP = 2,
	EDGE_RIGHT = 4,
	EDGE_BOTTOM = 8,
};

// The client rect for a window rect: the native caption (and top frame) removed, side and bottom frame kept.
// A maximised window's frame lies outside the monitor, so it is inset on every side. It also leaves one pixel
// free on any edge with an auto-hide taskbar; covering the whole monitor makes Windows treat it as fullscreen.
inline Rect2i client_rect(const Rect2i &p_window_rect, int p_frame, bool p_maximized, int p_autohide_edges = 0) {
	if (!p_maximized) {
		return Rect2i(p_window_rect.position.x + p_frame, p_window_rect.position.y, p_window_rect.size.x - p_frame * 2, p_window_rect.size.y - p_frame);
	}
	Rect2i r = p_window_rect.grow(-p_frame);
	if (p_autohide_edges & EDGE_LEFT) {
		r.position.x += 1;
		r.size.x -= 1;
	}
	if (p_autohide_edges & EDGE_TOP) {
		r.position.y += 1;
		r.size.y -= 1;
	}
	if (p_autohide_edges & EDGE_RIGHT) {
		r.size.x -= 1;
	}
	if (p_autohide_edges & EDGE_BOTTOM) {
		r.size.y -= 1;
	}
	return r;
}

// The (non-maximised) window rect whose client rect is p_client: the inverse of client_rect.
inline Rect2i window_rect_for_client(const Rect2i &p_client, int p_frame) {
	return Rect2i(p_client.position.x - p_frame, p_client.position.y, p_client.size.x + p_frame * 2, p_client.size.y + p_frame);
}

} // namespace StrataWindowFrame
