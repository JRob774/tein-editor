/*******************************************************************************
 * Core editor functionality with features shared across the map/level editors.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float MIN_LVL_EDITOR_ZOOM   =  .25f;
static constexpr float MAX_LVL_EDITOR_ZOOM   =     4;
static constexpr float MIN_MAP_EDITOR_ZOOM   = .125f;
static constexpr float MAX_MAP_EDITOR_ZOOM   =     1;
static constexpr float EDITOR_ZOOM_INCREMENT =   .1f;

struct Camera
{
    float x;
    float y;

    float zoom;
};

static constexpr size_t INVALID_TAB = static_cast<size_t>(-1);

enum class Tab_Type { LEVEL, MAP };

struct Tab
{
    // GENERAL
    Tab_Type        type;
    std::string     name;
    Camera        camera;
    bool unsaved_changes;

    // LEVEL
    Level         level;
    Tool_Info     tool_info;
    Level_History level_history;
    bool tile_layer_active[LEVEL_LAYER_TOTAL];
    std::vector<Select_Bounds> old_select_state; // We use this for the selection history undo/redo system.

    // MAP
    Map           map;
    Map_History   map_history;
    Map_Select    map_select;
    Map_Node_Info map_node_info;
};

struct Editor
{
    // Names of recently closed tabs so that they can be restored.
    std::vector<std::string> closed_tabs;

    std::vector<Tab> tabs;
    size_t current_tab;

    SDL_TimerID backup_timer;
    SDL_TimerID cooldown_timer;

    bool grid_visible;
    bool is_panning;
    bool dialog_box; // NOTE: See <dialog.cpp> for information.
};

static Editor editor;

/* -------------------------------------------------------------------------- */

FILDEF void init_editor (int argc, char** argv);
FILDEF void quit_editor ();

FILDEF void do_editor ();

FILDEF void handle_editor_events ();

FILDEF void update_backup_timer ();

FILDEF void set_current_tab    (size_t index);
FILDEF Tab& get_current_tab    ();
FILDEF Tab& get_tab_at_index   (size_t index);
FILDEF bool are_there_any_tabs ();

FILDEF void increment_tab ();
FILDEF void decrement_tab ();

FILDEF void set_main_window_subtitle_for_tab (const std::string& subtitle);

FILDEF bool are_there_any_level_tabs ();
FILDEF bool are_there_any_map_tabs   ();

FILDEF void create_new_level_tab_and_focus (int w = DEFAULT_LEVEL_WIDTH, int h = DEFAULT_LEVEL_HEIGHT);
FILDEF void create_new_map_tab_and_focus   ();

FILDEF bool current_tab_is_level ();
FILDEF bool current_tab_is_map   ();

FILDEF void close_tab         (size_t index);
FILDEF void close_current_tab ();
FILDEF void close_all_tabs    ();

FILDEF size_t get_tab_index_with_this_file_name (std::string file_name);

FILDEF void push_editor_camera_transform ();
FILDEF void pop_editor_camera_transform  ();

FILDEF int save_changes_prompt (Tab& tab);

FILDEF void backup_tab (Tab& tab);

FILDEF bool is_current_tab_empty ();

FILDEF void editor_select_all ();
FILDEF void editor_paste      ();

FILDEF bool save_prompt_all_editor_tabs ();

FILDEF void open_recently_closed_tab ();

FILDEF void save_restore_files ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
