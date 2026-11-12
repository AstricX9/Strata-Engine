/**************************************************************************/
/*  strata_toolbar.cpp                                                    */
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

#include "strata_toolbar.h"

#include "core/config/engine.h"
#include "core/object/callable_mp.h"
#include "editor/animation/animation_player_editor_plugin.h"
#include "editor/docks/editor_dock_manager.h"
#include "editor/docks/filesystem_dock.h"
#include "editor/docks/scene_tree_dock.h"
#include "editor/editor_main_screen.h"
#include "editor/editor_node.h"
#include "editor/editor_string_names.h"
#include "editor/editor_undo_redo_manager.h"
#include "editor/run/editor_run_bar.h"
#include "editor/settings/editor_settings.h"
#include "editor/strata/strata_build.h"
#include "editor/themes/editor_scale.h"
#include "editor/version_control/version_control_editor_plugin.h"
#include "scene/animation/animation_player.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/separator.h"
#include "scene/resources/dpi_texture.h"

// Toolbar dropdown: draws a small arrow beside the icon, as UE4 does, instead of a "▾" in the label.
class StrataToolbarMenuButton : public MenuButton {
	GDCLASS(StrataToolbarMenuButton, MenuButton);

protected:
	void _notification(int p_what) {
		if (p_what != NOTIFICATION_DRAW || get_button_icon().is_null()) {
			return;
		}
		const Ref<Texture2D> arrow = get_editor_theme_icon(SNAME("GuiDropdown"));
		const Ref<Texture2D> icon = get_button_icon();
		const Ref<StyleBox> style = get_theme_stylebox(SNAME("normal"));
		const Point2 icon_end((get_size().x + icon->get_width()) / 2, style->get_margin(SIDE_TOP) + icon->get_height());
		draw_texture(arrow, icon_end - Point2(0, arrow->get_height()), get_theme_color(SNAME("font_color")));
	}
};

Ref<DPITexture> StrataToolbar::make_toolbar_icon(const Ref<Texture2D> &p_icon) {
	const Ref<DPITexture> source = p_icon;
	if (source.is_null()) {
		return Ref<DPITexture>();
	}
	Ref<DPITexture> icon = source->duplicate();
	icon->set_base_scale(source->get_base_scale() * ICON_SCALE);
	return icon;
}

Vector<StrataToolbar::ButtonInfo> StrataToolbar::get_buttons() {
	return {
		{ SAVE_CURRENT, TTRC("Save Current"), "Save", false, false },
		{ SOURCE_CONTROL, TTRC("Source Control"), "VcsBranches", true, true },
		{ CONTENT, TTRC("Content"), "Folder", false, true },
		{ MARKETPLACE, TTRC("Marketplace"), "AssetStore", false, false },
		{ SETTINGS, TTRC("Settings"), "Tools", true, true },
		{ BLUEPRINTS, TTRC("Blueprints"), "Script", true, true },
		{ CINEMATICS, TTRC("Cinematics"), "Animation", true, false },
		{ BUILD, TTRC("Build"), "Bake", true, true },
		{ PLAY, TTRC("Play"), "Play", true, true },
		{ LAUNCH, TTRC("Launch"), "PlayRemote", true, true },
	};
}

MenuButton *StrataToolbar::_get_menu_button(ButtonId p_id) const {
	// Play is a plain button with a separate ▾ options button beside it.
	return p_id == PLAY ? play_options : Object::cast_to<MenuButton>(buttons[p_id]);
}

PopupMenu *StrataToolbar::_add_menu(ButtonId p_id) {
	PopupMenu *popup = _get_menu_button(ButtonId(p_id))->get_popup();
	popup->connect(SceneStringName(id_pressed), callable_mp(this, &StrataToolbar::_menu_item_pressed));
	popup->connect("about_to_popup", callable_mp(this, &StrataToolbar::_update_menu).bind(p_id));
	return popup;
}

void StrataToolbar::_button_pressed(int p_id) {
	switch (p_id) {
		case SAVE_CURRENT:
			EditorNode::get_singleton()->trigger_menu_option(EditorNode::SCENE_SAVE_SCENE, false);
			break;
		case CONTENT:
			EditorDockManager::get_singleton()->focus_dock(FileSystemDock::get_singleton());
			break;
		case MARKETPLACE:
			EditorNode::get_editor_main_screen()->select(EditorMainScreen::EDITOR_ASSETLIB);
			break;
		case PLAY:
			EditorRunBar::get_singleton()->play_current_scene();
			break;
		default:
			break;
	}
}

void StrataToolbar::_add_animation_player() {
	Node *root = EditorNode::get_singleton()->get_edited_scene();
	ERR_FAIL_NULL(root);
	AnimationPlayer *player = memnew(AnimationPlayer);
	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	undo_redo->create_action(TTR("Add AnimationPlayer to Level"));
	undo_redo->add_do_method(root, "add_child", player, true);
	undo_redo->add_do_method(player, "set_owner", root);
	undo_redo->add_do_reference(player);
	undo_redo->add_undo_method(root, "remove_child", player);
	undo_redo->commit_action();

	EditorSelection *selection = EditorNode::get_singleton()->get_editor_selection();
	selection->clear();
	selection->add_node(player);
}

void StrataToolbar::_menu_item_pressed(int p_item) {
	EditorNode *editor = EditorNode::get_singleton();
	switch (p_item) {
		case VCS_METADATA:
			VersionControlEditorPlugin::get_singleton()->popup_vcs_metadata_dialog();
			break;
		case VCS_SETTINGS:
			VersionControlEditorPlugin::get_singleton()->popup_vcs_set_up_dialog(editor->get_gui_base());
			break;
		case SETTINGS_PROJECT:
			editor->trigger_menu_option(EditorNode::PROJECT_OPEN_SETTINGS, false);
			break;
		case SETTINGS_EDITOR:
			editor->trigger_menu_option(EditorNode::EDITOR_OPEN_SETTINGS, false);
			break;
		case BLUEPRINTS_OPEN_SCRIPT_EDITOR:
			EditorNode::get_editor_main_screen()->select(EditorMainScreen::EDITOR_SCRIPT);
			break;
		case BLUEPRINTS_ATTACH:
		case BLUEPRINTS_EXTEND:
			SceneTreeDock::get_singleton()->attach_script_to_selected(p_item == BLUEPRINTS_EXTEND);
			break;
		case CINEMATICS_ADD_ANIMATION_PLAYER:
			_add_animation_player();
			break;
		case CINEMATICS_OPEN_ANIMATION:
			EditorDockManager::get_singleton()->focus_dock(AnimationPlayerEditor::get_singleton());
			break;
		case BUILD_ALL:
		case BUILD_LIGHTING:
		case BUILD_NAVIGATION:
			StrataBuild::build_all(editor->get_edited_scene(), p_item != BUILD_NAVIGATION, p_item != BUILD_LIGHTING);
			break;
		case PLAY_CURRENT:
			EditorRunBar::get_singleton()->play_current_scene();
			break;
		case PLAY_MAIN:
			EditorRunBar::get_singleton()->play_main_scene();
			break;
		case PLAY_MOVIE_MAKER: {
			EditorRunBar *run_bar = EditorRunBar::get_singleton();
			run_bar->set_movie_maker_enabled(!run_bar->is_movie_maker_enabled());
		} break;
		case LAUNCH_EXPORT:
			editor->trigger_menu_option(EditorNode::PROJECT_EXPORT, false);
			break;
		case LAUNCH_PACK_ZIP:
			editor->trigger_menu_option(EditorNode::PROJECT_PACK_AS_ZIP, false);
			break;
	}
}

void StrataToolbar::_update_menu(int p_id) {
	EditorNode *editor = EditorNode::get_singleton();
	PopupMenu *popup = _get_menu_button(ButtonId(p_id))->get_popup();
	switch (p_id) {
		case BLUEPRINTS: {
			const bool has_selection = !editor->get_editor_selection()->get_top_selected_node_list().is_empty();
			popup->set_item_disabled(popup->get_item_index(BLUEPRINTS_ATTACH), !has_selection);
			popup->set_item_disabled(popup->get_item_index(BLUEPRINTS_EXTEND), !has_selection);
		} break;
		case PLAY:
			popup->set_item_checked(popup->get_item_index(PLAY_MOVIE_MAKER), EditorRunBar::get_singleton()->is_movie_maker_enabled());
			break;
		default:
			break;
	}
}

void StrataToolbar::_update_enabled() {
	const bool has_level = EditorNode::get_singleton()->get_edited_scene() != nullptr;
	for (ButtonId id : { SAVE_CURRENT, CINEMATICS, BUILD }) {
		buttons[id]->set_disabled(!has_level);
	}
	buttons[PLAY]->set_disabled(!has_level || Engine::get_singleton()->is_recovery_mode_hint());
	buttons[MARKETPLACE]->set_disabled(!EditorNode::get_editor_main_screen()->is_button_enabled(EditorMainScreen::EDITOR_ASSETLIB));
}

void StrataToolbar::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			EditorNode::get_singleton()->connect("scene_changed", callable_mp(this, &StrataToolbar::_update_enabled));
			// Creating or deleting the level root goes through undo/redo without scene_changed.
			EditorUndoRedoManager::get_singleton()->connect("version_changed", callable_mp(this, &StrataToolbar::_update_enabled));
			_update_enabled();
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			for (const ButtonInfo &info : get_buttons()) {
				const Ref<Texture2D> icon = get_editor_theme_icon(StringName(info.icon));
				const Ref<DPITexture> big_icon = make_toolbar_icon(icon);
				buttons[info.id]->set_button_icon(big_icon.is_valid() ? Ref<Texture2D>(big_icon) : icon);
			}
			// Play stands out by an accent-coloured icon, not a filled button.
			const Color accent = get_theme_color(SNAME("accent_color"), EditorStringName(Editor));
			for (const char *state : { "icon_normal_color", "icon_hover_color", "icon_pressed_color", "icon_focus_color" }) {
				buttons[PLAY]->add_theme_color_override(state, accent);
			}
			play_options->set_button_icon(get_editor_theme_icon(SNAME("GuiDropdown")));
		} break;
	}
}

void StrataToolbar::set_run_bar(EditorRunBar *p_run_bar) {
	if (p_run_bar->get_parent()) {
		p_run_bar->reparent(row);
	} else {
		row->add_child(p_run_bar);
	}
	row->move_child(p_run_bar, play_options->get_index() + 1);
	p_run_bar->set_v_size_flags(SIZE_SHRINK_CENTER);
	p_run_bar->set_play_buttons_visible(false);
}

StrataToolbar::StrataToolbar() {
	set_name("StrataToolbar");
	set_theme_type_variation("StrataToolbar");
	row = memnew(HBoxContainer);
	row->add_theme_constant_override("separation", 2 * EDSCALE);
	add_child(row);

	for (const ButtonInfo &info : get_buttons()) {
		if (info.group_start) {
			row->add_child(memnew(VSeparator));
		}
		const bool is_menu = info.has_dropdown && info.id != PLAY;
		Button *button = is_menu ? memnew(StrataToolbarMenuButton) : memnew(Button);
		button->set_text(TTR(info.label));
		button->set_auto_translate_mode(AUTO_TRANSLATE_MODE_DISABLED);
		button->set_vertical_icon_alignment(VERTICAL_ALIGNMENT_TOP);
		button->set_icon_alignment(HORIZONTAL_ALIGNMENT_CENTER);
		button->set_custom_minimum_size(Size2(64, 0) * EDSCALE);
		button->set_focus_mode(FOCUS_ACCESSIBILITY);
		button->set_flat(true);
		if (!is_menu) {
			button->connect(SceneStringName(pressed), callable_mp(this, &StrataToolbar::_button_pressed).bind(info.id));
		}
		buttons[info.id] = button;
		row->add_child(button);

		if (info.id == PLAY) {
			play_options = memnew(MenuButton);
			play_options->set_flat(true);
			play_options->set_tooltip_text(TTRC("Play options."));
			row->add_child(play_options);
		}
	}

	PopupMenu *menu = _add_menu(SOURCE_CONTROL);
	menu->add_item(TTRC("Version Control Metadata..."), VCS_METADATA);
	menu->add_item(TTRC("Version Control Settings..."), VCS_SETTINGS);

	menu = _add_menu(SETTINGS);
	menu->add_item(TTRC("Project Settings..."), SETTINGS_PROJECT);
	menu->add_item(TTRC("Editor Settings..."), SETTINGS_EDITOR);

	menu = _add_menu(BLUEPRINTS);
	menu->add_item(TTRC("Open Script Editor"), BLUEPRINTS_OPEN_SCRIPT_EDITOR);
	menu->add_item(TTRC("Attach Script to Selected..."), BLUEPRINTS_ATTACH);
	menu->add_item(TTRC("Extend Script on Selected..."), BLUEPRINTS_EXTEND);

	menu = _add_menu(CINEMATICS);
	menu->add_item(TTRC("Add AnimationPlayer to Level"), CINEMATICS_ADD_ANIMATION_PLAYER);
	menu->add_item(TTRC("Open Animation Panel"), CINEMATICS_OPEN_ANIMATION);

	menu = _add_menu(BUILD);
	menu->add_item(TTRC("Build All"), BUILD_ALL);
	menu->add_item(TTRC("Build Lighting Only"), BUILD_LIGHTING);
	menu->add_item(TTRC("Build Navigation Only"), BUILD_NAVIGATION);

	// Item shortcuts keep Godot's F5/F6 working: the MenuButton forwards them while the toolbar is visible.
	menu = _add_menu(PLAY);
	if (Engine::get_singleton()->is_recovery_mode_hint()) {
		// Recovery Mode cannot run the project and registers no run shortcuts.
		play_options->set_disabled(true);
		menu->add_item(TTRC("Play Current Level"), PLAY_CURRENT);
		menu->add_item(TTRC("Play Main Level"), PLAY_MAIN);
	} else {
		menu->add_shortcut(ED_GET_SHORTCUT("editor/run_current_scene"), PLAY_CURRENT);
		menu->set_item_text(menu->get_item_count() - 1, TTRC("Play Current Level"));
		menu->add_shortcut(ED_GET_SHORTCUT("editor/run_project"), PLAY_MAIN);
		menu->set_item_text(menu->get_item_count() - 1, TTRC("Play Main Level"));
	}
	menu->add_separator();
	menu->add_check_item(TTRC("Movie Maker Mode"), PLAY_MOVIE_MAKER);

	menu = _add_menu(LAUNCH);
	menu->add_item(TTRC("Export Project..."), LAUNCH_EXPORT);
	menu->add_item(TTRC("Pack Project as ZIP..."), LAUNCH_PACK_ZIP);
}
