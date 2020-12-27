/*******************************************************************************
 * System for loading and storing user-defined settings for the editor.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

// These are default values for if certain settings values cannot be found.

static constexpr const char* SETTINGS_DEFAULT_GAME_PATH           = "";
static constexpr const char* SETTINGS_DEFAULT_UI_THEME            = "dark";
static constexpr const char* SETTINGS_DEFAULT_FONT_FACE           = "OpenSans";
static constexpr const char* SETTINGS_DEFAULT_TILE_GRAPHICS       = "new";
static constexpr bool        SETTINGS_DEFAULT_CUSTOM_CURSORS      = true;
static constexpr bool        SETTINGS_DEFAULT_SHOW_TOOLTIPS       = true;
static constexpr bool        SETTINGS_DEFAULT_UNLIMITED_BACKUPS   = false;
static constexpr int         SETTINGS_DEFAULT_BACKUP_COUNT        = 5;
static constexpr bool        SETTINGS_DEFAULT_AUTO_BACKUP         = true;
static constexpr int         SETTINGS_DEFAULT_BACKUP_INTERVAL     = 180;
static           const vec4  SETTINGS_DEFAULT_SELECT_COLOR        = { .94f, .0f, 1.0f, .25f };
static           const vec4  SETTINGS_DEFAULT_OUT_OF_BOUNDS_COLOR = { .25f, .1f,  .1f, .40f };
static           const vec4  SETTINGS_DEFAULT_CURSOR_COLOR        = { .20f, .9f,  .2f, .40f };
static           const vec4  SETTINGS_DEFAULT_MIRROR_LINE_COLOR   = { .80f, .2f,  .2f, .80f };

/* -------------------------------------------------------------------------- */

// Fallback copy of the settings file we can load in the case that the
// settings file is not present and then we just save these defaults.

static constexpr const char* SETTINGS_FALLBACK =
"user_interface_theme dark\n"
"font_face OpenSans\n"
"tile_graphics new\n"
"custom_cursors true\n"
"show_tooltips true\n"
"unlimited_backups false\n"
"backup_count 5\n"
"auto_backup true\n"
"auto_backup_interval 120\n"
"background_color none\n"
"select_color [0.900000 0.000000 1.000000 0.250000]\n"
"out_of_bounds_color [0.250000 0.100000 0.100000 0.400000]\n"
"cursor_color [0.200000 0.900000 0.200000 0.400000]\n"
"mirror_line_color [0.800000 0.200000 0.200000 0.800000]\n"
"tile_grid_color none\n";

/* -------------------------------------------------------------------------- */

static bool settings_loaded;

/* -------------------------------------------------------------------------- */

TEINAPI vec4 internal__get_settings_color (const GonObject& gon, std::string name, vec4 default_value, bool* did_default = NULL)
{
    if (did_default) *did_default = true;

    // If the color couldn't be found we just return back default.
    if (!gon.Contains(name) || gon[name].type != GonObject::FieldType::ARRAY)
    {
        return default_value;
    }

    vec4 color;

    color.r = static_cast<float>(gon[name][0].Number(1));
    color.g = static_cast<float>(gon[name][1].Number(1));
    color.b = static_cast<float>(gon[name][2].Number(1));
    color.a = static_cast<float>(gon[name][3].Number(1));

    if (did_default) *did_default = false;

    return color;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool operator== (const Settings& a, const Settings& b)
{
    return (a.game_path                  == b.game_path                  &&
            a.background_color_defaulted == b.background_color_defaulted &&
            a.tile_grid_color_defaulted  == b.tile_grid_color_defaulted  &&
            a.ui_theme                   == b.ui_theme                   &&
            a.font_face                  == b.font_face                  &&
            a.tile_graphics              == b.tile_graphics              &&
            a.custom_cursors             == b.custom_cursors             &&
            a.show_tooltips              == b.show_tooltips              &&
            a.unlimited_backups          == b.unlimited_backups          &&
            a.backup_count               == b.backup_count               &&
            a.auto_backup                == b.auto_backup                &&
            a.backup_interval            == b.backup_interval            &&
            a.background_color           == b.background_color           &&
            a.select_color               == b.select_color               &&
            a.out_of_bounds_color        == b.out_of_bounds_color        &&
            a.cursor_color               == b.cursor_color               &&
            a.mirror_line_color          == b.mirror_line_color          &&
            a.tile_grid_color            == b.tile_grid_color);
}

TEINAPI bool operator!= (const Settings& a, const Settings& b)
{
    return !(a == b);
}

/* -------------------------------------------------------------------------- */

TEINAPI void update_systems_that_rely_on_settings (bool tile_graphics_changed)
{
    update_backup_timer();
    update_editor_font ();
    load_ui_theme      ();

    if (tile_graphics_changed)
    {
        reload_tile_graphics();
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI bool load_editor_settings ()
{
    GonObject gon;
    try
    {
        std::string file_name(make_path_absolute(SETTINGS_FILE_NAME));
        gon = GonObject::Load(file_name);
    }
    catch (const char* msg)
    {
        LOG_ERROR(ERR_MED, "%s", msg);

        // If we already have settings data then we just inform the user that the operation
        // failed. Otherwise, we just fallback to using the default application settings.
        if (settings_loaded)
        {
            LOG_ERROR(ERR_MED, "Failed to reload settings data!");
            return false;
        }
        else
        {
            gon = GonObject::LoadFromBuffer(SETTINGS_FALLBACK);
        }
    }

    // If we reach this point and there are no settings then we just use the defaults.
    // This could be the case if the settings failed to load or haven't been modified.
    if (gon.type != GonObject::FieldType::OBJECT)
    {
        LOG_DEBUG("No settings file found -- using defaults.");
        gon = GonObject::LoadFromBuffer(SETTINGS_FALLBACK);
    }
    else
    {
        LOG_DEBUG("Loaded Editor Settings");
    }

    // Load the settings values from the GON into the actual values.
    editor_settings.game_path         = gon[SETTING_GAME_PATH        ].String(SETTINGS_DEFAULT_GAME_PATH        );
    editor_settings.ui_theme          = gon[SETTING_UI_THEME         ].String(SETTINGS_DEFAULT_UI_THEME         );
    editor_settings.font_face         = gon[SETTING_FONT_FACE        ].String(SETTINGS_DEFAULT_FONT_FACE        );
    editor_settings.tile_graphics     = gon[SETTING_TILE_GRAPHICS    ].String(SETTINGS_DEFAULT_TILE_GRAPHICS    );
    editor_settings.custom_cursors    = gon[SETTING_CUSTOM_CURSORS   ].Bool  (SETTINGS_DEFAULT_CUSTOM_CURSORS   );
    editor_settings.show_tooltips     = gon[SETTING_SHOW_TOOLTIPS    ].Bool  (SETTINGS_DEFAULT_SHOW_TOOLTIPS    );
    editor_settings.unlimited_backups = gon[SETTING_UNLIMITED_BACKUPS].Bool  (SETTINGS_DEFAULT_UNLIMITED_BACKUPS);
    editor_settings.backup_count      = gon[SETTING_BACKUP_COUNT     ].Int   (SETTINGS_DEFAULT_BACKUP_COUNT     );
    editor_settings.auto_backup       = gon[SETTING_AUTO_BACKUP      ].Bool  (SETTINGS_DEFAULT_AUTO_BACKUP      );
    editor_settings.backup_interval   = gon[SETTING_BACKUP_INTERVAL  ].Int   (SETTINGS_DEFAULT_BACKUP_INTERVAL  );

    update_systems_that_rely_on_settings(true);

    // Load the colors afterwards because some of them depend on the UI theme.
    vec4 default_background_color = ui_color_light;
    vec4 default_tile_grid_color  = is_ui_light() ? ui_color_black : ui_color_ex_dark;

    editor_settings.background_color    = internal__get_settings_color(gon, SETTING_BACKGROUND_COLOR,    default_background_color,            &editor_settings.background_color_defaulted);
    editor_settings.select_color        = internal__get_settings_color(gon, SETTING_SELECT_COLOR,        SETTINGS_DEFAULT_SELECT_COLOR                                                   );
    editor_settings.out_of_bounds_color = internal__get_settings_color(gon, SETTING_OUT_OF_BOUNDS_COLOR, SETTINGS_DEFAULT_OUT_OF_BOUNDS_COLOR                                            );
    editor_settings.cursor_color        = internal__get_settings_color(gon, SETTING_CURSOR_COLOR,        SETTINGS_DEFAULT_CURSOR_COLOR                                                   );
    editor_settings.mirror_line_color   = internal__get_settings_color(gon, SETTING_MIRROR_LINE_COLOR,   SETTINGS_DEFAULT_MIRROR_LINE_COLOR                                              );
    editor_settings.tile_grid_color     = internal__get_settings_color(gon, SETTING_TILE_GRID_COLOR,     default_tile_grid_color,             &editor_settings.tile_grid_color_defaulted );

    settings_loaded = true;
    return true;
}

TEINAPI void restore_editor_settings ()
{
    bool tile_graphics_changed = (editor_settings.tile_graphics != SETTINGS_DEFAULT_TILE_GRAPHICS);

    // Restore the editor settings values to their default values.
    editor_settings.ui_theme          = SETTINGS_DEFAULT_UI_THEME;
    editor_settings.font_face         = SETTINGS_DEFAULT_FONT_FACE;
    editor_settings.tile_graphics     = SETTINGS_DEFAULT_TILE_GRAPHICS;
    editor_settings.custom_cursors    = SETTINGS_DEFAULT_CUSTOM_CURSORS;
    editor_settings.show_tooltips     = SETTINGS_DEFAULT_SHOW_TOOLTIPS;
    editor_settings.unlimited_backups = SETTINGS_DEFAULT_UNLIMITED_BACKUPS;
    editor_settings.backup_count      = SETTINGS_DEFAULT_BACKUP_COUNT;
    editor_settings.auto_backup       = SETTINGS_DEFAULT_AUTO_BACKUP;
    editor_settings.backup_interval   = SETTINGS_DEFAULT_BACKUP_INTERVAL;

    update_systems_that_rely_on_settings(tile_graphics_changed);

    // Restore the colors afterwards because some of them depend on the UI theme.
    vec4 default_background_color = ui_color_light;
    vec4 default_tile_grid_color = is_ui_light() ? ui_color_black : ui_color_ex_dark;

    editor_settings.background_color    = default_background_color;
    editor_settings.select_color        = SETTINGS_DEFAULT_SELECT_COLOR;
    editor_settings.out_of_bounds_color = SETTINGS_DEFAULT_OUT_OF_BOUNDS_COLOR;
    editor_settings.cursor_color        = SETTINGS_DEFAULT_CURSOR_COLOR;
    editor_settings.mirror_line_color   = SETTINGS_DEFAULT_MIRROR_LINE_COLOR;
    editor_settings.tile_grid_color     = default_tile_grid_color;
}

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

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
