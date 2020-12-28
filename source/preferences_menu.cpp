/*******************************************************************************
 * Menu that allows for the modification of application settings and hotkeys.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static const std::map<std::string, const char*> PREFERENCES_SETTINGS_NAMES
{
{ gSettingUITheme,             "Theme"                         },
{ gSettingFontFace,            "Font"                          },
{ gSettingTileGraphics,        "Tile Graphics"                 },
{ gSettingCustomCursors,       "Cursors"                       },
{ gSettingShowTooltips,        "Tooltips"                      },
{ gSettingUnlimitedBackups,    "Unlimited Backups"             },
{ gSettingBackupCount,         "Backups Per Level"             },
{ gSettingAutoBackup,          "Automatic Backups"             },
{ gSettingBackupInterval,      "Auto-Backup Time"              },
{ gSettingBackgroundColor,     "Background"                    },
{ gSettingSelectColor,         "Select"                        },
{ gSettingOutOfBoundsColor,    "Out of Bounds"                 },
{ gSettingCursorColor,         "Cursor"                        },
{ gSettingMirrorLineColor,     "Mirror Lines"                  },
{ gSettingTileGridColor,       "Tile Grid"                     }
};

/* -------------------------------------------------------------------------- */

static const std::map<std::string, const char*> PREFERENCES_HOTKEYS_NAMES
{
{ gKbLevelNew,                 "New"                           },
{ gKbLevelOpen,                "Load"                          },
{ gKbLevelSave,                "Save"                          },
{ gKbLevelSaveAs,              "Save As"                       },
{ gKbLevelClose,               "Close"                         },
{ gKbLevelCloseAll,            "Close All"                     },
{ gKbLevelResize,              "Resize"                        },
{ gKbUndo,                     "Undo"                          },
{ gKbRedo,                     "Redo"                          },
{ gKbHistoryBegin,             "History Begin"                 },
{ gKbHistoryEnd,               "History End"                   },
{ gKbCameraZoomOut,            "Zoom Out"                      },
{ gKbCameraZoomIn,             "Zoom In"                       },
{ gKbRunGame,                  "Run Game"                      },
{ gKbPreferences,              "Preferences"                   },
{ gKbAbout,                    "About"                         },
{ gKbBugReport,                "Bug Report"                    },
{ gKbHelp,                     "Help"                          },
{ gKbToolBrush,                "Brush"                         },
{ gKbToolFill,                 "Fill"                          },
{ gKbToolSelect,               "Select"                        },
{ gKbFlipH,                    "Flip Horizontal"               },
{ gKbFlipV,                    "Flip Vertical"                 },
{ gKbMirrorHToggle,            "Mirror Horizontal"             },
{ gKbMirrorVToggle,            "Mirror Vertical"               },
{ gKbGpakUnpack,               "Unpack GPAK"                   },
{ gKbGpakPack,                 "Pack GPAK"                     },
{ gKbToggleEntity,             "Toggle Large Entities"         },
{ gKbToggleGuides,             "Toggle Entity Guides"          },
{ gKbGridToggle,               "Toggle Tile Grid"              },
{ gKbBoundsToggle,             "Toggle Out of Bounds"          },
{ gKbLayersToggle,             "Toggle Transparency"           },
{ gKbCameraReset,              "Reset Camera"                  },
{ gKbClearSelect,              "Clear Selection"               },
{ gKbDeselect,                 "Select Box Deselect"           },
{ gKbSelectAll,                "Cursor Deselect"               },
{ gKbCopy,                     "Copy"                          },
{ gKbCut,                      "Cut"                           },
{ gKbPaste,                    "Paste"                         },
{ gKbIncrementTile,            "Next Selected Tile"            },
{ gKbDecrementTile,            "Prev Selected Tile"            },
{ gKbIncrementGroup,           "Next Selected Group"           },
{ gKbDecrementGroup,           "Prev Selected Group"           },
{ gKbIncrementCategory,        "Next Selected Category"        },
{ gKbDecrementCategory,        "Prev Selected Category"        },
{ gKbToggleLayerActive,        "Toggle Active Layer"           },
{ gKbToggleLayerTag,           "Toggle Tag Layer"              },
{ gKbToggleLayerOverlay,       "Toggle Overlay Layer"          },
{ gKbToggleLayerBack1,         "Toggle Back 1 Layer"           },
{ gKbToggleLayerBack2,         "Toggle Back 2 Layer"           },
{ gKbCategoryBasic,            "Category Basic"                },
{ gKbCategoryTag,              "Category Tag"                  },
{ gKbCategoryOverlay,          "Category Overlay"              },
{ gKbCategoryEntity,           "Category Entity"               },
{ gKbCategoryBack1,            "Category Back 1"               },
{ gKbCategoryBack2,            "Category Back 2"               },
{ gKbIncrementTab,             "Next Tab"                      },
{ gKbDecrementTab,             "Prev Tab"                      },
{ gKbMoveTabLeft,              "Move Tab Left"                 },
{ gKbMoveTabRight,             "Move Tab Right"                },
{ gKbOpenRecentTab,            "Open Recent Tab"               },
{ gKbLoadPrevLevel,            "Load Next Level"               },
{ gKbLoadNextLevel,            "Load Prev Level"               }
};

/* -------------------------------------------------------------------------- */

static constexpr float PREFERENCES_V_FRAME_H       = 26;
static constexpr float PREFERENCES_SECTION_H       = 24;
static constexpr float PREFERENCES_INNER_XPAD      = 10;
static constexpr float PREFERENCES_INNER_YPAD      =  5;
static constexpr float PREFERENCES_TEXT_BOX_INSET  =  2;
static constexpr float PREFERENCES_SCROLLBAR_WIDTH = 12;
static constexpr float PREFERENCES_COLOR_HEIGHT    = 18;

enum class Preferennes_Tab { SETTINGS, HOTKEYS };

static Preferennes_Tab preferences_tab = Preferennes_Tab::SETTINGS;

static float preferences_scroll_offset = 0;
static bool  preferences_mouse_pressed = false;

// When the preferences menu is opened we cache the current settings and
// hotkey states. This allows the preferences menu to modify the current
// settings and hotkeys immediately for instant feedback. If the user
// then wants to cancel the changes made, we can just restore the cached
// versions of the settings and hotkeys to return to the original state.

static Settings                          cached_editor_settings;
static std::map<std::string, KeyBinding> cached_editor_hotkeys;

/* -------------------------------------------------------------------------- */

TEINAPI void internal__next_section (Vec2& cursor)
{
    cursor.x  = 0;
    cursor.y += PREFERENCES_SECTION_H;
}

/* -------------------------------------------------------------------------- */

TEINAPI void internal__do_settings_label (float w, const char* key)
{
    do_label(UI_ALIGN_LEFT,UI_ALIGN_CENTER, w,PREFERENCES_SECTION_H, PREFERENCES_SETTINGS_NAMES.at(key));

    advance_panel_cursor(PREFERENCES_INNER_XPAD/2);
    do_separator(PREFERENCES_SECTION_H);
    advance_panel_cursor(PREFERENCES_INNER_XPAD/2);
}

TEINAPI void internal__do_half_settings_label (float w, const char* key)
{
    do_label(UI_ALIGN_LEFT,UI_ALIGN_CENTER, w,PREFERENCES_SECTION_H, PREFERENCES_SETTINGS_NAMES.at(key));
    advance_panel_cursor(PREFERENCES_INNER_XPAD/2);
}

TEINAPI void internal__do_half_separator (Vec2& cursor)
{
    advance_panel_cursor((PREFERENCES_INNER_XPAD/2)+1);
    do_separator(PREFERENCES_SECTION_H);
    cursor.x -= 1;
    advance_panel_cursor(PREFERENCES_INNER_XPAD/2);
}

TEINAPI void internal__do_hotkeys_label (float w, const char* key)
{
    do_label(UI_ALIGN_LEFT,UI_ALIGN_CENTER, w,PREFERENCES_SECTION_H, PREFERENCES_HOTKEYS_NAMES.at(key));

    advance_panel_cursor(PREFERENCES_INNER_XPAD/2);
    do_separator(PREFERENCES_SECTION_H);
    advance_panel_cursor(PREFERENCES_INNER_XPAD/2);
}

/* -------------------------------------------------------------------------- */

TEINAPI void internal__begin_settings_area (const char* title, Vec2& cursor)
{
    float w = get_viewport().w;
    float h = PREFERENCES_SECTION_H;

    do_label(UI_ALIGN_CENTER,UI_ALIGN_CENTER, w,h, title);
    internal__next_section(cursor);

    set_panel_cursor_dir(UI_DIR_DOWN);

    advance_panel_cursor(PREFERENCES_INNER_YPAD);
    do_separator(w);
    advance_panel_cursor(PREFERENCES_INNER_YPAD);

    set_panel_cursor_dir(UI_DIR_RIGHT);
    cursor.x = 0;
}

TEINAPI void internal__end_settings_area ()
{
    set_panel_cursor_dir(UI_DIR_DOWN);
    advance_panel_cursor(PREFERENCES_INNER_YPAD);
    set_panel_cursor_dir(UI_DIR_RIGHT);
}

/* -------------------------------------------------------------------------- */

TEINAPI void internal__do_settings_color_swatch (Vec2& cursor, float sw, float sh, Vec4& color)
{
    // If we were presses we want to open the color picker with our color.
    if (preferences_mouse_pressed && mouse_in_ui_bounds_xywh(cursor.x, cursor.y, sw, sh))
    {
        open_color_picker(&color);
    }

    cursor.y += 3;
    set_draw_color(ui_color_light);
    fill_quad(cursor.x+0, cursor.y+0, cursor.x+sw-0, cursor.y+sh-0);
    set_draw_color(ui_color_ex_dark);
    fill_quad(cursor.x+1, cursor.y+1, cursor.x+sw-1, cursor.y+sh-1);

    float x1 = cursor.x   +2;
    float y1 = cursor.y   +2;
    float x2 = cursor.x+sw-2;
    float y2 = cursor.y+sh-2;

    float tw = x2 - x1;
    float th = sh - 4;
    float tx = x1 + (tw/2);
    float ty = y1 + (th/2);

    const Texture& tex = (static_cast<int>(th) % 14 == 0) ? resource_checker_14 : resource_checker_16;

    Quad clip = { 0, 0, tw, th };
    draw_texture(tex, tx, ty, &clip);

    Vec4 max(color.r, color.g, color.b,       1);
    Vec4 min(color.r, color.g, color.b, color.a);

    begin_draw(Buffer_Mode::TRIANGLE_STRIP);
    put_vertex(x1, y2, min); // BL
    put_vertex(x1, y1, min); // TL
    put_vertex(x2, y2, max); // BR
    put_vertex(x2, y1, max); // TR
    end_draw();

    cursor.y -= 3;
    advance_panel_cursor(sw);
}

TEINAPI void internal__do_color_setting_row (Vec2& cursor, float w, const char* key1, const char* key2, Vec4& c1, Vec4& c2)
{
    float lw1 = w-1;
    float lw2 = w;

    float sw = w;
    float sh = PREFERENCES_COLOR_HEIGHT;

    internal__do_half_settings_label(lw1, key1);
    internal__do_settings_color_swatch(cursor, sw, sh, c1);
    internal__do_half_separator(cursor);
    internal__do_half_settings_label(lw2, key2);
    internal__do_settings_color_swatch(cursor, sw, sh, c2);
    internal__next_section(cursor);
}

TEINAPI void internal__do_hotkey_label_titles (Vec2& cursor)
{
    float vw = get_viewport().w-(PREFERENCES_SCROLLBAR_WIDTH-1);
    float sh = PREFERENCES_SECTION_H+(PREFERENCES_INNER_YPAD*2);

    // We use get_viewport().w so the labels are slightly larger and align.
    float lw1 = (roundf((get_viewport().w-(PREFERENCES_INNER_XPAD*2)) / 3) - (PREFERENCES_INNER_XPAD/2));
    float lw2 = roundf((vw-(lw1*2))-roundf((PREFERENCES_INNER_XPAD*2.5f)+2));

    do_quad(vw, sh, ui_color_medium);

    cursor.x  = PREFERENCES_INNER_XPAD;
    cursor.y += PREFERENCES_INNER_YPAD;

    do_label(UI_ALIGN_LEFT,UI_ALIGN_CENTER, lw1,PREFERENCES_SECTION_H, "Hotkey Action");

    advance_panel_cursor(PREFERENCES_INNER_XPAD/2);
    advance_panel_cursor(1);
    advance_panel_cursor(PREFERENCES_INNER_XPAD/2);

    do_label(UI_ALIGN_RIGHT,UI_ALIGN_CENTER, lw2,PREFERENCES_SECTION_H, "Main Binding");
    cursor.x += roundf(PREFERENCES_INNER_XPAD/3);
    do_label(UI_ALIGN_RIGHT,UI_ALIGN_CENTER, lw2,PREFERENCES_SECTION_H, "Alternate Binding");

    cursor.x = 0;

    cursor.y += PREFERENCES_SECTION_H;
    cursor.y += PREFERENCES_INNER_YPAD;

    // To add a separation between hotkey sections.
    ++cursor.y;
}

TEINAPI void internal__do_hotkey_rebind (Vec2& cursor, const char* key)
{
    float vw = get_viewport().w-(PREFERENCES_SCROLLBAR_WIDTH-1);
    float sh = PREFERENCES_SECTION_H;

    // We use get_viewport().w so the labels are slightly larger and align.
    float lw = (roundf((get_viewport().w-(PREFERENCES_INNER_XPAD*2)) / 3) - (PREFERENCES_INNER_XPAD/2));
    float tw = roundf((vw-(lw*2))-roundf((PREFERENCES_INNER_XPAD*2.5f)+2));

    do_quad(vw, sh, ui_color_medium);
    cursor.x = PREFERENCES_INNER_XPAD;

    internal__do_hotkeys_label(lw, key);

    // It says text box inset but we're using for the hotkey rebind as well...
    float th = sh-(PREFERENCES_TEXT_BOX_INSET*2);
    cursor.y += PREFERENCES_TEXT_BOX_INSET;
    do_hotkey_rebind_main(tw,th, UI_NONE, gKeyBindings.at(key));
    cursor.x += roundf(PREFERENCES_INNER_XPAD/3);
    do_hotkey_rebind_alt(tw,th, UI_NONE, gKeyBindings.at(key));
    cursor.y -= PREFERENCES_TEXT_BOX_INSET;

    internal__next_section(cursor);

    // To add a separation between hotkey sections.
    ++cursor.y;
}

/* -------------------------------------------------------------------------- */

TEINAPI void internal__save_settings ()
{
    std::string file_name(MakePathAbsolute(gSettingsFileName));
    FILE* file = fopen(file_name.c_str(), "w");
    if (!file)
    {
        LogError(ERR_MED, "Failed to save settings data!");
        return;
    }
    Defer { fclose(file); };

    Vec4 c;

    if (!gEditorSettings.gamePath.empty())
    {
        fprintf(file, "%s \"%s\"\n", gSettingGamePath, gEditorSettings.gamePath.c_str());
    }
    fprintf(file, "%s %s\n", gSettingUITheme,           gEditorSettings.uiTheme     .c_str());
    fprintf(file, "%s %s\n", gSettingFontFace,          gEditorSettings.fontFace    .c_str());
    fprintf(file, "%s %s\n", gSettingTileGraphics,      gEditorSettings.tileGraphics.c_str());
    fprintf(file, "%s %s\n", gSettingCustomCursors,    (gEditorSettings.customCursors)    ? "true" : "false");
    fprintf(file, "%s %s\n", gSettingShowTooltips,     (gEditorSettings.showTooltips)     ? "true" : "false");
    fprintf(file, "%s %s\n", gSettingUnlimitedBackups, (gEditorSettings.unlimitedBackups) ? "true" : "false");
    fprintf(file, "%s %d\n", gSettingBackupCount,       gEditorSettings.backupCount);
    fprintf(file, "%s %s\n", gSettingAutoBackup,       (gEditorSettings.autoBackup)       ? "true" : "false");
    fprintf(file, "%s %d\n", gSettingBackupInterval,    gEditorSettings.backupInterval);
    if (!gEditorSettings.backgroundColorDefaulted)
    {
        c = gEditorSettings.backgroundColor;
        fprintf(file, "%s [%f %f %f %f]\n", gSettingBackgroundColor, c.r, c.g, c.b, c.a);
    }
    else
    {
        fprintf(file, "%s %s\n", gSettingBackgroundColor, "none");
    }
    c = gEditorSettings.selectColor;
    fprintf(file, "%s [%f %f %f %f]\n", gSettingSelectColor, c.r, c.g, c.b, c.a);
    c = gEditorSettings.outOfBoundsColor;
    fprintf(file, "%s [%f %f %f %f]\n", gSettingOutOfBoundsColor, c.r, c.g, c.b, c.a);
    c = gEditorSettings.cursorColor;
    fprintf(file, "%s [%f %f %f %f]\n", gSettingCursorColor, c.r, c.g, c.b, c.a);
    c = gEditorSettings.mirrorLineColor;
    fprintf(file, "%s [%f %f %f %f]\n", gSettingMirrorLineColor, c.r, c.g, c.b, c.a);
    if (!gEditorSettings.tileGridColorDefaulted)
    {
        c = gEditorSettings.tileGridColor;
        fprintf(file, "%s [%f %f %f %f]\n", gSettingTileGridColor, c.r, c.g, c.b, c.a);
    }
    else
    {
        fprintf(file, "%s %s\n", gSettingTileGridColor, "none");
    }

    fprintf(file, "\n");

    // Save all of the hotkeys to GON format.
    for (auto& [name,kb]: gKeyBindings)
    {
        fprintf(file, "%s { ", name.c_str());
        fprintf(file, "main [");
        if (kb.mod)
        {
            if (kb.mod&KMOD_CTRL ) fprintf(file, "\"Ctrl\" " );
            if (kb.mod&KMOD_ALT  ) fprintf(file, "\"Alt\" "  );
            if (kb.mod&KMOD_MODE ) fprintf(file, "\"AltGr\" ");
            if (kb.mod&KMOD_SHIFT) fprintf(file, "\"Shift\" ");
            if (kb.mod&KMOD_GUI  ) fprintf(file, "\"Gui\" "  );
        }
        if (kb.code)
        {
            fprintf(file, "\"%s\"", SDL_GetKeyName(kb.code));
        }
        fprintf(file, "] ");
        if (kb.hasAlt)
        {
            fprintf(file, "alt [");
            if (kb.altMod)
            {
                if (kb.altMod & KMOD_CTRL ) fprintf(file, "\"Ctrl\" " );
                if (kb.altMod & KMOD_ALT  ) fprintf(file, "\"Alt\" "  );
                if (kb.altMod & KMOD_MODE ) fprintf(file, "\"AltGr\" ");
                if (kb.altMod & KMOD_SHIFT) fprintf(file, "\"Shift\" ");
                if (kb.altMod & KMOD_GUI  ) fprintf(file, "\"Gui\" "  );
            }
            if (kb.altCode)
            {
                fprintf(file, "\"%s\"", SDL_GetKeyName(kb.altCode));
            }
            fprintf(file, "] ");
        }
        fprintf(file, "}\n");
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void internal__do_preferences_settings ()
{
    float vw = get_viewport().w;
    float vh = get_viewport().h;

    float xpad = PREFERENCES_INNER_XPAD;
    float ypad = PREFERENCES_INNER_YPAD;

    begin_panel(0, 0, vw, vh, UI_NONE, ui_color_medium);
    begin_panel(xpad, ypad, vw-(xpad*2), vh-(ypad*2), UI_NONE);

    // We redefine as these will now be different.
    vw = get_viewport().w;
    vh = get_viewport().h;

    float sw = roundf(vw / 2) - (xpad/2);
    float sh = PREFERENCES_SECTION_H;

    float bw = roundf(sw / 2);
    float th = sh-(PREFERENCES_TEXT_BOX_INSET*2);

    Vec2 cursor(0,0);

    set_panel_cursor_dir(UI_DIR_RIGHT);
    set_panel_cursor(&cursor);

    internal__begin_settings_area("User Interface", cursor);

    internal__do_settings_label(sw, gSettingUITheme);
    UI_Flag dark_flags  = (is_ui_light()) ? UI_INACTIVE : UI_NONE;
    UI_Flag light_flags = (is_ui_light()) ? UI_NONE : UI_INACTIVE;
    if (do_button_txt(NULL, bw,sh, dark_flags,  "Dark"))
    {
        gEditorSettings.uiTheme = "dark";
        load_ui_theme();
    }
    if (do_button_txt(NULL, bw,sh, light_flags, "Light"))
    {
        gEditorSettings.uiTheme = "light";
        load_ui_theme();
    }
    internal__next_section(cursor);

    internal__do_settings_label(sw, gSettingFontFace);
    UI_Flag sans_flags = (is_editor_font_opensans()) ? UI_NONE : UI_INACTIVE;
    UI_Flag dysl_flags = (is_editor_font_opensans()) ? UI_INACTIVE : UI_NONE;
    set_ui_font(&resource_font_regular_sans);
    if (do_button_txt(NULL, bw,sh, sans_flags, "OpenSans"))
    {
        gEditorSettings.fontFace = "OpenSans";
        update_editor_font();
    }
    set_ui_font(&resource_font_regular_dyslexic);
    if (do_button_txt(NULL, bw,sh, dysl_flags, "OpenDyslexic"))
    {
        gEditorSettings.fontFace = "OpenDyslexic";
        update_editor_font();
    }
    internal__next_section(cursor);

    // Reset the font to being the default for the editor.
    set_ui_font(&get_editor_regular_font());

    internal__do_settings_label(sw, gSettingTileGraphics);
    UI_Flag tile_graphics_new_flags = (gEditorSettings.tileGraphics == "new") ? UI_NONE : UI_INACTIVE;
    UI_Flag tile_graphics_old_flags = (gEditorSettings.tileGraphics == "new") ? UI_INACTIVE : UI_NONE;
    if (do_button_txt(NULL, bw,sh, tile_graphics_new_flags, "New"))
    {
        if (gEditorSettings.tileGraphics != "new")
        {
            gEditorSettings.tileGraphics = "new";
            reload_tile_graphics();
        }
    }
    if (do_button_txt(NULL, bw,sh, tile_graphics_old_flags, "Old"))
    {
        if (gEditorSettings.tileGraphics != "old")
        {
            gEditorSettings.tileGraphics = "old";
            reload_tile_graphics();
        }
    }
    internal__next_section(cursor);

    internal__do_settings_label(sw, gSettingCustomCursors);
    UI_Flag cursor_enabled_flags  = (CustomCursorsEnabled()) ? UI_NONE : UI_INACTIVE;
    UI_Flag cursor_disabled_flags = (CustomCursorsEnabled()) ? UI_INACTIVE : UI_NONE;
    if (do_button_txt(NULL, bw,sh, cursor_enabled_flags,  "Enabled"))
    {
        gEditorSettings.customCursors = true;
        LoadEditorCursors();
    }
    if (do_button_txt(NULL, bw,sh, cursor_disabled_flags, "Disabled"))
    {
        gEditorSettings.customCursors = false;
        LoadEditorCursors();
    }
    internal__next_section(cursor);

    UI_Flag tooltips_enabled_flags  = (gEditorSettings.showTooltips) ? UI_NONE : UI_INACTIVE;
    UI_Flag tooltips_disabled_flags = (gEditorSettings.showTooltips) ? UI_INACTIVE : UI_NONE;
    internal__do_settings_label(sw, gSettingShowTooltips);
    if (do_button_txt(NULL, bw,sh, tooltips_enabled_flags,  "Enabled"))
    {
        gEditorSettings.showTooltips = true;
    }
    if (do_button_txt(NULL, bw,sh, tooltips_disabled_flags, "Disabled"))
    {
        gEditorSettings.showTooltips = false;
    }
    internal__next_section(cursor);

    internal__end_settings_area();

    internal__begin_settings_area("Level Backups", cursor);

    internal__do_settings_label(sw, gSettingAutoBackup);
    UI_Flag backup_enabled_flags  = (gEditorSettings.autoBackup) ? UI_NONE : UI_INACTIVE;
    UI_Flag backup_disabled_flags = (gEditorSettings.autoBackup) ? UI_INACTIVE : UI_NONE;
    if (do_button_txt(NULL, bw,sh, backup_enabled_flags,  "Enabled"))
    {
        gEditorSettings.autoBackup = true;
        update_backup_timer();
    }
    if (do_button_txt(NULL, bw,sh, backup_disabled_flags, "Disabled"))
    {
        gEditorSettings.autoBackup = false;
        update_backup_timer();
    }
    internal__next_section(cursor);

    internal__do_settings_label(sw, gSettingUnlimitedBackups);
    UI_Flag unlimited_enabled_flags  = (gEditorSettings.unlimitedBackups) ? UI_NONE : UI_INACTIVE;
    UI_Flag unlimited_disabled_flags = (gEditorSettings.unlimitedBackups) ? UI_INACTIVE : UI_NONE;
    if (do_button_txt(NULL, bw,sh, unlimited_enabled_flags,  "Enabled"))
    {
        gEditorSettings.unlimitedBackups = true;
    }
    if (do_button_txt(NULL, bw,sh, unlimited_disabled_flags, "Disabled"))
    {
        gEditorSettings.unlimitedBackups = false;
    }
    internal__next_section(cursor);

    internal__do_settings_label(sw, gSettingBackupInterval);
    if (!gEditorSettings.autoBackup) set_panel_flags(UI_LOCKED);
    cursor.y += PREFERENCES_TEXT_BOX_INSET;
    std::string backup_interval_str(std::to_string(gEditorSettings.backupInterval));
    do_text_box(vw-cursor.x,th, UI_NUMERIC, backup_interval_str, "0");
    cursor.y -= PREFERENCES_TEXT_BOX_INSET;
    if (!gEditorSettings.autoBackup) set_panel_flags(UI_NONE);
    if (atoll(backup_interval_str.c_str()) > INT_MAX)
    {
        backup_interval_str = std::to_string(INT_MAX);
    }
    int backup_interval = atoi(backup_interval_str.c_str());
    if (backup_interval != gEditorSettings.backupInterval)
    {
        gEditorSettings.backupInterval = backup_interval;
        update_backup_timer();
    }
    internal__next_section(cursor);

    internal__do_settings_label(sw, gSettingBackupCount);
    if (gEditorSettings.unlimitedBackups) set_panel_flags(UI_LOCKED);
    cursor.y += PREFERENCES_TEXT_BOX_INSET;
    std::string backup_count_str(std::to_string(gEditorSettings.backupCount));
    do_text_box(vw-cursor.x,th, UI_NUMERIC, backup_count_str, "0");
    cursor.y -= PREFERENCES_TEXT_BOX_INSET;
    if (gEditorSettings.unlimitedBackups) set_panel_flags(UI_NONE);
    if (atoll(backup_count_str.c_str()) > INT_MAX)
    {
        backup_count_str = std::to_string(INT_MAX);
    }
    int backup_count = atoi(backup_count_str.c_str());
    if (backup_count != gEditorSettings.backupCount)
    {
        gEditorSettings.backupCount = backup_count;
    }
    internal__next_section(cursor);

    internal__end_settings_area();

    internal__begin_settings_area("Custom Colors", cursor);

    float hw = roundf(vw/4) - (xpad/2);

    internal__do_color_setting_row(cursor, hw, gSettingBackgroundColor, gSettingTileGridColor   , gEditorSettings.backgroundColor, gEditorSettings.tileGridColor   );
    internal__do_color_setting_row(cursor, hw, gSettingSelectColor    , gSettingOutOfBoundsColor, gEditorSettings.selectColor    , gEditorSettings.outOfBoundsColor);
    internal__do_color_setting_row(cursor, hw, gSettingCursorColor    , gSettingMirrorLineColor , gEditorSettings.cursorColor    , gEditorSettings.mirrorLineColor );

    internal__end_settings_area();

    end_panel();
    end_panel();
}

/* -------------------------------------------------------------------------- */

TEINAPI void internal__do_preferences_hotkeys ()
{
    float vw = get_viewport().w;
    float vh = get_viewport().h;

    begin_panel(0, 0, vw, vh, UI_NONE);

    Vec2 cursor(0,0);

    set_panel_cursor_dir(UI_DIR_RIGHT);
    set_panel_cursor(&cursor);

    float content_height = (PREFERENCES_HOTKEYS_NAMES.size() * (PREFERENCES_SECTION_H+1)) + (PREFERENCES_SECTION_H+(PREFERENCES_INNER_YPAD*2));
    do_scrollbar(vw-PREFERENCES_SCROLLBAR_WIDTH+1, -1, PREFERENCES_SCROLLBAR_WIDTH, vh+2, content_height, preferences_scroll_offset);

    internal__do_hotkey_label_titles(cursor);

    internal__do_hotkey_rebind(cursor, gKbLevelNew);
    internal__do_hotkey_rebind(cursor, gKbLevelOpen);
    internal__do_hotkey_rebind(cursor, gKbLevelSave);
    internal__do_hotkey_rebind(cursor, gKbLevelSaveAs);
    internal__do_hotkey_rebind(cursor, gKbLevelClose);
    internal__do_hotkey_rebind(cursor, gKbLevelCloseAll);
    internal__do_hotkey_rebind(cursor, gKbLevelResize);
    internal__do_hotkey_rebind(cursor, gKbUndo);
    internal__do_hotkey_rebind(cursor, gKbRedo);
    internal__do_hotkey_rebind(cursor, gKbHistoryBegin);
    internal__do_hotkey_rebind(cursor, gKbHistoryEnd);
    internal__do_hotkey_rebind(cursor, gKbCameraZoomOut);
    internal__do_hotkey_rebind(cursor, gKbCameraZoomIn);
    internal__do_hotkey_rebind(cursor, gKbRunGame);
    internal__do_hotkey_rebind(cursor, gKbPreferences);
    internal__do_hotkey_rebind(cursor, gKbAbout);
    internal__do_hotkey_rebind(cursor, gKbBugReport);
    internal__do_hotkey_rebind(cursor, gKbHelp);
    internal__do_hotkey_rebind(cursor, gKbToolBrush);
    internal__do_hotkey_rebind(cursor, gKbToolFill);
    internal__do_hotkey_rebind(cursor, gKbToolSelect);
    internal__do_hotkey_rebind(cursor, gKbFlipH);
    internal__do_hotkey_rebind(cursor, gKbFlipV);
    internal__do_hotkey_rebind(cursor, gKbMirrorHToggle);
    internal__do_hotkey_rebind(cursor, gKbMirrorVToggle);
    internal__do_hotkey_rebind(cursor, gKbGpakUnpack);
    internal__do_hotkey_rebind(cursor, gKbGpakPack);
    internal__do_hotkey_rebind(cursor, gKbToggleEntity);
    internal__do_hotkey_rebind(cursor, gKbToggleGuides);
    internal__do_hotkey_rebind(cursor, gKbGridToggle);
    internal__do_hotkey_rebind(cursor, gKbBoundsToggle);
    internal__do_hotkey_rebind(cursor, gKbLayersToggle);
    internal__do_hotkey_rebind(cursor, gKbCameraReset);
    internal__do_hotkey_rebind(cursor, gKbClearSelect);
    internal__do_hotkey_rebind(cursor, gKbDeselect);
    internal__do_hotkey_rebind(cursor, gKbSelectAll);
    internal__do_hotkey_rebind(cursor, gKbCopy);
    internal__do_hotkey_rebind(cursor, gKbCut);
    internal__do_hotkey_rebind(cursor, gKbPaste);
    internal__do_hotkey_rebind(cursor, gKbIncrementTile);
    internal__do_hotkey_rebind(cursor, gKbDecrementTile);
    internal__do_hotkey_rebind(cursor, gKbIncrementGroup);
    internal__do_hotkey_rebind(cursor, gKbDecrementGroup);
    internal__do_hotkey_rebind(cursor, gKbIncrementCategory);
    internal__do_hotkey_rebind(cursor, gKbDecrementCategory);
    internal__do_hotkey_rebind(cursor, gKbToggleLayerActive);
    internal__do_hotkey_rebind(cursor, gKbToggleLayerTag);
    internal__do_hotkey_rebind(cursor, gKbToggleLayerOverlay);
    internal__do_hotkey_rebind(cursor, gKbToggleLayerBack1);
    internal__do_hotkey_rebind(cursor, gKbToggleLayerBack2);
    internal__do_hotkey_rebind(cursor, gKbCategoryBasic);
    internal__do_hotkey_rebind(cursor, gKbCategoryTag);
    internal__do_hotkey_rebind(cursor, gKbCategoryOverlay);
    internal__do_hotkey_rebind(cursor, gKbCategoryEntity);
    internal__do_hotkey_rebind(cursor, gKbCategoryBack1);
    internal__do_hotkey_rebind(cursor, gKbCategoryBack2);
    internal__do_hotkey_rebind(cursor, gKbIncrementTab);
    internal__do_hotkey_rebind(cursor, gKbDecrementTab);
    internal__do_hotkey_rebind(cursor, gKbMoveTabLeft);
    internal__do_hotkey_rebind(cursor, gKbMoveTabRight);
    internal__do_hotkey_rebind(cursor, gKbOpenRecentTab);
    internal__do_hotkey_rebind(cursor, gKbLoadPrevLevel);
    internal__do_hotkey_rebind(cursor, gKbLoadNextLevel);

    end_panel();
}

/* -------------------------------------------------------------------------- */

TEINAPI void init_preferences_menu ()
{
    cached_editor_settings = gEditorSettings;
    cached_editor_hotkeys  = gKeyBindings;

    color_picker_mouse_pressed = false;
}

TEINAPI void do_preferences_menu ()
{
    Quad p1, p2;

    p1.x = gWindowBorder;
    p1.y = gWindowBorder;
    p1.w = get_viewport().w - (gWindowBorder * 2);
    p1.h = get_viewport().h - (gWindowBorder * 2);

    set_ui_font(&get_editor_regular_font());

    begin_panel(p1, UI_NONE, ui_color_ex_dark);

    Vec2 cursor;

    float pvfh = PREFERENCES_V_FRAME_H;

    float vw = get_viewport().w;
    float vh = get_viewport().h;

    float tw = roundf(vw / 2);
    float th = pvfh - gWindowBorder;

    float bw = roundf(vw / 3);
    float bh = pvfh - gWindowBorder;

    // Top tabs for switching from the settings and key bindings menu.
    cursor = Vec2(0,0);
    begin_panel(0, 0, vw, pvfh, UI_NONE, ui_color_medium);

    set_panel_cursor_dir(UI_DIR_RIGHT);
    set_panel_cursor(&cursor);

    UI_Flag settings_flags = (preferences_tab == Preferennes_Tab::SETTINGS) ? UI_HIGHLIGHT : UI_INACTIVE;
    UI_Flag hotkeys_flags  = (preferences_tab == Preferennes_Tab::HOTKEYS)  ? UI_HIGHLIGHT : UI_INACTIVE;

    if (do_button_txt(NULL, tw,th, settings_flags, "Settings")) preferences_tab = Preferennes_Tab::SETTINGS;
    if (do_button_txt(NULL, tw,th, hotkeys_flags,  "Hotkeys"))  preferences_tab = Preferennes_Tab::HOTKEYS;

    // Just in case of weird rounding manually add the right separator.
    cursor.x = vw;
    do_separator(bh);

    // Add a separator to the left for symmetry.
    cursor.x = 1;
    do_separator(bh);

    end_panel();

    // Bottom buttons for saving and exiting or cancelling changes.
    cursor = Vec2(0, gWindowBorder);
    begin_panel(0, vh-pvfh, vw, pvfh, UI_NONE, ui_color_medium);

    set_panel_cursor_dir(UI_DIR_RIGHT);
    set_panel_cursor(&cursor);

    std::string restore_message;
    switch (preferences_tab)
    {
        case (Preferennes_Tab::SETTINGS): restore_message = "Restore Settings"; break;
        case (Preferennes_Tab::HOTKEYS):  restore_message = "Restore Hotkeys";  break;
    }

    // Just to make sure that we always reach the end of the panel space.
    float bw2 = vw - (bw*2);

    if (do_button_txt(NULL, bw ,bh, UI_NONE, "Save and Exit"  )) save_preferences   ();
    if (do_button_txt(NULL, bw ,bh, UI_NONE, restore_message  )) restore_preferences();
    if (do_button_txt(NULL, bw2,bh, UI_NONE, "Cancel and Exit")) cancel_preferences ();

    // Add a separator to the left for symmetry.
    cursor.x = 1;
    do_separator(bh);

    end_panel();

    p2.x =                                  1;
    p2.y = pvfh                           + 1;
    p2.w = get_viewport().w               - 2;
    p2.h = get_viewport().h - p2.y - pvfh - 1;

    begin_panel(p2, UI_NONE);
    switch (preferences_tab)
    {
        case (Preferennes_Tab::SETTINGS): internal__do_preferences_settings(); break;
        case (Preferennes_Tab::HOTKEYS):  internal__do_preferences_hotkeys();  break;
    }
    end_panel();

    end_panel();
}

/* -------------------------------------------------------------------------- */

TEINAPI void handle_preferences_menu_events ()
{
    // Check if there was a mouse press for the color swatch settings.
    preferences_mouse_pressed = false;

    if (!IsWindowFocused("WINPREFERENCES")) return;

    switch (main_event.type)
    {
        case (SDL_MOUSEBUTTONDOWN):
        {
            if (main_event.button.button == SDL_BUTTON_LEFT)
            {
                preferences_mouse_pressed = true;
            }
        } break;
        case (SDL_KEYDOWN):
        {
            if (!text_box_is_active() && !hotkey_is_active())
            {
                switch (main_event.key.keysym.sym)
                {
                    case (SDLK_RETURN): save_preferences();   break;
                    case (SDLK_ESCAPE): cancel_preferences(); break;
                }
            }
        } break;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void restore_preferences ()
{
    switch (preferences_tab)
    {
        case (Preferennes_Tab::SETTINGS): RestoreEditorSettings(); break;
        case (Preferennes_Tab::HOTKEYS): RestoreEditorKeyBindings(); break;
    }
}

TEINAPI void cancel_preferences ()
{
    // We only want to shot this prompt if the user actually made any changes to preferences.
    if (gEditorSettings != cached_editor_settings || gKeyBindings != cached_editor_hotkeys)
    {
        // Make sure the user is certain about what they are doing and cancel the
        // action if they decide that they do not actually want to discard changes.
        if (ShowAlert("Discard Changes", "Are you sure you want do discard changes?", ALERT_TYPE_WARNING,
            ALERT_BUTTON_YES_NO, "WINPREFERENCES") == ALERT_RESULT_NO)
        {
            return;
        }
    }

    bool tile_graphics_changed = (gEditorSettings.tileGraphics != cached_editor_settings.tileGraphics);

    // Restore the settings/hotkeys states from before modifying preferences.
    gEditorSettings = cached_editor_settings;
    gKeyBindings = cached_editor_hotkeys;

    UpdateSystemsThatRelyOnSettings(tile_graphics_changed);

    HideWindow("WINCOLOR");
    HideWindow("WINPREFERENCES");
}

TEINAPI void save_preferences ()
{
    // No point saving unless there were changes.
    if (gEditorSettings != cached_editor_settings || gKeyBindings != cached_editor_hotkeys)
    {
        internal__save_settings();
    }

    HideWindow("WINCOLOR");
    HideWindow("WINPREFERENCES");
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
