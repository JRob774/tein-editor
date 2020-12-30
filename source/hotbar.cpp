/*******************************************************************************
 * Editor GUI widget containing global editor actions/buttons.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr const char* HB_NAME_NEW         = "New";
static constexpr const char* HB_NAME_LOAD        = "Load";
static constexpr const char* HB_NAME_SAVE        = "Save";
static constexpr const char* HB_NAME_SAVE_AS     = "Save As";
static constexpr const char* HB_NAME_UNDO        = "Undo";
static constexpr const char* HB_NAME_REDO        = "Redo";
static constexpr const char* HB_NAME_ZOOM_OUT    = "Zoom Out";
static constexpr const char* HB_NAME_ZOOM_IN     = "Zoom In";
static constexpr const char* HB_NAME_PACK        = "Pack GPAK";
static constexpr const char* HB_NAME_UNPACK      = "Unpack GPAK";
static constexpr const char* HB_NAME_RUN_GAME    = "Play";
static constexpr const char* HB_NAME_PREFERENCES = "Preferences";
static constexpr const char* HB_NAME_ABOUT       = "About";
static constexpr const char* HB_NAME_HELP        = "Help";
static constexpr const char* HB_NAME_BUG_REPORT  = "Bug Report";
static constexpr const char* HB_NAME_UPDATE      = "Update";

/* -------------------------------------------------------------------------- */

static constexpr const char* HB_INFO_NEW         = "Create a new empty level.";
static constexpr const char* HB_INFO_LOAD        = "Load an existing level.";
static constexpr const char* HB_INFO_SAVE        = "Save the current level.";
static constexpr const char* HB_INFO_SAVE_AS     = "Save the current level as a new file.";
static constexpr const char* HB_INFO_UNDO        = "Undo the last recorded action.";
static constexpr const char* HB_INFO_REDO        = "Redo the last recorded action.";
static constexpr const char* HB_INFO_ZOOM_OUT    = "Zoom out the editor camera.";
static constexpr const char* HB_INFO_ZOOM_IN     = "Zoom in the editor camera.";
static constexpr const char* HB_INFO_PACK        = "Pack data files into the GPAK file format.";
static constexpr const char* HB_INFO_UNPACK      = "Unpack data files from the GPAK file format.";
static constexpr const char* HB_INFO_RUN_GAME    = "Runs The End is Nigh game application.";
static constexpr const char* HB_INFO_PREFERENCES = "Open the preferences menu to customize the editor.";
static constexpr const char* HB_INFO_ABOUT       = "Open the about menu for application information.";
static constexpr const char* HB_INFO_BUG_REPORT  = "Report technical issues or bugs with the editor.";
static constexpr const char* HB_INFO_HELP        = "Information and help about modding The End is Nigh.";
static constexpr const char* HB_INFO_UPDATE      = "There is a new update available for the editor.";

/* -------------------------------------------------------------------------- */

TEINAPI void do_hotbar ()
{
    Vec2 cursor(0,0);

    // Height for the toolbar buttons.
    float bh = HOTBAR_HEIGHT - gWindowBorder;

    SetUiTexture(&gResourceIcons);
    SetUiFont(&GetEditorRegularFont());

    UiFlag save_flags     = UI_NONE;
    UiFlag save_as_flags  = UI_NONE;
    UiFlag undo_flags     = UI_NONE;
    UiFlag redo_flags     = UI_NONE;
    UiFlag zoom_out_flags = UI_NONE;
    UiFlag zoom_in_flags  = UI_NONE;
    UiFlag pack_flags     = UI_NONE;
    UiFlag unpack_flags   = UI_NONE;

    pack_flags = (IsGPAKPackComplete()) ? UI_NONE : UI_LOCKED;
    unpack_flags = (IsGPAKUnpackComplete()) ? UI_NONE : UI_LOCKED;

    if (are_there_any_tabs())
    {
        const Tab& tab = get_current_tab();

        if (tab.type == Tab_Type::LEVEL)
        {
            if (tab.level_history.current_position == tab.level_history.state.size()-1)
            {
                redo_flags = UI_LOCKED;
            }
            if (tab.level_history.current_position <= -1)
            {
                undo_flags = UI_LOCKED;
            }
            if (tab.level_history.state.empty())
            {
                undo_flags = UI_LOCKED;
                redo_flags = UI_LOCKED;
            }
        }
        else
        {
            if (tab.map_history.current_position == tab.map_history.state.size()-1)
            {
                redo_flags = UI_LOCKED;
            }
            if (tab.map_history.current_position <= 0)
            {
                undo_flags = UI_LOCKED;
            }
            if (tab.map_history.state.size() == 1)
            {
                undo_flags = UI_LOCKED;
                redo_flags = UI_LOCKED;
            }
        }

        if (tab.type == Tab_Type::LEVEL)
        {
            zoom_out_flags = (tab.camera.zoom == MIN_LVL_EDITOR_ZOOM) ? UI_LOCKED : UI_NONE;
            zoom_in_flags  = (tab.camera.zoom == MAX_LVL_EDITOR_ZOOM) ? UI_LOCKED : UI_NONE;
        }
        else
        {
            zoom_out_flags = (tab.camera.zoom == MIN_MAP_EDITOR_ZOOM) ? UI_LOCKED : UI_NONE;
            zoom_in_flags  = (tab.camera.zoom == MAX_MAP_EDITOR_ZOOM) ? UI_LOCKED : UI_NONE;
        }
    }
    else
    {
        save_flags     = UI_LOCKED;
        save_as_flags  = UI_LOCKED;
        undo_flags     = UI_LOCKED;
        redo_flags     = UI_LOCKED;
        zoom_out_flags = UI_LOCKED;
        zoom_in_flags  = UI_LOCKED;
    }

    // The hotbar is a horizontal list of available actions.
    BeginPanel(0, 0, GetViewport().w, HOTBAR_HEIGHT, UI_NONE, gUiColorMedium);

    SetPanelCursor(&cursor);
    SetPanelCursorDir(UI_DIR_RIGHT);

    // We want these to be the same width as the toolbar (looks nice).
    float bw = TOOLBAR_WIDTH+1;

    float width = 0;

    width += CalculateTextButtonWidth(HB_NAME_NEW        );
    width += CalculateTextButtonWidth(HB_NAME_LOAD       );
    width += CalculateTextButtonWidth(HB_NAME_SAVE       );
    width += CalculateTextButtonWidth(HB_NAME_SAVE_AS    );
    width += CalculateTextButtonWidth(HB_NAME_UNDO       );
    width += CalculateTextButtonWidth(HB_NAME_REDO       );
    width += CalculateTextButtonWidth(HB_NAME_ZOOM_OUT   );
    width += CalculateTextButtonWidth(HB_NAME_ZOOM_IN    );
    width += CalculateTextButtonWidth(HB_NAME_PACK       );
    width += CalculateTextButtonWidth(HB_NAME_UNPACK     );
    width += CalculateTextButtonWidth(HB_NAME_RUN_GAME   );
    width += CalculateTextButtonWidth(HB_NAME_PREFERENCES);
    width += CalculateTextButtonWidth(HB_NAME_ABOUT      );
    width += CalculateTextButtonWidth(HB_NAME_BUG_REPORT );
    width += CalculateTextButtonWidth(HB_NAME_HELP       );
    width += (are_there_updates()) ? bw : 0;

    // Display text or icons depending on what we have room for.
    if (width < GetViewport().w)
    {
    DoTextButton(hb_new,             bh,  UI_NONE,         HB_NAME_NEW,          HB_INFO_NEW,          gKbLevelNew                          );
    DoTextButton(hb_load,            bh,  UI_NONE,         HB_NAME_LOAD,         HB_INFO_LOAD,         gKbLevelOpen                         );
    DoTextButton(hb_save,            bh,  save_flags,      HB_NAME_SAVE,         HB_INFO_SAVE,         gKbLevelSave                         );
    DoTextButton(hb_save_as,         bh,  save_as_flags,   HB_NAME_SAVE_AS,      HB_INFO_SAVE_AS,      gKbLevelSaveAs                       );
    DoTextButton(hb_undo_action,     bh,  undo_flags,      HB_NAME_UNDO,         HB_INFO_UNDO,         gKbUndo                              );
    DoTextButton(hb_redo_action,     bh,  redo_flags,      HB_NAME_REDO,         HB_INFO_REDO,         gKbRedo                              );
    DoTextButton(hb_zoom_out,        bh,  zoom_out_flags,  HB_NAME_ZOOM_OUT,     HB_INFO_ZOOM_OUT,     gKbCameraZoomOut                     );
    DoTextButton(hb_zoom_in,         bh,  zoom_in_flags,   HB_NAME_ZOOM_IN,      HB_INFO_ZOOM_IN,      gKbCameraZoomIn                      );
    DoTextButton(hb_gpak_pack,       bh,  pack_flags,      HB_NAME_PACK,         HB_INFO_PACK,         gKbGpakPack                          );
    DoTextButton(hb_gpak_unpack,     bh,  unpack_flags,    HB_NAME_UNPACK,       HB_INFO_UNPACK,       gKbGpakUnpack                        );
    DoTextButton(hb_run_game,        bh,  UI_NONE,         HB_NAME_RUN_GAME,     HB_INFO_RUN_GAME,     gKbRunGame                           );
    DoTextButton(hb_preferences,     bh,  UI_NONE,         HB_NAME_PREFERENCES,  HB_INFO_PREFERENCES,  gKbPreferences                       );
    DoTextButton(hb_about,           bh,  UI_NONE,         HB_NAME_ABOUT,        HB_INFO_ABOUT,        gKbAbout                             );
    DoTextButton(hb_bug_report,      bh,  UI_NONE,         HB_NAME_BUG_REPORT,   HB_INFO_BUG_REPORT,   gKbBugReport                         );
    DoTextButton(hb_help,            bh,  UI_NONE,         HB_NAME_HELP,         HB_INFO_HELP,         gKbHelp                              );
    }
    else
    {
    DoImageButton(hb_new,          bw,bh,  UI_NONE,         &gClipNew,            HB_INFO_NEW,          gKbLevelNew,      HB_NAME_NEW        );
    DoImageButton(hb_load,         bw,bh,  UI_NONE,         &gClipLoad,           HB_INFO_LOAD,         gKbLevelOpen,     HB_NAME_LOAD       );
    DoImageButton(hb_save,         bw,bh,  save_flags,      &gClipSave,           HB_INFO_SAVE,         gKbLevelSave,     HB_NAME_SAVE       );
    DoImageButton(hb_save,         bw,bh,  save_as_flags,   &gClipSaveAs,         HB_INFO_SAVE_AS,      gKbLevelSaveAs,   HB_NAME_SAVE_AS    );
    DoImageButton(hb_undo_action,  bw,bh,  undo_flags,      &gClipUndo,           HB_INFO_UNDO,         gKbUndo,          HB_NAME_UNDO       );
    DoImageButton(hb_redo_action,  bw,bh,  redo_flags,      &gClipRedo,           HB_INFO_REDO,         gKbRedo,          HB_NAME_REDO       );
    DoImageButton(hb_zoom_out,     bw,bh,  zoom_out_flags,  &gClipZoomOut,        HB_INFO_ZOOM_OUT,     gKbCameraZoomOut, HB_NAME_ZOOM_OUT   );
    DoImageButton(hb_zoom_in,      bw,bh,  zoom_in_flags,   &gClipZoomIn,         HB_INFO_ZOOM_IN,      gKbCameraZoomIn,  HB_NAME_ZOOM_IN    );
    DoImageButton(hb_gpak_pack,    bw,bh,  pack_flags,      &gClipPack,           HB_INFO_PACK,         gKbGpakPack,      HB_NAME_PACK       );
    DoImageButton(hb_gpak_unpack,  bw,bh,  unpack_flags,    &gClipUnpack,         HB_INFO_UNPACK,       gKbGpakUnpack,    HB_NAME_UNPACK     );
    DoImageButton(hb_run_game,     bw,bh,  UI_NONE,         &gClipRun,            HB_INFO_RUN_GAME,     gKbRunGame,       HB_NAME_RUN_GAME   );
    DoImageButton(hb_preferences,  bw,bh,  UI_NONE,         &gClipSettings,       HB_INFO_PREFERENCES,  gKbPreferences,   HB_NAME_PREFERENCES);
    DoImageButton(hb_about,        bw,bh,  UI_NONE,         &gClipAbout,          HB_INFO_ABOUT,        gKbAbout,         HB_NAME_ABOUT      );
    DoImageButton(hb_bug_report,   bw,bh,  UI_NONE,         &gClipBug,            HB_INFO_BUG_REPORT,   gKbBugReport,     HB_NAME_BUG_REPORT );
    DoImageButton(hb_help,         bw,bh,  UI_NONE,         &gClipHelp,           HB_INFO_HELP,         gKbHelp,          HB_NAME_HELP       );
    }

    // Right-aligned update button.
    if (are_there_updates())
    {
        cursor.x = GetViewport().w - (bw-1);
        DoImageButton(hb_update, bw,bh, UI_NONE, &gClipUpdate, HB_INFO_UPDATE, "", HB_NAME_UPDATE);
    }

    EndPanel();
}

/* -------------------------------------------------------------------------- */

TEINAPI void hb_new ()
{
    open_new();
}

/* -------------------------------------------------------------------------- */

TEINAPI void hb_load ()
{
    std::vector<std::string> file_names = OpenDialog(DialogType::LVL_CSV);
    if (!file_names.empty())
    {
        for (auto file: file_names)
        {
            std::string ext(file.substr(file.find_last_of(".")));
            if      (ext == ".lvl") load_level_tab(file);
            else if (ext == ".csv") load_map_tab  (file);
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void hb_save ()
{
    if (!are_there_any_tabs()) return;

    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): le_save     (get_current_tab()); break;
        case (Tab_Type::MAP  ): save_map_tab(get_current_tab()); break;
    }
}

TEINAPI void hb_save_as ()
{
    if (!are_there_any_tabs()) return;

    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): le_save_as     (); break;
        case (Tab_Type::MAP  ): save_map_tab_as(); break;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void hb_undo_action ()
{
    if (!are_there_any_tabs()) return;

    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): le_undo(); break;
        case (Tab_Type::MAP  ): me_undo(); break;
    }
}

TEINAPI void hb_redo_action ()
{
    if (!are_there_any_tabs()) return;

    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): le_redo(); break;
        case (Tab_Type::MAP  ): me_redo(); break;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void hb_history_begin ()
{
    if (!are_there_any_tabs()) return;

    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): le_history_begin(); break;
        case (Tab_Type::MAP  ): me_history_begin(); break;
    }
}

TEINAPI void hb_history_end ()
{
    if (!are_there_any_tabs()) return;

    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): le_history_end(); break;
        case (Tab_Type::MAP  ): me_history_end(); break;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void hb_zoom_out ()
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

TEINAPI void hb_zoom_in ()
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

/* -------------------------------------------------------------------------- */

TEINAPI void hb_gpak_unpack ()
{
    std::vector<std::string> files = OpenDialog(DialogType::GPAK, false);
    if (!files.empty())
    {
        bool should_overwrite = true;
        if (ShowAlert("Overwrite", "Do you want to overwrite any existing files during unpack?", ALERT_TYPE_INFO, ALERT_BUTTON_YES_NO) == ALERT_RESULT_NO)
        {
            should_overwrite = false;
        }

        GPAKUnpack(files.at(0), should_overwrite);
    }
}

TEINAPI void hb_gpak_pack ()
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

/* -------------------------------------------------------------------------- */

TEINAPI void hb_run_game ()
{
    constexpr const char* EXE_STEAM_X86 = "C:/Program Files (x86)/Steam/steamapps/common/theendisnigh/TheEndIsNigh.exe";
    constexpr const char* EXE_STEAM_X64 = "C:/Program Files/Steam/steamapps/common/theendisnigh/TheEndIsNigh.exe";
    constexpr const char* EXE_STEAM_APP = "TheEndIsNigh.exe";

    constexpr const char* EXE_EPIC_X86  = "C:/Program Files (x86)/Epic Games/theendisnigh/TheEnd.exe";
    constexpr const char* EXE_EPIC_X64  = "C:/Program Files)/Epic Games/theendisnigh/TheEnd.exe";
    constexpr const char* EXE_EPIC_APP  = "TheEnd.exe";

    const std::vector<std::string> EXECUTABLES
    {
        EXE_STEAM_X86, EXE_STEAM_X64, EXE_STEAM_APP,
        EXE_EPIC_X86, EXE_EPIC_X64, EXE_EPIC_APP
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
        for (auto exe: EXECUTABLES)
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
        open_path();
    }
    else if (!RunExecutable(executable))
    {
        LogError(ERR_MED, "Failed to launch The End is Nigh executable!");
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void hb_preferences ()
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

TEINAPI void hb_about ()
{
    if (IsWindowHidden("WINABOUT"))
    {
        ShowWindow("WINABOUT");
    }
    else
    {
        RaiseWindow("WINABOUT");
    }
}

TEINAPI void hb_help ()
{
    LoadWebpage("http://glaielgames.com/teinworkshop/moddermanual/");
}

TEINAPI void hb_bug_report ()
{
    LoadWebpage("https://itch.io/t/822898/bug-reports");
}

TEINAPI void hb_update ()
{
    open_update_window();
}

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
