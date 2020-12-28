#pragma once

static constexpr const char* gSettingsFileName = "settings_editor.txt";

// The names for all of the user settings found in the user settings file.
// We store them here so that if we ever want to change any of the names
// then we only have to update them here rather than everywhere they are
// referenced throughout the program's source code -- would be annoying!

static constexpr const char* gSettingGamePath         = "game_path";
static constexpr const char* gSettingUITheme          = "user_interface_theme";
static constexpr const char* gSettingFontFace         = "font_face";
static constexpr const char* gSettingTileGraphics     = "tile_graphics";
static constexpr const char* gSettingCustomCursors    = "custom_cursors";
static constexpr const char* gSettingShowTooltips     = "show_tooltips";
static constexpr const char* gSettingUnlimitedBackups = "unlimited_backups";
static constexpr const char* gSettingBackupCount      = "backup_count";
static constexpr const char* gSettingAutoBackup       = "auto_backup";
static constexpr const char* gSettingBackupInterval   = "auto_backup_interval";
static constexpr const char* gSettingBackgroundColor  = "background_color";
static constexpr const char* gSettingSelectColor      = "select_color";
static constexpr const char* gSettingOutOfBoundsColor = "out_of_bounds_color";
static constexpr const char* gSettingCursorColor      = "cursor_color";
static constexpr const char* gSettingMirrorLineColor  = "mirror_line_color";
static constexpr const char* gSettingTileGridColor    = "tile_grid_color";

struct Settings
{
    // Not a proper setting but we save it for convenience when running the game.
    std::string gamePath;

    // Special cases for these two colors where their defaults depend on the
    // UI so we need to know if they are defaulted or not when displaying.
    bool backgroundColorDefaulted;
    bool tileGridColorDefaulted;

    // USER INTERFACE
    std::string uiTheme;
    std::string fontFace;
    std::string tileGraphics;
    bool customCursors;
    bool showTooltips;

    // LEVEL BACKUPS
    bool unlimitedBackups;
    int backupCount;
    bool autoBackup;
    int backupInterval;

    // EDITOR COLORS
    Vec4 backgroundColor;
    Vec4 selectColor;
    Vec4 outOfBoundsColor;
    Vec4 cursorColor;
    Vec4 mirrorLineColor;
    Vec4 tileGridColor;
};

TEINAPI inline bool operator== (const Settings& a, const Settings& b);
TEINAPI inline bool operator!= (const Settings& a, const Settings& b);

static Settings gEditorSettings;

// The boolean is passed so we don't attempt to reload tiles if they haven't changed (it takes a while).
TEINAPI void UpdateSystemsThatRelyOnSettings (bool tileGraphicsChanged);

TEINAPI bool LoadEditorSettings    ();
TEINAPI void RestoreEditorSettings ();
