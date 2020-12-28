// These are default values for if certain settings values cannot be found.

static constexpr const char* gSettingsDefaultGamePath         = "";
static constexpr const char* gSettingsDefaultUITheme          = "dark";
static constexpr const char* gSettingsDefaultFontFace         = "OpenSans";
static constexpr const char* gSettingsDefaultTileGraphics     = "new";
static constexpr bool        gSettingsDefaultCustomCursors    = true;
static constexpr bool        gSettingsDefaultShowTooltips     = true;
static constexpr bool        gSettingsDefaultUnlimitedBackups = false;
static constexpr int         gSettingsDefaultBackupCount      = 5;
static constexpr bool        gSettingsDefaultAutoBackup       = true;
static constexpr int         gSettingsDefaultBackupInterval   = 180;
static           const Vec4  gSettingsDefaultSelectColor      = { .94f, .0f, 1.0f, .25f };
static           const Vec4  gSettingsDefaultOutOfBoundsColor = { .25f, .1f,  .1f, .40f };
static           const Vec4  gSettingsDefaultCursorColor      = { .20f, .9f,  .2f, .40f };
static           const Vec4  gSettingsDefaultMirrorLineColor  = { .80f, .2f,  .2f, .80f };

// Fallback copy of the settings file we can load in the case that the
// settings file is not present and then we just save these defaults.

static constexpr const char* gSettingsFallback =
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

static bool gSettingsLoaded;

namespace Internal
{
    TEINAPI Vec4 GetSettingsColor (const GonObject& gon, std::string name, Vec4 defaultValue, bool* didDefault = NULL)
    {
        // If the color couldn't be found we just return back default.
        if (!gon.Contains(name) || gon[name].type != GonObject::FieldType::ARRAY)
        {
            if (didDefault) *didDefault = true;
            return defaultValue;
        }

        Vec4 color;

        color.r = static_cast<float>(gon[name][0].Number(1));
        color.g = static_cast<float>(gon[name][1].Number(1));
        color.b = static_cast<float>(gon[name][2].Number(1));
        color.a = static_cast<float>(gon[name][3].Number(1));

        if (didDefault) *didDefault = false;

        return color;
    }
}

TEINAPI inline bool operator== (const Settings& a, const Settings& b)
{
    return (a.gamePath                 == b.gamePath                 &&
            a.backgroundColorDefaulted == b.backgroundColorDefaulted &&
            a.tileGridColorDefaulted   == b.tileGridColorDefaulted   &&
            a.uiTheme                  == b.uiTheme                  &&
            a.fontFace                 == b.fontFace                 &&
            a.tileGraphics             == b.tileGraphics             &&
            a.customCursors            == b.customCursors            &&
            a.showTooltips             == b.showTooltips             &&
            a.unlimitedBackups         == b.unlimitedBackups         &&
            a.backupCount              == b.backupCount              &&
            a.autoBackup               == b.autoBackup               &&
            a.backupInterval           == b.backupInterval           &&
            a.backgroundColor          == b.backgroundColor          &&
            a.selectColor              == b.selectColor              &&
            a.outOfBoundsColor         == b.outOfBoundsColor         &&
            a.cursorColor              == b.cursorColor              &&
            a.mirrorLineColor          == b.mirrorLineColor          &&
            a.tileGridColor            == b.tileGridColor);
}
TEINAPI inline bool operator!= (const Settings& a, const Settings& b)
{
    return !(a == b);
}

TEINAPI void UpdateSystemsThatRelyOnSettings (bool tileGraphicsChanged)
{
    update_backup_timer();
    update_editor_font();
    load_ui_theme();
    if (tileGraphicsChanged)
    {
        reload_tile_graphics();
    }
}

TEINAPI bool LoadEditorSettings ()
{
    GonObject gon;
    try
    {
        std::string fileName(MakePathAbsolute(gSettingsFileName));
        gon = GonObject::Load(fileName);
    }
    catch (const char* msg)
    {
        LogError(ERR_MED, "%s", msg);

        // If we already have settings data then we just inform the user that the operation
        // failed. Otherwise, we just fallback to using the default application settings.
        if (gSettingsLoaded)
        {
            LogError(ERR_MED, "Failed to reload settings data!");
            return false;
        }
        else
        {
            gon = GonObject::LoadFromBuffer(gSettingsFallback);
        }
    }

    // If we reach this point and there are no settings then we just use the defaults.
    // This could be the case if the settings failed to load or haven't been modified.
    if (gon.type != GonObject::FieldType::OBJECT)
    {
        LogDebug("No settings file found -- using defaults.");
        gon = GonObject::LoadFromBuffer(gSettingsFallback);
    }
    else
    {
        LogDebug("Loaded Editor Settings");
    }

    // Load the settings values from the GON into the actual values.
    gEditorSettings.gamePath         = gon[gSettingGamePath        ].String(gSettingsDefaultGamePath        );
    gEditorSettings.uiTheme          = gon[gSettingUITheme         ].String(gSettingsDefaultUITheme         );
    gEditorSettings.fontFace         = gon[gSettingFontFace        ].String(gSettingsDefaultFontFace        );
    gEditorSettings.tileGraphics     = gon[gSettingTileGraphics    ].String(gSettingsDefaultTileGraphics    );
    gEditorSettings.customCursors    = gon[gSettingCustomCursors   ].Bool  (gSettingsDefaultCustomCursors   );
    gEditorSettings.showTooltips     = gon[gSettingShowTooltips    ].Bool  (gSettingsDefaultShowTooltips    );
    gEditorSettings.unlimitedBackups = gon[gSettingUnlimitedBackups].Bool  (gSettingsDefaultUnlimitedBackups);
    gEditorSettings.backupCount      = gon[gSettingBackupCount     ].Int   (gSettingsDefaultBackupCount     );
    gEditorSettings.autoBackup       = gon[gSettingAutoBackup      ].Bool  (gSettingsDefaultAutoBackup      );
    gEditorSettings.backupInterval   = gon[gSettingBackupInterval  ].Int   (gSettingsDefaultBackupInterval  );

    UpdateSystemsThatRelyOnSettings(true);

    // Load the colors afterwards because some of them depend on the UI theme.
    Vec4 defaultBackgroundColor = ui_color_light;
    Vec4 defaultTileGridColor = is_ui_light() ? ui_color_black : ui_color_ex_dark;

    gEditorSettings.backgroundColor  = Internal::GetSettingsColor(gon, gSettingBackgroundColor,  defaultBackgroundColor, &gEditorSettings.backgroundColorDefaulted);
    gEditorSettings.selectColor      = Internal::GetSettingsColor(gon, gSettingSelectColor,      gSettingsDefaultSelectColor);
    gEditorSettings.outOfBoundsColor = Internal::GetSettingsColor(gon, gSettingOutOfBoundsColor, gSettingsDefaultOutOfBoundsColor);
    gEditorSettings.cursorColor      = Internal::GetSettingsColor(gon, gSettingCursorColor,      gSettingsDefaultCursorColor);
    gEditorSettings.mirrorLineColor  = Internal::GetSettingsColor(gon, gSettingMirrorLineColor,  gSettingsDefaultMirrorLineColor);
    gEditorSettings.tileGridColor    = Internal::GetSettingsColor(gon, gSettingTileGridColor,    defaultTileGridColor, &gEditorSettings.tileGridColorDefaulted);

    gSettingsLoaded = true;
    return true;
}

TEINAPI void RestoreEditorSettings ()
{
    bool tileGraphicsChanged = (gEditorSettings.tileGraphics != gSettingsDefaultTileGraphics);

    // Restore the editor settings values to their default values.
    gEditorSettings.uiTheme          = gSettingsDefaultUITheme;
    gEditorSettings.fontFace         = gSettingsDefaultFontFace;
    gEditorSettings.tileGraphics     = gSettingsDefaultTileGraphics;
    gEditorSettings.customCursors    = gSettingsDefaultCustomCursors;
    gEditorSettings.showTooltips     = gSettingsDefaultShowTooltips;
    gEditorSettings.unlimitedBackups = gSettingsDefaultUnlimitedBackups;
    gEditorSettings.backupCount      = gSettingsDefaultBackupCount;
    gEditorSettings.autoBackup       = gSettingsDefaultAutoBackup;
    gEditorSettings.backupInterval   = gSettingsDefaultBackupInterval;

    UpdateSystemsThatRelyOnSettings(tileGraphicsChanged);

    // Restore the colors afterwards because some of them depend on the UI theme.
    Vec4 defaultBackgroundColor = ui_color_light;
    Vec4 defaultTileGridColor = is_ui_light() ? ui_color_black : ui_color_ex_dark;

    gEditorSettings.backgroundColor  = defaultBackgroundColor;
    gEditorSettings.selectColor      = gSettingsDefaultSelectColor;
    gEditorSettings.outOfBoundsColor = gSettingsDefaultOutOfBoundsColor;
    gEditorSettings.cursorColor      = gSettingsDefaultCursorColor;
    gEditorSettings.mirrorLineColor  = gSettingsDefaultMirrorLineColor;
    gEditorSettings.tileGridColor    = defaultTileGridColor;
}
