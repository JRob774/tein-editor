/*******************************************************************************
 * System for binding specific keys to actions and handling them when pressed.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#ifndef KEY_BINDINGS_HPP__ /*/////////////////////////////////////////////////*/
#define KEY_BINDINGS_HPP__

#ifdef COMPILER_HAS_PRAGMA_ONCE
#pragma once
#endif

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr const char* KEY_BINDINGS_FILE_NAME = "editor_settings.txt";

// The names for all of the key bindings found in the user settings file.
// We store them here so that if we ever want to change any of the names
// then we only have to update them here rather than everywhere they are
// referenced throughout the program's source code -- would be annoying!

static constexpr const char* KB_LEVEL_NEW            = "level_new";
static constexpr const char* KB_LEVEL_OPEN           = "level_open";
static constexpr const char* KB_LEVEL_SAVE           = "level_save";
static constexpr const char* KB_LEVEL_SAVE_AS        = "level_save_as";
static constexpr const char* KB_LEVEL_CLOSE          = "level_close";
static constexpr const char* KB_LEVEL_CLOSE_ALL      = "level_close_all";
static constexpr const char* KB_LEVEL_RESIZE         = "level_resize";
static constexpr const char* KB_UNDO                 = "undo";
static constexpr const char* KB_REDO                 = "redo";
static constexpr const char* KB_HISTORY_BEGIN        = "history_begin";
static constexpr const char* KB_HISTORY_END          = "history_end";
static constexpr const char* KB_CAMERA_ZOOM_OUT      = "camera_zoom_out";
static constexpr const char* KB_CAMERA_ZOOM_IN       = "camera_zoom_in";
static constexpr const char* KB_RUN_GAME             = "run_game";
static constexpr const char* KB_PREFERENCES          = "preferences";
static constexpr const char* KB_ABOUT                = "about";
static constexpr const char* KB_BUG_REPORT           = "bug_report";
static constexpr const char* KB_HELP                 = "help";
static constexpr const char* KB_TOOL_BRUSH           = "tool_brush";
static constexpr const char* KB_TOOL_FILL            = "tool_fill";
static constexpr const char* KB_TOOL_SELECT          = "tool_select";
static constexpr const char* KB_FLIP_H               = "flip_h";
static constexpr const char* KB_FLIP_V               = "flip_v";
static constexpr const char* KB_MIRROR_H_TOGGLE      = "mirror_h_toggle";
static constexpr const char* KB_MIRROR_V_TOGGLE      = "mirror_v_toggle";
static constexpr const char* KB_GPAK_UNPACK          = "gpak_unpack";
static constexpr const char* KB_GPAK_PACK            = "gpak_pack";
static constexpr const char* KB_TOGGLE_ENTITY        = "toggle_large_entity";
static constexpr const char* KB_TOGGLE_GUIDES        = "toggle_entity_guides";
static constexpr const char* KB_GRID_TOGGLE          = "grid_toggle";
static constexpr const char* KB_BOUNDS_TOGGLE        = "bounds_toggle";
static constexpr const char* KB_LAYERS_TOGGLE        = "layer_trans_toggle";
static constexpr const char* KB_CAMERA_RESET         = "camera_reset";
static constexpr const char* KB_CLEAR_SELECT         = "clear_select";
static constexpr const char* KB_DESELECT             = "deselect";
static constexpr const char* KB_SELECT_ALL           = "select_all";
static constexpr const char* KB_COPY                 = "copy";
static constexpr const char* KB_CUT                  = "cut";
static constexpr const char* KB_PASTE                = "paste";
static constexpr const char* KB_INCREMENT_TILE       = "increment_tile";
static constexpr const char* KB_DECREMENT_TILE       = "decrement_tile";
static constexpr const char* KB_INCREMENT_GROUP      = "increment_group";
static constexpr const char* KB_DECREMENT_GROUP      = "decrement_group";
static constexpr const char* KB_INCREMENT_CATEGORY   = "increment_category";
static constexpr const char* KB_DECREMENT_CATEGORY   = "decrement_category";
static constexpr const char* KB_TOGGLE_LAYER_ACTIVE  = "toggle_layer_active";
static constexpr const char* KB_TOGGLE_LAYER_TAG     = "toggle_layer_tag";
static constexpr const char* KB_TOGGLE_LAYER_OVERLAY = "toggle_layer_overlay";
static constexpr const char* KB_TOGGLE_LAYER_BACK1   = "toggle_layer_back1";
static constexpr const char* KB_TOGGLE_LAYER_BACK2   = "toggle_layer_back2";
static constexpr const char* KB_CATEGORY_BASIC       = "category_basic";
static constexpr const char* KB_CATEGORY_TAG         = "category_tag";
static constexpr const char* KB_CATEGORY_OVERLAY     = "category_overlay";
static constexpr const char* KB_CATEGORY_ENTITY      = "category_entity";
static constexpr const char* KB_CATEGORY_BACK1       = "category_back1";
static constexpr const char* KB_CATEGORY_BACK2       = "category_back2";
static constexpr const char* KB_INCREMENT_TAB        = "increment_tab";
static constexpr const char* KB_DECREMENT_TAB        = "decrement_tab";
static constexpr const char* KB_MOVE_TAB_LEFT        = "move_tab_left";
static constexpr const char* KB_MOVE_TAB_RIGHT       = "move_tab_right";
static constexpr const char* KB_OPEN_RECENT_TAB      = "open_recent_tab";
static constexpr const char* KB_LOAD_PREV_LEVEL      = "load_prev_level";
static constexpr const char* KB_LOAD_NEXT_LEVEL      = "load_next_level";

/* -------------------------------------------------------------------------- */

typedef void(*KB_Action)(void);

struct Key_Binding
{
    KB_Action action;

    bool has_alt;

    int mod;
    int code;

    int alt_mod;
    int alt_code;
};

FILDEF bool operator== (const Key_Binding& a, const Key_Binding& b);
FILDEF bool operator!= (const Key_Binding& a, const Key_Binding& b);

/* -------------------------------------------------------------------------- */

static std::map<std::string, Key_Binding> key_bindings;

/* -------------------------------------------------------------------------- */

FILDEF bool load_editor_key_bindings    ();
FILDEF void restore_editor_key_bindings ();

FILDEF void handle_key_binding_events   ();

FILDEF const Key_Binding& get_key_binding (std::string name);

FILDEF std::string get_key_binding_main_string (const Key_Binding& kb);
FILDEF std::string get_key_binding_main_string (std::string      name);
FILDEF std::string get_key_binding_alt_string  (const Key_Binding& kb);
FILDEF std::string get_key_binding_alt_string  (std::string      name);

FILDEF bool is_key_binding_active   (std::string name);
FILDEF bool is_key_mod_state_active (int          mod);
FILDEF bool is_key_code_active      (int         code);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

#endif /* KEY_BINDINGS_HPP__ /////////////////////////////////////////////////*/

/*******************************************************************************
 *
 * Copyright (c) 2020 Joshua Robertson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
*******************************************************************************/
