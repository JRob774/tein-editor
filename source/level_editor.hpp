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
    std::vector<vec2> frontier;
    std::vector<bool> searched;

    Level_Layer layer;

    Tile_ID find_id;
    Tile_ID replace_id;

    vec2 start;

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

    vec2 mouse_world;
    vec2 mouse;
    vec2 mouse_tile;

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

FILDEF void init_level_editor ();
FILDEF void do_level_editor   ();

FILDEF void handle_level_editor_events ();

FILDEF bool mouse_inside_level_editor_viewport ();

FILDEF void new_level_history_state (Level_History_Action action);

FILDEF void add_to_history_normal_state (Level_History_Info info);
FILDEF void add_to_history_clear_state  (Level_History_Info info);

FILDEF bool are_all_layers_inactive ();

FILDEF bool are_any_select_boxes_visible ();
FILDEF void get_ordered_select_bounds    (const Select_Bounds& bounds, int* l, int* t, int* r, int* b);
FILDEF void get_total_select_boundary    (int* l, int* t, int* r, int* b);

FILDEF void load_level_tab (std::string file_name);

FILDEF bool le_save            (Tab& level);
FILDEF bool le_save_as         ();
FILDEF void le_clear_select    ();
FILDEF void le_deselect        ();
FILDEF void le_cursor_deselect ();
FILDEF void le_select_all      ();
FILDEF void le_copy            ();
FILDEF void le_cut             ();
FILDEF void le_paste           ();

FILDEF void flip_level_h ();
FILDEF void flip_level_v ();

FILDEF void level_has_unsaved_changes ();

FILDEF void le_undo ();
FILDEF void le_redo ();

FILDEF void le_history_begin ();
FILDEF void le_history_end   ();

FILDEF void le_resize      ();
FILDEF void le_resize_okay ();

FILDEF void le_load_prev_level ();
FILDEF void le_load_next_level ();

FILDEF void level_drop_file (Tab* tab, std::string file_name);

FILDEF void backup_level_tab (const Level& level, const std::string& file_name);

FILDEF bool is_current_level_empty ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
