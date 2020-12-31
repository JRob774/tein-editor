static constexpr const char* gHotbarNameNew = "New";
static constexpr const char* gHotbarNameLoad = "Load";
static constexpr const char* gHotbarNameSave = "Save";
static constexpr const char* gHotbarNameSaveAs = "Save As";
static constexpr const char* gHotbarNameUndo = "Undo";
static constexpr const char* gHotbarNameRedo = "Redo";
static constexpr const char* gHotbarNameZoomOut = "Zoom Out";
static constexpr const char* gHotbarNameZoomIn = "Zoom In";
static constexpr const char* gHotbarNamePack = "Pack GPAK";
static constexpr const char* gHotbarNameUnpack = "Unpack GPAK";
static constexpr const char* gHotbarNameRunGame = "Play";
static constexpr const char* gHotbarNamePreferences = "Preferences";
static constexpr const char* gHotbarNameAbout = "About";
static constexpr const char* gHotbarNameHelp = "Help";
static constexpr const char* gHotbarNameBugReport = "Bug Report";
static constexpr const char* gHotbarNameUpdate = "Update";

static constexpr const char* gHotbarInfoNew = "Create a new empty level.";
static constexpr const char* gHotbarInfoLoad = "Load an existing level.";
static constexpr const char* gHotbarInfoSave = "Save the current level.";
static constexpr const char* gHotbarInfoSaveAs = "Save the current level as a new file.";
static constexpr const char* gHotbarInfoUndo = "Undo the last recorded action.";
static constexpr const char* gHotbarInfoRedo = "Redo the last recorded action.";
static constexpr const char* gHotbarInfoZoomOut = "Zoom out the editor camera.";
static constexpr const char* gHotbarInfoZoomIn = "Zoom in the editor camera.";
static constexpr const char* gHotbarInfoPack = "Pack data files into the GPAK file format.";
static constexpr const char* gHotbarInfoUnpack = "Unpack data files from the GPAK file format.";
static constexpr const char* gHotbarInfoRunGame = "Runs The End is Nigh game application.";
static constexpr const char* gHotbarInfoPreferences = "Open the preferences menu to customize the editor.";
static constexpr const char* gHotbarInfoAbout = "Open the about menu for application information.";
static constexpr const char* gHotbarInfoBugReport  = "Report technical issues or bugs with the editor.";
static constexpr const char* gHotbarInfoHelp = "Information and help about modding The End is Nigh.";
static constexpr const char* gHotbarInfoUpdate = "There is a new update available for the editor.";

TEINAPI void DoHotbar ()
{
    Vec2 cursor(0,0);

    // Height for the toolbar buttons.
    float bh = gHotbarHeight - gWindowBorder;

    SetUiTexture(&gResourceIcons);
    SetUiFont(&GetEditorRegularFont());

    UiFlag saveFlags = UI_NONE;
    UiFlag saveAsFlags = UI_NONE;
    UiFlag undoFlags = UI_NONE;
    UiFlag redoFlags = UI_NONE;
    UiFlag zoomOutFlags = UI_NONE;
    UiFlag zoomInFlags = UI_NONE;
    UiFlag packFlags = UI_NONE;
    UiFlag unpackFlags = UI_NONE;

    packFlags = ((IsGPAKPackComplete()) ? UI_NONE : UI_LOCKED);
    unpackFlags = ((IsGPAKUnpackComplete()) ? UI_NONE : UI_LOCKED);

    if (are_there_any_tabs())
    {
        const Tab& tab = get_current_tab();

        // @Improve: Duplicate across level and map, pull-out and generalise for both tab types!
        if (tab.type == Tab_Type::LEVEL)
        {
            if (tab.level_history.currentPosition == tab.level_history.state.size()-1)
            {
                redoFlags = UI_LOCKED;
            }
            if (tab.level_history.currentPosition <= -1)
            {
                undoFlags = UI_LOCKED;
            }
            if (tab.level_history.state.empty())
            {
                undoFlags = UI_LOCKED;
                redoFlags = UI_LOCKED;
            }
        }
        else
        {
            if (tab.map_history.currentPosition == tab.map_history.state.size()-1)
            {
                redoFlags = UI_LOCKED;
            }
            if (tab.map_history.currentPosition <= 0)
            {
                undoFlags = UI_LOCKED;
            }
            if (tab.map_history.state.size() == 1)
            {
                undoFlags = UI_LOCKED;
                redoFlags = UI_LOCKED;
            }
        }

        // @Improve: Duplicate across level and map, pull-out and generalise for both tab types!
        if (tab.type == Tab_Type::LEVEL)
        {
            zoomOutFlags = ((tab.camera.zoom == MIN_LVL_EDITOR_ZOOM) ? UI_LOCKED : UI_NONE);
            zoomInFlags = ((tab.camera.zoom == MAX_LVL_EDITOR_ZOOM) ? UI_LOCKED : UI_NONE);
        }
        else
        {
            zoomOutFlags = (tab.camera.zoom == MIN_MAP_EDITOR_ZOOM) ? UI_LOCKED : UI_NONE;
            zoomInFlags = (tab.camera.zoom == MAX_MAP_EDITOR_ZOOM) ? UI_LOCKED : UI_NONE;
        }
    }
    else
    {
        saveFlags = UI_LOCKED;
        saveAsFlags = UI_LOCKED;
        undoFlags = UI_LOCKED;
        redoFlags = UI_LOCKED;
        zoomOutFlags = UI_LOCKED;
        zoomInFlags = UI_LOCKED;
    }

    // The hotbar is a horizontal list of available actions.
    BeginPanel(0, 0, GetViewport().w, gHotbarHeight, UI_NONE, gUiColorMedium);

    SetPanelCursor(&cursor);
    SetPanelCursorDir(UI_DIR_RIGHT);

    // We want these to be the same width as the toolbar (looks nice).
    float bw = gToolbarDefaultWidth+1;

    float width = 0;

    width += CalculateTextButtonWidth(gHotbarNameNew);
    width += CalculateTextButtonWidth(gHotbarNameLoad);
    width += CalculateTextButtonWidth(gHotbarNameSave);
    width += CalculateTextButtonWidth(gHotbarNameSaveAs);
    width += CalculateTextButtonWidth(gHotbarNameUndo);
    width += CalculateTextButtonWidth(gHotbarNameRedo);
    width += CalculateTextButtonWidth(gHotbarNameZoomOut);
    width += CalculateTextButtonWidth(gHotbarNameZoomIn);
    width += CalculateTextButtonWidth(gHotbarNamePack);
    width += CalculateTextButtonWidth(gHotbarNameUnpack);
    width += CalculateTextButtonWidth(gHotbarNameRunGame);
    width += CalculateTextButtonWidth(gHotbarNamePreferences);
    width += CalculateTextButtonWidth(gHotbarNameAbout);
    width += CalculateTextButtonWidth(gHotbarNameBugReport);
    width += CalculateTextButtonWidth(gHotbarNameHelp);
    width += (are_there_updates()) ? bw : 0;

    // Display text or icons depending on what we have room for.
    if (width < GetViewport().w)
    {
        DoTextButton(HotbarNew, bh, UI_NONE, gHotbarNameNew, gHotbarInfoNew, gKbLevelNew);
        DoTextButton(HotbarLoad, bh, UI_NONE, gHotbarNameLoad, gHotbarInfoLoad, gKbLevelOpen);
        DoTextButton(HotbarSave, bh, saveFlags, gHotbarNameSave, gHotbarInfoSave, gKbLevelSave);
        DoTextButton(HotbarSaveAs, bh, saveAsFlags, gHotbarNameSaveAs, gHotbarInfoSaveAs, gKbLevelSaveAs);
        DoTextButton(HotbarUndo, bh, undoFlags, gHotbarNameUndo, gHotbarInfoUndo, gKbUndo);
        DoTextButton(HotbarRedo, bh, redoFlags, gHotbarNameRedo, gHotbarInfoRedo, gKbRedo);
        DoTextButton(HotbarZoomOut, bh, zoomOutFlags, gHotbarNameZoomOut, gHotbarInfoZoomOut, gKbCameraZoomOut);
        DoTextButton(HotbarZoomIn, bh, zoomInFlags, gHotbarNameZoomIn, gHotbarInfoZoomIn, gKbCameraZoomIn);
        DoTextButton(HotbarGPAKPack, bh, packFlags, gHotbarNamePack, gHotbarInfoPack, gKbGpakPack);
        DoTextButton(HotbarGPAKUnpack, bh, unpackFlags, gHotbarNameUnpack, gHotbarInfoUnpack, gKbGpakUnpack);
        DoTextButton(HotbarRunGame, bh, UI_NONE, gHotbarNameRunGame, gHotbarInfoRunGame, gKbRunGame);
        DoTextButton(HotbarPreferences, bh, UI_NONE, gHotbarNamePreferences, gHotbarInfoPreferences, gKbPreferences);
        DoTextButton(HotbarAbout, bh, UI_NONE, gHotbarNameAbout, gHotbarInfoAbout, gKbAbout);
        DoTextButton(HotbarBugReport, bh, UI_NONE, gHotbarNameBugReport, gHotbarInfoBugReport, gKbBugReport);
        DoTextButton(HotbarHelp, bh, UI_NONE, gHotbarNameHelp, gHotbarInfoHelp, gKbHelp);
    }
    else
    {
        DoImageButton(HotbarNew, bw,bh, UI_NONE, &gClipNew, gHotbarInfoNew, gKbLevelNew, gHotbarNameNew);
        DoImageButton(HotbarLoad, bw,bh, UI_NONE, &gClipLoad, gHotbarInfoLoad, gKbLevelOpen, gHotbarNameLoad);
        DoImageButton(HotbarSave, bw,bh, saveFlags, &gClipSave, gHotbarInfoSave, gKbLevelSave, gHotbarNameSave);
        DoImageButton(HotbarSaveAs, bw,bh, saveAsFlags, &gClipSaveAs, gHotbarInfoSaveAs, gKbLevelSaveAs, gHotbarNameSaveAs);
        DoImageButton(HotbarUndo, bw,bh, undoFlags, &gClipUndo, gHotbarInfoUndo, gKbUndo, gHotbarNameUndo);
        DoImageButton(HotbarRedo, bw,bh, redoFlags, &gClipRedo, gHotbarInfoRedo, gKbRedo, gHotbarNameRedo);
        DoImageButton(HotbarZoomOut, bw,bh, zoomOutFlags, &gClipZoomOut, gHotbarInfoZoomOut, gKbCameraZoomOut, gHotbarNameZoomOut);
        DoImageButton(HotbarZoomIn, bw,bh, zoomInFlags, &gClipZoomIn, gHotbarInfoZoomIn, gKbCameraZoomIn, gHotbarNameZoomIn);
        DoImageButton(HotbarGPAKPack, bw,bh, packFlags, &gClipPack, gHotbarInfoPack, gKbGpakPack, gHotbarNamePack);
        DoImageButton(HotbarGPAKUnpack, bw,bh, unpackFlags, &gClipUnpack, gHotbarInfoUnpack, gKbGpakUnpack, gHotbarNameUnpack);
        DoImageButton(HotbarRunGame, bw,bh, UI_NONE, &gClipRun, gHotbarInfoRunGame, gKbRunGame, gHotbarNameRunGame);
        DoImageButton(HotbarPreferences, bw,bh, UI_NONE, &gClipSettings, gHotbarInfoPreferences, gKbPreferences, gHotbarNamePreferences);
        DoImageButton(HotbarAbout, bw,bh, UI_NONE, &gClipAbout, gHotbarInfoAbout, gKbAbout, gHotbarNameAbout);
        DoImageButton(HotbarBugReport, bw,bh, UI_NONE, &gClipBug, gHotbarInfoBugReport, gKbBugReport, gHotbarNameBugReport);
        DoImageButton(HotbarHelp, bw,bh, UI_NONE, &gClipHelp, gHotbarInfoHelp, gKbHelp, gHotbarNameHelp);
    }

    // Right-aligned update button.
    if (are_there_updates())
    {
        cursor.x = GetViewport().w - (bw-1);
        DoImageButton(HotbarUpdate, bw,bh, UI_NONE, &gClipUpdate, gHotbarInfoUpdate, "", gHotbarNameUpdate);
    }

    EndPanel();
}

TEINAPI void HotbarNew ()
{
    OpenNew();
}

TEINAPI void HotbarLoad ()
{
    std::vector<std::string> fileNames = OpenDialog(DialogType::LVL_CSV);
    if (!fileNames.empty())
    {
        for (auto file: fileNames)
        {
            std::string ext(file.substr(file.find_last_of(".")));
            if (ext == ".lvl") LoadLevelTab(file);
            else if (ext == ".csv") LoadMapTab(file);
        }
    }
}

TEINAPI void HotbarSave ()
{
    if (!are_there_any_tabs()) return;
    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): LevelEditorSave(get_current_tab()); break;
        case (Tab_Type::MAP): SaveMapTab(get_current_tab()); break;
    }
}

TEINAPI void HotbarSaveAs ()
{
    if (!are_there_any_tabs()) return;
    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): LevelEditorSaveAs(); break;
        case (Tab_Type::MAP): SaveMapTabAs(); break;
    }
}

TEINAPI void HotbarUndo ()
{
    if (!are_there_any_tabs()) return;
    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): LevelEditorUndo(); break;
        case (Tab_Type::MAP): MapEditorUndo(); break;
    }
}

TEINAPI void HotbarRedo ()
{
    if (!are_there_any_tabs()) return;
    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): LevelEditorRedo(); break;
        case (Tab_Type::MAP): MapEditorRedo(); break;
    }
}

TEINAPI void HotbarHistoryBegin ()
{
    if (!are_there_any_tabs()) return;
    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): LevelEditorHistoryBegin(); break;
        case (Tab_Type::MAP): MapEditorHistoryBegin(); break;
    }
}

TEINAPI void HotbarHistoryEnd ()
{
    if (!are_there_any_tabs()) return;
    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): LevelEditorHistoryEnd(); break;
        case (Tab_Type::MAP): MapEditorHistoryEnd(); break;
    }
}

TEINAPI void HotbarZoomOut ()
{
    if (!are_there_any_tabs()) return;
    Tab& tab = get_current_tab();
    if (tab.type == Tab_Type::LEVEL)
    {
        if ((tab.camera.zoom /= 2) < MIN_LVL_EDITOR_ZOOM)
        {
            tab.camera.zoom = MIN_LVL_EDITOR_ZOOM;
        }
    }
    else
    {
        if ((tab.camera.zoom /= 2) < MIN_MAP_EDITOR_ZOOM)
        {
            tab.camera.zoom = MIN_MAP_EDITOR_ZOOM;
        }
    }
}

TEINAPI void HotbarZoomIn ()
{
    if (!are_there_any_tabs()) return;
    Tab& tab = get_current_tab();
    if (tab.type == Tab_Type::LEVEL)
    {
        if ((tab.camera.zoom *= 2) > MAX_LVL_EDITOR_ZOOM)
        {
            tab.camera.zoom = MAX_LVL_EDITOR_ZOOM;
        }
    }
    else
    {
        if ((tab.camera.zoom *= 2) > MAX_MAP_EDITOR_ZOOM)
        {
            tab.camera.zoom = MAX_MAP_EDITOR_ZOOM;
        }
    }
}

TEINAPI void HotbarGPAKUnpack ()
{
    std::vector<std::string> files = OpenDialog(DialogType::GPAK, false);
    if (!files.empty())
    {
        bool shouldOverwrite = true;
        if (ShowAlert("Overwrite", "Do you want to overwrite any existing files during unpack?", ALERT_TYPE_INFO, ALERT_BUTTON_YES_NO) == ALERT_RESULT_NO)
        {
            shouldOverwrite = false;
        }

        GPAKUnpack(files.at(0), shouldOverwrite);
    }
}

TEINAPI void HotbarGPAKPack ()
{
    std::vector<std::string> paths = PathDialog();
    if (!paths.empty())
    {
        std::string file = SaveDialog(DialogType::GPAK);
        if (!file.empty())
        {
            GPAKPack(file, paths);
        }
    }
}

TEINAPI void HotbarRunGame ()
{
    constexpr const char* ExeSteamX86 = "C:/Program Files (x86)/Steam/steamapps/common/theendisnigh/TheEndIsNigh.exe";
    constexpr const char* ExeSteamX64 = "C:/Program Files/Steam/steamapps/common/theendisnigh/TheEndIsNigh.exe";
    constexpr const char* ExeSteamApp = "TheEndIsNigh.exe";
    constexpr const char* ExeEpicX86  = "C:/Program Files (x86)/Epic Games/theendisnigh/TheEnd.exe";
    constexpr const char* ExeEpicX64  = "C:/Program Files)/Epic Games/theendisnigh/TheEnd.exe";
    constexpr const char* ExeEpicApp  = "TheEnd.exe";

    const std::vector<std::string> Executables
    {
        ExeSteamX86, ExeSteamX64, ExeSteamApp,
        ExeEpicX86, ExeEpicX64, ExeEpicApp
    };

    std::string executable;
    if (!gEditorSettings.gamePath.empty())
    {
        executable = gEditorSettings.gamePath;
        if (!DoesFileExist(executable))
        {
            executable.clear();
        }
    }
    if (executable.empty())
    {
        for (auto exe: Executables)
        {
            executable = exe;
            if (DoesFileExist(executable))
            {
                break;
            }
        }
        if (!DoesFileExist(executable))
        {
            executable.clear();
        }
    }

    // Executable couldn't be found so we will ask for the location.
    if (executable.empty())
    {
        OpenPath();
    }
    else if (!RunExecutable(executable))
    {
        LogError(ERR_MED, "Failed to launch The End is Nigh executable!");
    }
}

TEINAPI void HotbarPreferences ()
{
    if (IsWindowHidden("WINPREFERENCES"))
    {
        init_preferences_menu(); // Load current settings.
        ShowWindow("WINPREFERENCES");
    }
    else
    {
        RaiseWindow("WINPREFERENCES");
    }
}

TEINAPI void HotbarAbout ()
{
    if (IsWindowHidden("WINABOUT")) ShowWindow("WINABOUT"); else RaiseWindow("WINABOUT");
}

TEINAPI void HotbarHelp ()
{
    LoadWebpage("http://glaielgames.com/teinworkshop/moddermanual/");
}

TEINAPI void HotbarBugReport ()
{
    LoadWebpage("https://itch.io/t/822898/bug-reports");
}

TEINAPI void HotbarUpdate ()
{
    open_update_window();
}
