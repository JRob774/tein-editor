/*******************************************************************************
 * THe systems and functionality for the level editing portion of the editor.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

enum class Tool_State { IDLE, PLACE, ERASE  };
enum class Tool_Type  { BRUSH, FILL, SELECT };

struct Tab; // Defined in <editor.hpp>

struct Tool_Fill
{
    std::vector<Vec2> frontier;
    std::vector<bool> searched;

    Level_Layer layer;

    Tile_ID find_id;
    Tile_ID replace_id;

    Vec2 start;

    bool inside_select;
};

struct Select_Bounds
{
    int top;
    int right;
    int bottom;
    int left;

    bool visible;
};

struct Tool_Select
{
    std::vector<Select_Bounds> bounds;

    bool start;
    bool add;

    size_t cached_size;
};

struct Tool_Info
{
    Tool_Fill   fill;
    Tool_Select select;
};

/* -------------------------------------------------------------------------- */

enum class Level_History_Action
{
    NORMAL,
    FLIP_LEVEL_H,
    FLIP_LEVEL_V,
    SELECT_STATE,
    CLEAR,
    RESIZE
};

struct Level_History_Info
{
    int x;
    int y;

    Tile_ID old_id;
    Tile_ID new_id;

    Level_Layer tile_layer;
};

struct Level_History_State
{
    Level_History_Action action;

    std::vector<Level_History_Info> info;

    // What layers were active at the time. Used by flips so only those
    // layers end up getting flipped during the undo and redo actions.
    bool tile_layer_active[LEVEL_LAYER_TOTAL];

    // Used by the select box history to know the size and position.
    std::vector<Select_Bounds> old_select_state;
    std::vector<Select_Bounds> new_select_state;

    // Used by the resizing history to restore old/new size.
    Resize_Dir resize_dir;

    int old_width;
    int old_height;

    int new_width;
    int new_height;

    // The data of the level before and after a resize.
    Level_Data old_data;
    Level_Data new_data;
};

struct Level_History
{
    int current_position;
    std::vector<Level_History_State> state;
};

/* -------------------------------------------------------------------------- */

static constexpr float DEFAULT_TILE_SIZE      = 16;
static constexpr float DEFAULT_TILE_SIZE_HALF = DEFAULT_TILE_SIZE / 2;

struct Level_Clipboard
{
    Level_Data data;

    int x;
    int y;
    int w;
    int h;
};

struct Level_Editor
{
    Tool_State tool_state = Tool_State::IDLE;
    Tool_Type  tool_type  = Tool_Type::BRUSH;

    std::vector<Level_Clipboard> clipboard;

    Vec2 mouse_world;
    Vec2 mouse;
    Vec2 mouse_tile;

    bool bounds_visible;
    bool layer_transparency;
    bool large_tiles = true;
    bool entity_guides;

    bool mirror_h;
    bool mirror_v;

    quad bounds;
    quad viewport;
};

/* -------------------------------------------------------------------------- */

static Level_Editor level_editor;

/* -------------------------------------------------------------------------- */

TEINAPI void init_level_editor ();
TEINAPI void do_level_editor   ();

TEINAPI void handle_level_editor_events ();

TEINAPI bool mouse_inside_level_editor_viewport ();

TEINAPI void new_level_history_state (Level_History_Action action);

TEINAPI void add_to_history_normal_state (Level_History_Info info);
TEINAPI void add_to_history_clear_state  (Level_History_Info info);

TEINAPI bool are_all_layers_inactive ();

TEINAPI bool are_any_select_boxes_visible ();
TEINAPI void get_ordered_select_bounds    (const Select_Bounds& bounds, int* l, int* t, int* r, int* b);
TEINAPI void get_total_select_boundary    (int* l, int* t, int* r, int* b);

TEINAPI void load_level_tab (std::string file_name);

TEINAPI bool le_save            (Tab& level);
TEINAPI bool le_save_as         ();
TEINAPI void le_clear_select    ();
TEINAPI void le_deselect        ();
TEINAPI void le_cursor_deselect ();
TEINAPI void le_select_all      ();
TEINAPI void le_copy            ();
TEINAPI void le_cut             ();
TEINAPI void le_paste           ();

TEINAPI void flip_level_h ();
TEINAPI void flip_level_v ();

TEINAPI void level_has_unsaved_changes ();

TEINAPI void le_undo ();
TEINAPI void le_redo ();

TEINAPI void le_history_begin ();
TEINAPI void le_history_end   ();

TEINAPI void le_resize      ();
TEINAPI void le_resize_okay ();

TEINAPI void le_load_prev_level ();
TEINAPI void le_load_next_level ();

TEINAPI void level_drop_file (Tab* tab, std::string file_name);

TEINAPI void backup_level_tab (const Level& level, const std::string& file_name);

TEINAPI bool is_current_level_empty ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
