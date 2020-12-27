/*******************************************************************************
 * System for loading and storing user-defined settings for the editor.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr const char* SETTINGS_FILE_NAME = "settings_editor.txt";

/* -------------------------------------------------------------------------- */

// The names for all of the user settings found in the user settings file.
// We store them here so that if we ever want to change any of the names
// then we only have to update them here rather than everywhere they are
// referenced throughout the program's source code -- would be annoying!

static constexpr const char* SETTING_GAME_PATH           = "game_path";
static constexpr const char* SETTING_UI_THEME            = "user_interface_theme";
static constexpr const char* SETTING_FONT_FACE           = "font_face";
static constexpr const char* SETTING_TILE_GRAPHICS       = "tile_graphics";
static constexpr const char* SETTING_CUSTOM_CURSORS      = "custom_cursors";
static constexpr const char* SETTING_SHOW_TOOLTIPS       = "show_tooltips";
static constexpr const char* SETTING_UNLIMITED_BACKUPS   = "unlimited_backups";
static constexpr const char* SETTING_BACKUP_COUNT        = "backup_count";
static constexpr const char* SETTING_AUTO_BACKUP         = "auto_backup";
static constexpr const char* SETTING_BACKUP_INTERVAL     = "auto_backup_interval";
static constexpr const char* SETTING_BACKGROUND_COLOR    = "background_color";
static constexpr const char* SETTING_SELECT_COLOR        = "select_color";
static constexpr const char* SETTING_OUT_OF_BOUNDS_COLOR = "out_of_bounds_color";
static constexpr const char* SETTING_CURSOR_COLOR        = "cursor_color";
static constexpr const char* SETTING_MIRROR_LINE_COLOR   = "mirror_line_color";
static constexpr const char* SETTING_TILE_GRID_COLOR     = "tile_grid_color";

/* -------------------------------------------------------------------------- */

struct Settings
{
    // Not a proper setting but we save it for convenience when running the game.
    std::string game_path;

    // Special cases for these two colors where their defaults depend on the
    // UI so we need to know if they are defaulted or not when displaying.
    bool background_color_defaulted;
    bool tile_grid_color_defaulted;

    // USER INTERFACE
    std::string      ui_theme;
    std::string     font_face;
    std::string tile_graphics;
    bool       custom_cursors;
    bool        show_tooltips;
    // LEVEL BACKUPS
    bool    unlimited_backups;
    int          backup_count;
    bool          auto_backup;
    int       backup_interval;
    // EDITOR COLORS
    vec4     background_color;
    vec4         select_color;
    vec4  out_of_bounds_color;
    vec4         cursor_color;
    vec4    mirror_line_color;
    vec4      tile_grid_color;
};

TEINAPI bool operator== (const Settings& a, const Settings& b);
TEINAPI bool operator!= (const Settings& a, const Settings& b);

/* -------------------------------------------------------------------------- */

static Settings editor_settings;

/* -------------------------------------------------------------------------- */

TEINAPI void update_systems_that_rely_on_settings (bool tile_graphics_changed);

/* -------------------------------------------------------------------------- */

TEINAPI bool    load_editor_settings ();
TEINAPI void restore_editor_settings ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
