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

TEINAPI void init_editor (int argc, char** argv);
TEINAPI void quit_editor ();

TEINAPI void do_editor ();

TEINAPI void handle_editor_events ();

TEINAPI void update_backup_timer ();

TEINAPI void set_current_tab    (size_t index);
TEINAPI Tab& get_current_tab    ();
TEINAPI Tab& get_tab_at_index   (size_t index);
TEINAPI bool are_there_any_tabs ();

TEINAPI void increment_tab ();
TEINAPI void decrement_tab ();

TEINAPI void set_main_window_subtitle_for_tab (const std::string& subtitle);

TEINAPI bool are_there_any_level_tabs ();
TEINAPI bool are_there_any_map_tabs   ();

TEINAPI void create_new_level_tab_and_focus (int w = gDefaultLevelWidth, int h = gDefaultLevelHeight);
TEINAPI void create_new_map_tab_and_focus   ();

TEINAPI bool current_tab_is_level ();
TEINAPI bool current_tab_is_map   ();

TEINAPI void close_tab         (size_t index);
TEINAPI void close_current_tab ();
TEINAPI void close_all_tabs    ();

TEINAPI size_t get_tab_index_with_this_file_name (std::string file_name);

TEINAPI void push_editor_camera_transform ();
TEINAPI void pop_editor_camera_transform  ();

TEINAPI int save_changes_prompt (Tab& tab);

TEINAPI void backup_tab (Tab& tab);

TEINAPI bool is_current_tab_empty ();

TEINAPI void editor_select_all ();
TEINAPI void editor_paste      ();

TEINAPI bool save_prompt_all_editor_tabs ();

TEINAPI void open_recently_closed_tab ();

TEINAPI void save_restore_files ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
