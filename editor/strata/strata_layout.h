/**************************************************************************/
/*  strata_layout.h                                                       */
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

#include "core/io/config_file.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"

struct StrataDockKeys {
	String outliner;
	String details;
	String import;
	String signals;
	String groups;
	String history;
};

// UE4-style default dock arrangement. Layout keys keep Godot's dock names so saved layouts still load.
namespace StrataLayout {
inline constexpr const char *KEY_OUTLINER = "Scene";
inline constexpr const char *KEY_DETAILS = "Inspector";
inline constexpr const char *KEY_CONTENT_BROWSER = "FileSystem";
inline constexpr const char *KEY_OUTPUT_LOG = "Output";
inline constexpr int RIGHT_COLUMN_WIDTH = 380;

void fill_default_layout(const Ref<ConfigFile> &p_layout, const StrataDockKeys &p_keys, int p_vsplit_count);
String get_bottom_dock_order(const PackedStringArray &p_bottom_docks);
} // namespace StrataLayout
