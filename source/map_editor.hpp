/*******************************************************************************
 * The systems and functionality for the map editing portion of the editor.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

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
    IVec2 a; // Start
    IVec2 b; // End
};

typedef std::vector<Map_Node> Map_Clipboard;

struct Map_Node_Info
{
    std::string cached_lvl_text;

    IVec2     pressed_node_pos;
    IVec2     active_pos;
    Map_Node* active;

    size_t cursor;
    size_t select; // Start point of text selection.
    bool   selecting;
};

struct Map_Editor
{
    struct Color_Map_Compare
    {
        inline bool operator() (const Vec4& lhs, const Vec4& rhs) const
        {
            return ((lhs.x+lhs.y+lhs.z+lhs.w) < (rhs.x+rhs.y+rhs.z+rhs.w));
        }
    };

    Map_Clipboard clipboard;

    Vec2 mouse_world;
    Vec2 mouse;
    Vec2 mouse_tile;

    bool pressed;
    bool left_pressed;

    SDL_TimerID cursor_blink_timer;
    bool        cursor_visible;

    Quad bounds;
    Quad viewport;

    std::map<Vec4,Vec4, Color_Map_Compare> text_color_map;
};

static Map_Editor map_editor;

/* -------------------------------------------------------------------------- */

TEINAPI void init_map_editor ();
TEINAPI void do_map_editor   ();

TEINAPI void handle_map_editor_events ();

TEINAPI void load_map_tab    (std::string file_name);
TEINAPI bool save_map_tab    (Tab& tab);
TEINAPI void save_map_tab_as ();

TEINAPI void map_drop_file (Tab* tab, std::string file_name);

TEINAPI void backup_map_tab (const Tab& tab, const std::string& file_name);

TEINAPI bool is_current_map_empty ();

TEINAPI float get_min_map_bounds_x ();
TEINAPI float get_min_map_bounds_y ();
TEINAPI float get_max_map_bounds_x ();
TEINAPI float get_max_map_bounds_y ();

TEINAPI void me_cut   ();
TEINAPI void me_copy  ();
TEINAPI void me_paste ();

TEINAPI void me_deselect     ();
TEINAPI void me_clear_select ();
TEINAPI void me_select_all   ();

TEINAPI void me_undo ();
TEINAPI void me_redo ();

TEINAPI void me_history_begin ();
TEINAPI void me_history_end   ();

TEINAPI void new_map_history_state (Map& map);

TEINAPI bool map_select_box_present ();
TEINAPI void get_map_select_bounds  (int* l, int* t, int* r, int* b);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
