/**************************************************************************/
/*  strata_build.cpp                                                      */
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

#include "strata_build.h"

#include "core/os/os.h"
#include "editor/editor_node.h"
#include "scene/3d/lightmap_gi.h"
#include "scene/3d/navigation/navigation_region_3d.h"
#include "scene/3d/voxel_gi.h"

static void _collect(Node *p_node, StrataBakeTargets &r_targets) {
	if (LightmapGI *lightmap = Object::cast_to<LightmapGI>(p_node)) {
		r_targets.lightmaps.push_back(lightmap);
	} else if (VoxelGI *voxel_gi = Object::cast_to<VoxelGI>(p_node)) {
		r_targets.voxel_gis.push_back(voxel_gi);
	} else if (NavigationRegion3D *nav = Object::cast_to<NavigationRegion3D>(p_node)) {
		r_targets.nav_regions.push_back(nav);
	}
	for (int i = 0; i < p_node->get_child_count(); i++) {
		_collect(p_node->get_child(i), r_targets);
	}
}

StrataBakeTargets StrataBuild::collect_targets(Node *p_level_root) {
	StrataBakeTargets targets;
	if (p_level_root) {
		_collect(p_level_root, targets);
	}
	return targets;
}

String StrataBuild::lightmap_save_path(const LightmapGI *p_lightmap, const String &p_level_path) {
	const Ref<LightmapGIData> data = p_lightmap->get_light_data();
	if (data.is_valid() && data->get_path().is_resource_file()) {
		return data->get_path();
	}
	return p_level_path.is_empty() ? String() : p_level_path.get_basename() + ".lmbake";
}

static EditorProgress *lightmap_progress = nullptr;

static bool _lightmap_bake_step(float p_progress, const String &p_description, void *, bool p_refresh) {
	if (!lightmap_progress) {
		lightmap_progress = memnew(EditorProgress("bake_lightmaps", TTR("Bake Lightmaps"), 1000, true));
	}
	return lightmap_progress->step(p_description, p_progress * 1000, p_refresh);
}

static void _report_failure(Node *p_node, const String &p_reason) {
	ERR_PRINT(vformat(TTR("Build: %s failed (%s)."), String(p_node->get_path()), p_reason));
}

void StrataBuild::build_all(Node *p_level_root, bool p_lighting, bool p_navigation) {
	const StrataBakeTargets targets = collect_targets(p_level_root);
	if (p_lighting) {
		for (VoxelGI *voxel_gi : targets.voxel_gis) {
			voxel_gi->bake();
		}
		for (LightmapGI *lightmap : targets.lightmaps) {
			const String path = lightmap_save_path(lightmap, p_level_root->get_scene_file_path());
			if (path.is_empty()) {
				_report_failure(lightmap, TTR("save the level first"));
				continue;
			}
			// Same bake root rule as LightmapGIEditorPlugin.
			Node *from = lightmap == p_level_root ? lightmap : lightmap->get_parent();
			const LightmapGI::BakeError err = lightmap->bake(from, path, _lightmap_bake_step);
			if (lightmap_progress) {
				memdelete(lightmap_progress);
				lightmap_progress = nullptr;
			}
			if (err != LightmapGI::BAKE_ERROR_OK) {
				_report_failure(lightmap, vformat(TTR("lightmap error %d"), int(err)));
			}
		}
	}
	if (p_navigation) {
		for (NavigationRegion3D *nav : targets.nav_regions) {
			if (nav->get_navigation_mesh().is_null()) {
				_report_failure(nav, TTR("no NavigationMesh resource"));
				continue;
			}
			nav->bake_navigation_mesh(false);
		}
	}
}
