/**************************************************************************/
/*  test_strata_build.cpp                                                 */
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

TEST_FORCE_LINK(test_strata_build)

#ifdef TOOLS_ENABLED

#include "editor/strata/strata_build.h"
#include "scene/3d/lightmap_gi.h"
#include "scene/3d/navigation/navigation_region_3d.h"
#include "scene/3d/voxel_gi.h"

namespace TestStrataBuild {

TEST_CASE("[Strata][Build] No level gives no bake targets") {
	const StrataBakeTargets targets = StrataBuild::collect_targets(nullptr);
	CHECK(targets.lightmaps.is_empty());
	CHECK(targets.voxel_gis.is_empty());
	CHECK(targets.nav_regions.is_empty());
}

TEST_CASE("[SceneTree][Strata][Build] Bake targets are found anywhere in the level") {
	Node3D *root = memnew(Node3D);
	VoxelGI *voxel_gi = memnew(VoxelGI);
	root->add_child(voxel_gi);
	Node3D *group = memnew(Node3D);
	root->add_child(group);
	NavigationRegion3D *nav = memnew(NavigationRegion3D);
	group->add_child(nav);
	LightmapGI *lightmap = memnew(LightmapGI);
	root->add_child(lightmap);

	const StrataBakeTargets targets = StrataBuild::collect_targets(root);
	REQUIRE(targets.voxel_gis.size() == 1);
	REQUIRE(targets.nav_regions.size() == 1);
	REQUIRE(targets.lightmaps.size() == 1);
	CHECK(targets.voxel_gis[0] == voxel_gi);
	CHECK(targets.nav_regions[0] == nav);
	CHECK(targets.lightmaps[0] == lightmap);

	memdelete(root);
}

TEST_CASE("[SceneTree][Strata][Build] Lightmap save path follows the level file") {
	LightmapGI *lightmap = memnew(LightmapGI);
	CHECK(StrataBuild::lightmap_save_path(lightmap, "res://levels/arena.tscn") == "res://levels/arena.lmbake");
	memdelete(lightmap);
}

TEST_CASE("[SceneTree][Strata][Build] Unsaved level has no lightmap save path") {
	LightmapGI *lightmap = memnew(LightmapGI);
	CHECK(StrataBuild::lightmap_save_path(lightmap, "") == "");
	memdelete(lightmap);
}

} // namespace TestStrataBuild

#endif // TOOLS_ENABLED
