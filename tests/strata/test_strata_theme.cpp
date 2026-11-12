/**************************************************************************/
/*  test_strata_theme.cpp                                                 */
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

TEST_FORCE_LINK(test_strata_theme)

#ifdef TOOLS_ENABLED

#include "editor/strata/strata_theme.h"

namespace TestStrataTheme {

TEST_CASE("[Strata][Theme] Default preset is Strata's own") {
	const StrataThemePreset p = StrataTheme::get_default_preset();
	CHECK(p.base_color.is_equal_approx(Color(0.118, 0.122, 0.133)));
	CHECK(p.accent_color.is_equal_approx(Color(0.16, 0.74, 0.62)));
	CHECK(p.contrast == doctest::Approx(0.25));
	CHECK(p.icon_saturation == doctest::Approx(2.0));
	CHECK(p.corner_radius == 2);
	CHECK(p.base_spacing == 2);
	CHECK(p.extra_spacing == 2);
	CHECK(p.dialogs_buttons_min_size == Size2(90, 26));
	CHECK(p.main_font_size == 12);
}

} // namespace TestStrataTheme

#endif // TOOLS_ENABLED
