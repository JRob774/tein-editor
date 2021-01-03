static const std::map<std::string,const char*> gPreferencesSettingsNames
{
{ gSettingUITheme, "Theme" },
{ gSettingFontFace, "Font" },
{ gSettingTileGraphics, "Tile Graphics" },
{ gSettingCustomCursors, "Cursors" },
{ gSettingShowTooltips, "Tooltips" },
{ gSettingUnlimitedBackups, "Unlimited Backups" },
{ gSettingBackupCount, "Backups Per Level" },
{ gSettingAutoBackup, "Automatic Backups" },
{ gSettingBackupInterval, "Auto-Backup Time" },
{ gSettingBackgroundColor, "Background" },
{ gSettingSelectColor, "Select" },
{ gSettingOutOfBoundsColor, "Out of Bounds" },
{ gSettingCursorColor, "Cursor" },
{ gSettingMirrorLineColor, "Mirror Lines" },
{ gSettingTileGridColor, "Tile Grid" }
};

static const std::map<std::string,const char*> gPreferencesHotkeysNames
{
{ gKbLevelNew, "New" },
{ gKbLevelOpen, "Load" },
{ gKbLevelSave, "Save" },
{ gKbLevelSaveAs, "Save As" },
{ gKbLevelClose, "Close" },
{ gKbLevelCloseAll, "Close All" },
{ gKbLevelResize, "Resize" },
{ gKbUndo, "Undo" },
{ gKbRedo, "Redo" },
{ gKbHistoryBegin, "History Begin" },
{ gKbHistoryEnd, "History End" },
{ gKbCameraZoomOut, "Zoom Out" },
{ gKbCameraZoomIn, "Zoom In" },
{ gKbRunGame, "Run Game" },
{ gKbPreferences, "Preferences" },
{ gKbAbout, "About" },
{ gKbBugReport, "Bug Report" },
{ gKbHelp, "Help" },
{ gKbToolBrush, "Brush" },
{ gKbToolFill, "Fill" },
{ gKbToolSelect, "Select" },
{ gKbFlipH, "Flip Horizontal" },
{ gKbFlipV, "Flip Vertical" },
{ gKbMirrorHToggle, "Mirror Horizontal" },
{ gKbMirrorVToggle, "Mirror Vertical" },
{ gKbGpakUnpack, "Unpack GPAK" },
{ gKbGpakPack, "Pack GPAK" },
{ gKbToggleEntity, "Toggle Large Entities" },
{ gKbToggleGuides, "Toggle Entity Guides" },
{ gKbGridToggle, "Toggle Tile Grid" },
{ gKbBoundsToggle, "Toggle Out of Bounds" },
{ gKbLayersToggle, "Toggle Transparency" },
{ gKbCameraReset, "Reset Camera" },
{ gKbClearSelect, "Clear Selection" },
{ gKbDeselect, "Select Box Deselect" },
{ gKbSelectAll, "Cursor Deselect" },
{ gKbCopy, "Copy" },
{ gKbCut, "Cut" },
{ gKbPaste, "Paste" },
{ gKbIncrementTile, "Next Selected Tile" },
{ gKbDecrementTile, "Prev Selected Tile" },
{ gKbIncrementGroup, "Next Selected Group" },
{ gKbDecrementGroup, "Prev Selected Group" },
{ gKbIncrementCategory, "Next Selected Category" },
{ gKbDecrementCategory, "Prev Selected Category" },
{ gKbToggleLayerActive, "Toggle Active Layer" },
{ gKbToggleLayerTag, "Toggle Tag Layer" },
{ gKbToggleLayerOverlay, "Toggle Overlay Layer" },
{ gKbToggleLayerBack1, "Toggle Back 1 Layer" },
{ gKbToggleLayerBack2, "Toggle Back 2 Layer" },
{ gKbCategoryBasic, "Category Basic" },
{ gKbCategoryTag, "Category Tag" },
{ gKbCategoryOverlay, "Category Overlay" },
{ gKbCategoryEntity, "Category Entity" },
{ gKbCategoryBack1, "Category Back 1" },
{ gKbCategoryBack2, "Category Back 2" },
{ gKbIncrementTab, "Next Tab" },
{ gKbDecrementTab, "Prev Tab" },
{ gKbMoveTabLeft, "Move Tab Left" },
{ gKbMoveTabRight, "Move Tab Right" },
{ gKbOpenRecentTab, "Open Recent Tab" },
{ gKbLoadPrevLevel, "Load Next Level" },
{ gKbLoadNextLevel, "Load Prev Level" }
};

static constexpr float gPreferencesVerticalFrameHeight = 26;
static constexpr float gPreferencesSectionHeight = 24;
static constexpr float gPreferencesInnerXPad = 10;
static constexpr float gPreferencesInnerYPad = 5;
static constexpr float gPreferencesTextBoxInset = 2;
static constexpr float gPreferencesScrollbarWidth = 12;
static constexpr float gPreferencesColorHeight = 18;

enum class PreferencesTab { Settings, Hotkeys };

static PreferencesTab gPreferencesTab = PreferencesTab::Settings;

static float gPreferencesScrollOffset = 0;
static bool gPreferencesMousePressed = false;

// When the preferences menu is opened we cache the current settings and
// hotkey states. This allows the preferences menu to modify the current
// settings and hotkeys immediately for instant feedback. If the user
// then wants to cancel the changes made, we can just restore the cached
// versions of the settings and hotkeys to return to the original state.

static Settings gCachedEditorSettings;
static std::map<std::string,KeyBinding> gCachedEditorHotkeys;

namespace Internal
{
    TEINAPI void NextSection (Vec2& cursor)
    {
        cursor.x = 0;
        cursor.y += gPreferencesSectionHeight;
    }

    TEINAPI void DoSettingsLabel (float w, const char* key)
    {
        DoLabel(UiAlign::Left,UiAlign::Center, w,gPreferencesSectionHeight, gPreferencesSettingsNames.at(key));
        AdvancePanelCursor(gPreferencesInnerXPad/2);
        DoSeparator(gPreferencesSectionHeight);
        AdvancePanelCursor(gPreferencesInnerXPad/2);
    }

    TEINAPI void DoHalfSettingsLabel (float w, const char* key)
    {
        DoLabel(UiAlign::Left,UiAlign::Center, w,gPreferencesSectionHeight, gPreferencesSettingsNames.at(key));
        AdvancePanelCursor(gPreferencesInnerXPad/2);
    }

    TEINAPI void DoHalfSeparator (Vec2& cursor)
    {
        AdvancePanelCursor((gPreferencesInnerXPad/2)+1);
        DoSeparator(gPreferencesSectionHeight);
        cursor.x -= 1;
        AdvancePanelCursor(gPreferencesInnerXPad/2);
    }

    TEINAPI void DoHotkeysLabel (float w, const char* key)
    {
        DoLabel(UiAlign::Left,UiAlign::Center, w,gPreferencesSectionHeight, gPreferencesHotkeysNames.at(key));
        AdvancePanelCursor(gPreferencesInnerXPad/2);
        DoSeparator(gPreferencesSectionHeight);
        AdvancePanelCursor(gPreferencesInnerXPad/2);
    }

    TEINAPI void BeginSettingsArea (const char* title, Vec2& cursor)
    {
        float w = GetViewport().w;
        float h = gPreferencesSectionHeight;

        DoLabel(UiAlign::Center,UiAlign::Center, w,h, title);
        NextSection(cursor);

        SetPanelCursorDir(UiDir::Down);

        AdvancePanelCursor(gPreferencesInnerYPad);
        DoSeparator(w);
        AdvancePanelCursor(gPreferencesInnerYPad);

        SetPanelCursorDir(UiDir::Right);
        cursor.x = 0;
    }

    TEINAPI void EndSettingsArea ()
    {
        SetPanelCursorDir(UiDir::Down);
        AdvancePanelCursor(gPreferencesInnerYPad);
        SetPanelCursorDir(UiDir::Right);
    }

    TEINAPI void DoSettingsColorSwatch (Vec2& cursor, float sw, float sh, Vec4& color)
    {
        // If we were presses we want to open the color picker with our color.
        if (gPreferencesMousePressed && MouseInUiBoundsXYWH(cursor.x, cursor.y, sw, sh)) OpenColorPicker(&color);

        cursor.y += 3;
        SetDrawColor(gUiColorLight);
        FillQuad(cursor.x+0, cursor.y+0, cursor.x+sw-0, cursor.y+sh-0);
        SetDrawColor(gUiColorExDark);
        FillQuad(cursor.x+1, cursor.y+1, cursor.x+sw-1, cursor.y+sh-1);

        float x1 = cursor.x+2;
        float y1 = cursor.y+2;
        float x2 = cursor.x+sw-2;
        float y2 = cursor.y+sh-2;

        float tw = x2 - x1;
        float th = sh - 4;
        float tx = x1 + (tw/2);
        float ty = y1 + (th/2);

        const Texture& texture = (static_cast<int>(th) % 14 == 0) ? gResourceChecker14 : gResourceChecker16;

        Quad clip = { 0,0,tw,th };
        DrawTexture(texture, tx, ty, &clip);

        Vec4 max = color;
        Vec4 min = color;

        max.a = 1;

        BeginDraw(BufferMode::TriangleStrip);
        PutVertex(x1, y2, min); // BL
        PutVertex(x1, y1, min); // TL
        PutVertex(x2, y2, max); // BR
        PutVertex(x2, y1, max); // TR
        EndDraw();

        cursor.y -= 3;
        AdvancePanelCursor(sw);
    }

    TEINAPI void DoColorSettingRow (Vec2& cursor, float w, const char* key1, const char* key2, Vec4& c1, Vec4& c2)
    {
        float lw1 = w-1;
        float lw2 = w;

        float sw = w;
        float sh = gPreferencesColorHeight;

        DoHalfSettingsLabel(lw1, key1);
        DoSettingsColorSwatch(cursor, sw, sh, c1);
        DoHalfSeparator(cursor);
        DoHalfSettingsLabel(lw2, key2);
        DoSettingsColorSwatch(cursor, sw, sh, c2);
        NextSection(cursor);
    }

    TEINAPI void DoHotkeyLabelTitles (Vec2& cursor)
    {
        float vw = GetViewport().w-(gPreferencesScrollbarWidth-1);
        float sh = gPreferencesSectionHeight+(gPreferencesInnerYPad*2);

        // We use GetViewport().w so the labels are slightly larger and align.
        float lw1 = (roundf((GetViewport().w-(gPreferencesInnerXPad*2)) / 3) - (gPreferencesInnerXPad/2));
        float lw2 = roundf((vw-(lw1*2))-roundf((gPreferencesInnerXPad*2.5f)+2));

        DoQuad(vw, sh, gUiColorMedium);

        cursor.x = gPreferencesInnerXPad;
        cursor.y += gPreferencesInnerYPad;

        DoLabel(UiAlign::Left,UiAlign::Center, lw1,gPreferencesSectionHeight, "Hotkey Action");

        AdvancePanelCursor(gPreferencesInnerXPad/2);
        AdvancePanelCursor(1);
        AdvancePanelCursor(gPreferencesInnerXPad/2);

        DoLabel(UiAlign::Right,UiAlign::Center, lw2,gPreferencesSectionHeight, "Main Binding");
        cursor.x += roundf(gPreferencesInnerXPad/3);
        DoLabel(UiAlign::Right,UiAlign::Center, lw2,gPreferencesSectionHeight, "Alternate Binding");

        cursor.x = 0;

        cursor.y += gPreferencesSectionHeight;
        cursor.y += gPreferencesInnerYPad;

        // To add a separation between hotkey sections.
        ++cursor.y;
    }

    TEINAPI void DoHotkeyRebind (Vec2& cursor, const char* key)
    {
        float vw = GetViewport().w-(gPreferencesScrollbarWidth-1);
        float sh = gPreferencesSectionHeight;

        // We use GetViewport().w so the labels are slightly larger and align.
        float lw = (roundf((GetViewport().w-(gPreferencesInnerXPad*2)) / 3) - (gPreferencesInnerXPad/2));
        float tw = roundf((vw-(lw*2))-roundf((gPreferencesInnerXPad*2.5f)+2));

        DoQuad(vw, sh, gUiColorMedium);
        cursor.x = gPreferencesInnerXPad;

        DoHotkeysLabel(lw, key);

        // It says text box inset but we're using for the hotkey rebind as well...
        float th = sh-(gPreferencesTextBoxInset*2);
        cursor.y += gPreferencesTextBoxInset;
        DoHotkeyRebindMain(tw,th, UiFlag::None, gKeyBindings.at(key));
        cursor.x += roundf(gPreferencesInnerXPad/3);
        DoHotkeyRebindAlt(tw,th, UiFlag::None, gKeyBindings.at(key));
        cursor.y -= gPreferencesTextBoxInset;

        NextSection(cursor);

        // To add a separation between hotkey sections.
        ++cursor.y;
    }

    TEINAPI void SaveSettings ()
    {
        std::string fileName(MakePathAbsolute(gSettingsFileName));
        FILE* file = fopen(fileName.c_str(), "w");
        if (!file) {
            LogError(ErrorLevel::Med, "Failed to save settings data!");
            return;
        }
        Defer { fclose(file); };

        Vec4 c;

        if (!gEditorSettings.gamePath.empty()) {
            fprintf(file, "%s \"%s\"\n", gSettingGamePath, gEditorSettings.gamePath.c_str());
        }
        fprintf(file, "%s %s\n", gSettingUITheme, gEditorSettings.uiTheme.c_str());
        fprintf(file, "%s %s\n", gSettingFontFace, gEditorSettings.fontFace.c_str());
        fprintf(file, "%s %s\n", gSettingTileGraphics, gEditorSettings.tileGraphics.c_str());
        fprintf(file, "%s %s\n", gSettingCustomCursors, (gEditorSettings.customCursors) ? "true" : "false");
        fprintf(file, "%s %s\n", gSettingShowTooltips, (gEditorSettings.showTooltips) ? "true" : "false");
        fprintf(file, "%s %s\n", gSettingUnlimitedBackups, (gEditorSettings.unlimitedBackups) ? "true" : "false");
        fprintf(file, "%s %d\n", gSettingBackupCount, gEditorSettings.backupCount);
        fprintf(file, "%s %s\n", gSettingAutoBackup, (gEditorSettings.autoBackup) ? "true" : "false");
        fprintf(file, "%s %d\n", gSettingBackupInterval, gEditorSettings.backupInterval);
        if (!gEditorSettings.backgroundColorDefaulted) {
            c = gEditorSettings.backgroundColor;
            fprintf(file, "%s [%f %f %f %f]\n", gSettingBackgroundColor, c.r, c.g, c.b, c.a);
        } else {
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
        if (!gEditorSettings.tileGridColorDefaulted) {
            c = gEditorSettings.tileGridColor;
            fprintf(file, "%s [%f %f %f %f]\n", gSettingTileGridColor, c.r, c.g, c.b, c.a);
        } else {
            fprintf(file, "%s %s\n", gSettingTileGridColor, "none");
        }

        fprintf(file, "\n");

        // Save all of the hotkeys to GON format.
        for (auto& [name,kb]: gKeyBindings) {
            fprintf(file, "%s { ", name.c_str());
            fprintf(file, "main [");
            if (kb.mod) {
                if (kb.mod&KMOD_CTRL) fprintf(file, "\"Ctrl\" ");
                if (kb.mod&KMOD_ALT) fprintf(file, "\"Alt\" ");
                if (kb.mod&KMOD_MODE) fprintf(file, "\"AltGr\" ");
                if (kb.mod&KMOD_SHIFT) fprintf(file, "\"Shift\" ");
                if (kb.mod&KMOD_GUI) fprintf(file, "\"Gui\" ");
            }
            if (kb.code) {
                fprintf(file, "\"%s\"", SDL_GetKeyName(kb.code));
            }
            fprintf(file, "] ");
            if (kb.hasAlt) {
                fprintf(file, "alt [");
                if (kb.altMod) {
                    if (kb.altMod & KMOD_CTRL) fprintf(file, "\"Ctrl\" ");
                    if (kb.altMod & KMOD_ALT) fprintf(file, "\"Alt\" ");
                    if (kb.altMod & KMOD_MODE) fprintf(file, "\"AltGr\" ");
                    if (kb.altMod & KMOD_SHIFT) fprintf(file, "\"Shift\" ");
                    if (kb.altMod & KMOD_GUI) fprintf(file, "\"Gui\" ");
                }
                if (kb.altCode) {
                    fprintf(file, "\"%s\"", SDL_GetKeyName(kb.altCode));
                }
                fprintf(file, "] ");
            }
            fprintf(file, "}\n");
        }
    }

    TEINAPI void DoPreferencesSettings ()
    {
        float vw = GetViewport().w;
        float vh = GetViewport().h;

        float xPad = gPreferencesInnerXPad;
        float yPad = gPreferencesInnerYPad;

        BeginPanel(0, 0, vw, vh, UiFlag::None, gUiColorMedium);
        BeginPanel(xPad, yPad, vw-(xPad*2), vh-(yPad*2), UiFlag::None);

        // We redefine as these will now be different.
        vw = GetViewport().w;
        vh = GetViewport().h;

        float sw = roundf(vw / 2) - (xPad/2);
        float sh = gPreferencesSectionHeight;

        float bw = roundf(sw / 2);
        float th = sh-(gPreferencesTextBoxInset*2);

        Vec2 cursor(0,0);

        SetPanelCursorDir(UiDir::Right);
        SetPanelCursor(&cursor);

        BeginSettingsArea("User Interface", cursor);

        DoSettingsLabel(sw, gSettingUITheme);
        UiFlag darkFlags = (IsUiLight()) ? UiFlag::Inactive : UiFlag::None;
        UiFlag lightFlags = (IsUiLight()) ? UiFlag::None : UiFlag::Inactive;
        if (DoTextButton(NULL, bw,sh, darkFlags, "Dark")) {
            gEditorSettings.uiTheme = "dark";
            LoadUiTheme();
        }
        if (DoTextButton(NULL, bw,sh, lightFlags, "Light")) {
            gEditorSettings.uiTheme = "light";
            LoadUiTheme();
        }
        NextSection(cursor);

        DoSettingsLabel(sw, gSettingFontFace);
        UiFlag sansFlags = (IsEditorFontOpenSans()) ? UiFlag::None : UiFlag::Inactive;
        UiFlag dylsexicFlags = (IsEditorFontOpenSans()) ? UiFlag::Inactive : UiFlag::None;
        SetUiFont(&gResourceFontRegularSans);
        if (DoTextButton(NULL, bw,sh, sansFlags, "OpenSans")) {
            gEditorSettings.fontFace = "OpenSans";
            UpdateEditorFont();
        }
        SetUiFont(&gResourceFontRegularDyslexic);
        if (DoTextButton(NULL, bw,sh, dylsexicFlags, "OpenDyslexic")) {
            gEditorSettings.fontFace = "OpenDyslexic";
            UpdateEditorFont();
        }
        NextSection(cursor);

        // Reset the font to being the default for the editor.
        SetUiFont(&GetEditorRegularFont());

        DoSettingsLabel(sw, gSettingTileGraphics);
        UiFlag tileGraphicsNewFlags = (gEditorSettings.tileGraphics == "new") ? UiFlag::None : UiFlag::Inactive;
        UiFlag tileGraphicsOldFlags = (gEditorSettings.tileGraphics == "new") ? UiFlag::Inactive : UiFlag::None;
        if (DoTextButton(NULL, bw,sh, tileGraphicsNewFlags, "New")) {
            if (gEditorSettings.tileGraphics != "new") {
                gEditorSettings.tileGraphics = "new";
                ReloadTileGraphics();
            }
        }
        if (DoTextButton(NULL, bw,sh, tileGraphicsOldFlags, "Old")) {
            if (gEditorSettings.tileGraphics != "old") {
                gEditorSettings.tileGraphics = "old";
                ReloadTileGraphics();
            }
        }
        NextSection(cursor);

        DoSettingsLabel(sw, gSettingCustomCursors);
        UiFlag cursorEnabledFlags = (CustomCursorsEnabled()) ? UiFlag::None : UiFlag::Inactive;
        UiFlag cursorDisabledFlags = (CustomCursorsEnabled()) ? UiFlag::Inactive : UiFlag::None;
        if (DoTextButton(NULL, bw,sh, cursorEnabledFlags, "Enabled")) {
            gEditorSettings.customCursors = true;
            LoadEditorCursors();
        }
        if (DoTextButton(NULL, bw,sh, cursorDisabledFlags, "Disabled")) {
            gEditorSettings.customCursors = false;
            LoadEditorCursors();
        }
        NextSection(cursor);

        UiFlag tooltipsEnabledFlags = (gEditorSettings.showTooltips) ? UiFlag::None : UiFlag::Inactive;
        UiFlag tooltipsDisabledFlags = (gEditorSettings.showTooltips) ? UiFlag::Inactive : UiFlag::None;
        DoSettingsLabel(sw, gSettingShowTooltips);
        if (DoTextButton(NULL, bw,sh, tooltipsEnabledFlags, "Enabled")) {
            gEditorSettings.showTooltips = true;
        }
        if (DoTextButton(NULL, bw,sh, tooltipsDisabledFlags, "Disabled")) {
            gEditorSettings.showTooltips = false;
        }
        NextSection(cursor);

        EndSettingsArea();

        BeginSettingsArea("Level Backups", cursor);

        DoSettingsLabel(sw, gSettingAutoBackup);
        UiFlag backupEnabledFlags = (gEditorSettings.autoBackup) ? UiFlag::None : UiFlag::Inactive;
        UiFlag backupDisabledFlags = (gEditorSettings.autoBackup) ? UiFlag::Inactive : UiFlag::None;
        if (DoTextButton(NULL, bw,sh, backupEnabledFlags, "Enabled")) {
            gEditorSettings.autoBackup = true;
            UpdateBackupTimer();
        }
        if (DoTextButton(NULL, bw,sh, backupDisabledFlags, "Disabled")) {
            gEditorSettings.autoBackup = false;
            UpdateBackupTimer();
        }
        NextSection(cursor);

        DoSettingsLabel(sw, gSettingUnlimitedBackups);
        UiFlag unlimitedEnabledFlags = (gEditorSettings.unlimitedBackups) ? UiFlag::None : UiFlag::Inactive;
        UiFlag unlimitedDisabledFlags = (gEditorSettings.unlimitedBackups) ? UiFlag::Inactive : UiFlag::None;
        if (DoTextButton(NULL, bw,sh, unlimitedEnabledFlags, "Enabled")) {
            gEditorSettings.unlimitedBackups = true;
        }
        if (DoTextButton(NULL, bw,sh, unlimitedDisabledFlags, "Disabled")) {
            gEditorSettings.unlimitedBackups = false;
        }
        NextSection(cursor);

        DoSettingsLabel(sw, gSettingBackupInterval);
        if (!gEditorSettings.autoBackup) SetPanelFlags(UiFlag::Locked);
        cursor.y += gPreferencesTextBoxInset;
        std::string backupIntervalString(std::to_string(gEditorSettings.backupInterval));
        DoTextBox(vw-cursor.x,th, UiFlag::Numeric, backupIntervalString, "0");
        cursor.y -= gPreferencesTextBoxInset;
        if (!gEditorSettings.autoBackup) SetPanelFlags(UiFlag::None);
        if (atoll(backupIntervalString.c_str()) > INT_MAX) {
            backupIntervalString = std::to_string(INT_MAX);
        }
        int backupInterval = atoi(backupIntervalString.c_str());
        if (backupInterval != gEditorSettings.backupInterval) {
            gEditorSettings.backupInterval = backupInterval;
            UpdateBackupTimer();
        }
        NextSection(cursor);

        DoSettingsLabel(sw, gSettingBackupCount);
        if (gEditorSettings.unlimitedBackups) SetPanelFlags(UiFlag::Locked);
        cursor.y += gPreferencesTextBoxInset;
        std::string backupCountString(std::to_string(gEditorSettings.backupCount));
        DoTextBox(vw-cursor.x,th, UiFlag::Numeric, backupCountString, "0");
        cursor.y -= gPreferencesTextBoxInset;
        if (gEditorSettings.unlimitedBackups) SetPanelFlags(UiFlag::None);
        if (atoll(backupCountString.c_str()) > INT_MAX) {
            backupCountString = std::to_string(INT_MAX);
        }
        int backupCount = atoi(backupCountString.c_str());
        if (backupCount != gEditorSettings.backupCount) {
            gEditorSettings.backupCount = backupCount;
        }
        NextSection(cursor);

        EndSettingsArea();

        BeginSettingsArea("Custom Colors", cursor);

        float hw = roundf(vw/4) - (xPad/2);

        DoColorSettingRow(cursor, hw, gSettingBackgroundColor, gSettingTileGridColor, gEditorSettings.backgroundColor, gEditorSettings.tileGridColor);
        DoColorSettingRow(cursor, hw, gSettingSelectColor, gSettingOutOfBoundsColor, gEditorSettings.selectColor, gEditorSettings.outOfBoundsColor);
        DoColorSettingRow(cursor, hw, gSettingCursorColor, gSettingMirrorLineColor , gEditorSettings.cursorColor, gEditorSettings.mirrorLineColor);

        EndSettingsArea();

        EndPanel();
        EndPanel();
    }

    TEINAPI void DoPreferencesHotkeys ()
    {
        float vw = GetViewport().w;
        float vh = GetViewport().h;

        BeginPanel(0,0,vw,vh, UiFlag::None);

        Vec2 cursor(0,0);

        SetPanelCursorDir(UiDir::Right);
        SetPanelCursor(&cursor);

        float contentHeight = (gPreferencesHotkeysNames.size() * (gPreferencesSectionHeight+1)) + (gPreferencesSectionHeight+(gPreferencesInnerYPad*2));
        DoScrollbar(vw-gPreferencesScrollbarWidth+1, -1, gPreferencesScrollbarWidth, vh+2, contentHeight, gPreferencesScrollOffset);

        DoHotkeyLabelTitles(cursor);

        DoHotkeyRebind(cursor, gKbLevelNew);
        DoHotkeyRebind(cursor, gKbLevelOpen);
        DoHotkeyRebind(cursor, gKbLevelSave);
        DoHotkeyRebind(cursor, gKbLevelSaveAs);
        DoHotkeyRebind(cursor, gKbLevelClose);
        DoHotkeyRebind(cursor, gKbLevelCloseAll);
        DoHotkeyRebind(cursor, gKbLevelResize);
        DoHotkeyRebind(cursor, gKbUndo);
        DoHotkeyRebind(cursor, gKbRedo);
        DoHotkeyRebind(cursor, gKbHistoryBegin);
        DoHotkeyRebind(cursor, gKbHistoryEnd);
        DoHotkeyRebind(cursor, gKbCameraZoomOut);
        DoHotkeyRebind(cursor, gKbCameraZoomIn);
        DoHotkeyRebind(cursor, gKbRunGame);
        DoHotkeyRebind(cursor, gKbPreferences);
        DoHotkeyRebind(cursor, gKbAbout);
        DoHotkeyRebind(cursor, gKbBugReport);
        DoHotkeyRebind(cursor, gKbHelp);
        DoHotkeyRebind(cursor, gKbToolBrush);
        DoHotkeyRebind(cursor, gKbToolFill);
        DoHotkeyRebind(cursor, gKbToolSelect);
        DoHotkeyRebind(cursor, gKbFlipH);
        DoHotkeyRebind(cursor, gKbFlipV);
        DoHotkeyRebind(cursor, gKbMirrorHToggle);
        DoHotkeyRebind(cursor, gKbMirrorVToggle);
        DoHotkeyRebind(cursor, gKbGpakUnpack);
        DoHotkeyRebind(cursor, gKbGpakPack);
        DoHotkeyRebind(cursor, gKbToggleEntity);
        DoHotkeyRebind(cursor, gKbToggleGuides);
        DoHotkeyRebind(cursor, gKbGridToggle);
        DoHotkeyRebind(cursor, gKbBoundsToggle);
        DoHotkeyRebind(cursor, gKbLayersToggle);
        DoHotkeyRebind(cursor, gKbCameraReset);
        DoHotkeyRebind(cursor, gKbClearSelect);
        DoHotkeyRebind(cursor, gKbDeselect);
        DoHotkeyRebind(cursor, gKbSelectAll);
        DoHotkeyRebind(cursor, gKbCopy);
        DoHotkeyRebind(cursor, gKbCut);
        DoHotkeyRebind(cursor, gKbPaste);
        DoHotkeyRebind(cursor, gKbIncrementTile);
        DoHotkeyRebind(cursor, gKbDecrementTile);
        DoHotkeyRebind(cursor, gKbIncrementGroup);
        DoHotkeyRebind(cursor, gKbDecrementGroup);
        DoHotkeyRebind(cursor, gKbIncrementCategory);
        DoHotkeyRebind(cursor, gKbDecrementCategory);
        DoHotkeyRebind(cursor, gKbToggleLayerActive);
        DoHotkeyRebind(cursor, gKbToggleLayerTag);
        DoHotkeyRebind(cursor, gKbToggleLayerOverlay);
        DoHotkeyRebind(cursor, gKbToggleLayerBack1);
        DoHotkeyRebind(cursor, gKbToggleLayerBack2);
        DoHotkeyRebind(cursor, gKbCategoryBasic);
        DoHotkeyRebind(cursor, gKbCategoryTag);
        DoHotkeyRebind(cursor, gKbCategoryOverlay);
        DoHotkeyRebind(cursor, gKbCategoryEntity);
        DoHotkeyRebind(cursor, gKbCategoryBack1);
        DoHotkeyRebind(cursor, gKbCategoryBack2);
        DoHotkeyRebind(cursor, gKbIncrementTab);
        DoHotkeyRebind(cursor, gKbDecrementTab);
        DoHotkeyRebind(cursor, gKbMoveTabLeft);
        DoHotkeyRebind(cursor, gKbMoveTabRight);
        DoHotkeyRebind(cursor, gKbOpenRecentTab);
        DoHotkeyRebind(cursor, gKbLoadPrevLevel);
        DoHotkeyRebind(cursor, gKbLoadNextLevel);

        EndPanel();
    }
}

TEINAPI void InitPreferencesMenu ()
{
    gCachedEditorSettings = gEditorSettings;
    gCachedEditorHotkeys = gKeyBindings;
    gColorPickerMousePressed = false;
}

TEINAPI void DoPreferencesMenu ()
{
    Quad p1, p2;

    p1.x = gWindowBorder;
    p1.y = gWindowBorder;
    p1.w = GetViewport().w - (gWindowBorder * 2);
    p1.h = GetViewport().h - (gWindowBorder * 2);

    SetUiFont(&GetEditorRegularFont());

    BeginPanel(p1, UiFlag::None, gUiColorExDark);

    Vec2 cursor;

    float pvfh = gPreferencesVerticalFrameHeight;

    float vw = GetViewport().w;
    float vh = GetViewport().h;

    float tw = roundf(vw / 2);
    float th = pvfh - gWindowBorder;

    float bw = roundf(vw / 3);
    float bh = pvfh - gWindowBorder;

    // Top tabs for switching from the settings and key bindings menu.
    cursor = Vec2(0,0);
    BeginPanel(0, 0, vw, pvfh, UiFlag::None, gUiColorMedium);

    SetPanelCursorDir(UiDir::Right);
    SetPanelCursor(&cursor);

    UiFlag settingsFlags = (gPreferencesTab == PreferencesTab::Settings) ? UiFlag::Highlight : UiFlag::Inactive;
    UiFlag hotkeysFlags = (gPreferencesTab == PreferencesTab::Hotkeys) ? UiFlag::Highlight : UiFlag::Inactive;

    if (DoTextButton(NULL, tw,th, settingsFlags, "Settings")) gPreferencesTab = PreferencesTab::Settings;
    if (DoTextButton(NULL, tw,th, hotkeysFlags, "Hotkeys")) gPreferencesTab = PreferencesTab::Hotkeys;

    // Just in case of weird rounding manually add the right separator.
    cursor.x = vw;
    DoSeparator(bh);

    // Add a separator to the left for symmetry.
    cursor.x = 1;
    DoSeparator(bh);

    EndPanel();

    // Bottom buttons for saving and exiting or cancelling changes.
    cursor = Vec2(0, gWindowBorder);
    BeginPanel(0, vh-pvfh, vw, pvfh, UiFlag::None, gUiColorMedium);

    SetPanelCursorDir(UiDir::Right);
    SetPanelCursor(&cursor);

    std::string restoreMessage;
    switch (gPreferencesTab) {
        case (PreferencesTab::Settings): restoreMessage = "Restore Settings"; break;
        case (PreferencesTab::Hotkeys): restoreMessage = "Restore Hotkeys";  break;
    }

    // Just to make sure that we always reach the end of the panel space.
    float bw2 = vw - (bw*2);

    if (DoTextButton(NULL, bw ,bh, UiFlag::None, "Save and Exit"  )) SavePreferences();
    if (DoTextButton(NULL, bw ,bh, UiFlag::None, restoreMessage   )) RestorePreferences();
    if (DoTextButton(NULL, bw2,bh, UiFlag::None, "Cancel and Exit")) CancelPreferences();

    // Add a separator to the left for symmetry.
    cursor.x = 1;
    DoSeparator(bh);

    EndPanel();

    p2.x = 1;
    p2.y = pvfh + 1;
    p2.w = GetViewport().w - 2;
    p2.h = GetViewport().h - p2.y - pvfh - 1;

    BeginPanel(p2, UiFlag::None);
    switch (gPreferencesTab) {
        case (PreferencesTab::Settings): Internal::DoPreferencesSettings(); break;
        case (PreferencesTab::Hotkeys): Internal::DoPreferencesHotkeys(); break;
    }
    EndPanel();

    EndPanel();
}

TEINAPI void HandlePreferencesMenuEvents ()
{
    // Check if there was a mouse press for the color swatch settings.
    gPreferencesMousePressed = false;

    if (!IsWindowFocused("WINPREFERENCES")) return;

    switch (gMainEvent.type) {
        case (SDL_MOUSEBUTTONDOWN): {
            if (gMainEvent.button.button == SDL_BUTTON_LEFT) {
                gPreferencesMousePressed = true;
            }
        } break;
        case (SDL_KEYDOWN): {
            if (!TextBoxIsActive() && !HotkeyIsActive()) {
                switch (gMainEvent.key.keysym.sym) {
                    case (SDLK_RETURN): SavePreferences(); break;
                    case (SDLK_ESCAPE): CancelPreferences(); break;
                }
            }
        } break;
    }
}

TEINAPI void RestorePreferences ()
{
    switch (gPreferencesTab) {
        case (PreferencesTab::Settings): RestoreEditorSettings(); break;
        case (PreferencesTab::Hotkeys): RestoreEditorKeyBindings(); break;
    }
}

TEINAPI void CancelPreferences ()
{
    // We only want to shot this prompt if the user actually made any changes to preferences.
    if (gEditorSettings != gCachedEditorSettings || gKeyBindings != gCachedEditorHotkeys) {
        // Make sure the user is certain about what they are doing and cancel the
        // action if they decide that they do not actually want to discard changes.
        if (ShowAlert("Discard Changes", "Are you sure you want do discard changes?", AlertType::Warning, AlertButton::YesNo, "WINPREFERENCES") == AlertResult::No) {
            return;
        }
    }

    bool tileGraphicsChanged = (gEditorSettings.tileGraphics != gCachedEditorSettings.tileGraphics);

    // Restore the settings/hotkeys states from before modifying preferences.
    gEditorSettings = gCachedEditorSettings;
    gKeyBindings = gCachedEditorHotkeys;

    UpdateSystemsThatRelyOnSettings(tileGraphicsChanged);

    HideWindow("WINCOLOR");
    HideWindow("WINPREFERENCES");
}

TEINAPI void SavePreferences ()
{
    // No point saving unless there were changes.
    if (gEditorSettings != gCachedEditorSettings || gKeyBindings != gCachedEditorHotkeys) Internal::SaveSettings();

    HideWindow("WINCOLOR");
    HideWindow("WINPREFERENCES");
}
