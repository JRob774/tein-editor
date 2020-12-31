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

    if (AreThereAnyTabs())
    {
        const Tab& tab = GetCurrentTab();

        // @Improve: Duplicate across level and map, pull-out and generalise for both tab types!
        if (tab.type == TabType::LEVEL)
        {
            if (tab.levelHistory.currentPosition == tab.levelHistory.state.size()-1)
            {
                redoFlags = UI_LOCKED;
            }
            if (tab.levelHistory.currentPosition <= -1)
            {
                undoFlags = UI_LOCKED;
            }
            if (tab.levelHistory.state.empty())
            {
                undoFlags = UI_LOCKED;
                redoFlags = UI_LOCKED;
            }
        }
        else
        {
            if (tab.mapHistory.currentPosition == tab.mapHistory.state.size()-1)
            {
                redoFlags = UI_LOCKED;
            }
            if (tab.mapHistory.currentPosition <= 0)
            {
                undoFlags = UI_LOCKED;
            }
            if (tab.mapHistory.state.size() == 1)
            {
                undoFlags = UI_LOCKED;
                redoFlags = UI_LOCKED;
            }
        }

        // @Improve: Duplicate across level and map, pull-out and generalise for both tab types!
        if (tab.type == TabType::LEVEL)
        {
            zoomOutFlags = ((tab.camera.zoom == gMinLevelEditorZoom) ? UI_LOCKED : UI_NONE);
            zoomInFlags = ((tab.camera.zoom == gMaxLevelEditorZoom) ? UI_LOCKED : UI_NONE);
        }
        else
        {
            zoomOutFlags = (tab.camera.zoom == gMinMapEditorZoom) ? UI_LOCKED : UI_NONE;
            zoomInFlags = (tab.camera.zoom == gMaxMapEditorZoom) ? UI_LOCKED : UI_NONE;
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
    if (!AreThereAnyTabs()) return;
    switch (GetCurrentTab().type)
    {
        case (TabType::LEVEL): LevelEditorSave(GetCurrentTab()); break;
        case (TabType::MAP): SaveMapTab(GetCurrentTab()); break;
    }
}

TEINAPI void HotbarSaveAs ()
{
    if (!AreThereAnyTabs()) return;
    switch (GetCurrentTab().type)
    {
        case (TabType::LEVEL): LevelEditorSaveAs(); break;
        case (TabType::MAP): SaveMapTabAs(); break;
    }
}

TEINAPI void HotbarUndo ()
{
    if (!AreThereAnyTabs()) return;
    switch (GetCurrentTab().type)
    {
        case (TabType::LEVEL): LevelEditorUndo(); break;
        case (TabType::MAP): MapEditorUndo(); break;
    }
}

TEINAPI void HotbarRedo ()
{
    if (!AreThereAnyTabs()) return;
    switch (GetCurrentTab().type)
    {
        case (TabType::LEVEL): LevelEditorRedo(); break;
        case (TabType::MAP): MapEditorRedo(); break;
    }
}

TEINAPI void HotbarHistoryBegin ()
{
    if (!AreThereAnyTabs()) return;
    switch (GetCurrentTab().type)
    {
        case (TabType::LEVEL): LevelEditorHistoryBegin(); break;
        case (TabType::MAP): MapEditorHistoryBegin(); break;
    }
}

TEINAPI void HotbarHistoryEnd ()
{
    if (!AreThereAnyTabs()) return;
    switch (GetCurrentTab().type)
    {
        case (TabType::LEVEL): LevelEditorHistoryEnd(); break;
        case (TabType::MAP): MapEditorHistoryEnd(); break;
    }
}

TEINAPI void HotbarZoomOut ()
{
    if (!AreThereAnyTabs()) return;
    Tab& tab = GetCurrentTab();
    if (tab.type == TabType::LEVEL)
    {
        if ((tab.camera.zoom /= 2) < gMinLevelEditorZoom)
        {
            tab.camera.zoom = gMinLevelEditorZoom;
        }
    }
    else
    {
        if ((tab.camera.zoom /= 2) < gMinMapEditorZoom)
        {
            tab.camera.zoom = gMinMapEditorZoom;
        }
    }
}

TEINAPI void HotbarZoomIn ()
{
    if (!AreThereAnyTabs()) return;
    Tab& tab = GetCurrentTab();
    if (tab.type == TabType::LEVEL)
    {
        if ((tab.camera.zoom *= 2) > gMaxLevelEditorZoom)
        {
            tab.camera.zoom = gMaxLevelEditorZoom;
        }
    }
    else
    {
        if ((tab.camera.zoom *= 2) > gMaxMapEditorZoom)
        {
            tab.camera.zoom = gMaxMapEditorZoom;
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
        InitPreferencesMenu(); // Load current settings.
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
