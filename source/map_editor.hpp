/*******************************************************************************
 * The systems and functionality for the map editing portion of the editor.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#ifndef MAP_EDITOR_HPP__ /*///////////////////////////////////////////////////*/
#define MAP_EDITOR_HPP__

#ifdef COMPILER_HAS_PRAGMA_ONCE
#pragma once
#endif

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float MAP_EDITOR_TEXT_CUT_OFF = .5f;
static constexpr float MAP_EDITOR_TEXT_PAD = 3;

static constexpr float MAP_NODE_W = 88;
static constexpr float MAP_NODE_H = 22;

struct Map_History
{
    int current_position;
    std::vector<Map> state;
};

struct Map_Select
{
    ivec2 a; // Start
    ivec2 b; // End
};

typedef std::vector<Map_Node> Map_Clipboard;

struct Map_Node_Info
{
    std::string cached_lvl_text;

    ivec2     pressed_node_pos;
    ivec2     active_pos;
    Map_Node* active;

    size_t cursor;
    size_t select; // Start point of text selection.
    bool   selecting;
};

struct Map_Editor
{
    struct Color_Map_Compare
    {
        inline bool operator() (const vec4& lhs, const vec4& rhs) const
        {
            return ((lhs.x+lhs.y+lhs.z+lhs.w) < (rhs.x+rhs.y+rhs.z+rhs.w));
        }
    };

    Map_Clipboard clipboard;

    vec2 mouse_world;
    vec2 mouse;
    vec2 mouse_tile;

    bool pressed;
    bool left_pressed;

    SDL_TimerID cursor_blink_timer;
    bool        cursor_visible;

    quad bounds;
    quad viewport;

    std::map<vec4,vec4, Color_Map_Compare> text_color_map;
};

static Map_Editor map_editor;

/* -------------------------------------------------------------------------- */

FILDEF void init_map_editor ();
FILDEF void do_map_editor   ();

FILDEF void handle_map_editor_events ();

FILDEF void load_map_tab    (std::string file_name);
FILDEF bool save_map_tab    (Tab& tab);
FILDEF void save_map_tab_as ();

FILDEF void map_drop_file (Tab* tab, std::string file_name);

FILDEF void backup_map_tab (const Tab& tab, const std::string& file_name);

FILDEF bool is_current_map_empty ();

FILDEF float get_min_map_bounds_x ();
FILDEF float get_min_map_bounds_y ();
FILDEF float get_max_map_bounds_x ();
FILDEF float get_max_map_bounds_y ();

FILDEF void me_cut   ();
FILDEF void me_copy  ();
FILDEF void me_paste ();

FILDEF void me_deselect     ();
FILDEF void me_clear_select ();
FILDEF void me_select_all   ();

FILDEF void me_undo ();
FILDEF void me_redo ();

FILDEF void me_history_begin ();
FILDEF void me_history_end   ();

FILDEF void new_map_history_state (Map& map);

FILDEF bool map_select_box_present ();
FILDEF void get_map_select_bounds  (int* l, int* t, int* r, int* b);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

#endif /* MAP_EDITOR_HPP__ ///////////////////////////////////////////////////*/

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
