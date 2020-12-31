static constexpr const char* gTabStateKeyName = "Software\\TheEndEditor\\PreviousSessionTabs";
static constexpr const char* gSelectedStateKeyName = "Software\\TheEndEditor\\SelectedTab";

static size_t gTabToStartFromSessionLoad = gInvalidTab;

namespace Internal
{
    TEINAPI Tab& CreateNewTabAndFocus (TabType type)
    {
        size_t location;

        if (gEditor.currentTab == gInvalidTab) location = 0; // No tabs!
        else location = gEditor.currentTab+1;

        gEditor.tabs.insert(gEditor.tabs.begin()+location, Tab());
        Tab& tab = gEditor.tabs.at(location);

        tab.type = type;
        tab.camera.x = 0;
        tab.camera.y = 0;
        tab.camera.zoom = 1;
        tab.unsavedChanges = false;

        // Set this newly added tab to be the current tab for the editor.
        gEditor.currentTab = location;

        SetMainWindowSubtitleForTab(tab.name);

        MaybeScrollTabBar();

        return tab;
    }

    TEINAPI U32 BackupCallback (U32 interval, void* userData)
    {
        PushEditorEvent(EditorEvent::BackupTab, NULL, NULL);
        // This tells SDL to setup the timer again to run with the new interval.
        // In this case we are just using the exact same interval as previously.
        return interval;
    }

    TEINAPI std::vector<std::string> GetRestoreFiles ()
    {
        std::vector<std::string> files;
        ListPathFiles(GetExecutablePath(), files);

        // Remove any listed files that are not .restore files.
        std::vector<std::string> restores;
        for (auto& file: files)
        {
            std::string fileName(StripFilePath(file));
            if (InsensitiveCompare(".restore", fileName.substr(4, strlen(".restore")))) // NOTE: 4 because that is the length of ".csv" and ".lvl".
            {
                restores.push_back(file);
            }
        }

        return restores;
    }

    TEINAPI bool RestoreTab (std::string fileName)
    {
        std::string type(StripFilePath(fileName).substr(0, 4)); // NOTE: 4 because that is the length of ".csv" and ".lvl".
        if (type == ".lvl")
        {
            CreateNewLevelTabAndFocus();
            Tab& tab = GetCurrentTab();
            return LoadRestoreLevel(tab, fileName);
        }
        if (type == ".csv")
        {
            CreateNewMapTabAndFocus();
            Tab& tab = GetCurrentTab();
            return LoadRestoreMap(tab, fileName);
        }
        return false;
    }

    #if defined(PLATFORM_WIN32)
    TEINAPI void LoadSessionTabs ()
    {
        // LOAD THE PREVIOUS SESSION TABS
        {
            HKEY key;
            if (RegOpenKeyExA(HKEY_CURRENT_USER, gTabStateKeyName, 0, KEY_READ, &key) != ERROR_SUCCESS)
            {
                // We don't bother logging an error because it isn't that important...
                return;
            }
            Defer { RegCloseKey(key); };

            constexpr DWORD ValueNameLen = 32767; // Docs say that is the max size of a registry value name.
            char valueName[ValueNameLen] = {};
            LSTATUS ret = ERROR_SUCCESS;
            DWORD index = 0;
            while (ret == ERROR_SUCCESS)
            {
                DWORD valueNameLen = ValueNameLen;
                DWORD type;
                DWORD valueSize;
                ret = RegEnumValueA(key, index, valueName, &valueNameLen, NULL, &type, NULL, &valueSize);
                if (ret == ERROR_SUCCESS)
                {
                    if (valueSize)
                    {
                        valueNameLen = ValueNameLen;
                        std::string buffer;
                        buffer.resize(valueSize-1);
                        ret = RegEnumValueA(key, index, valueName, &valueNameLen, NULL, &type, reinterpret_cast<BYTE*>(&buffer[0]), &valueSize);
                        // Load the actual level/map now that we have the name.
                        if (DoesFileExist(buffer))
                        {
                            std::string ext(buffer.substr(buffer.find_last_of(".")));
                            Tab* tab = NULL;
                            if (ext == ".lvl") LevelDropFile(tab, buffer);
                            else if (ext == ".csv") MapDropFile(tab, buffer);
                        }
                    }
                }
                index++;
            }
        }

        // FOCUS ON PREVIOUSLY FOCUSED TAB
        {
            HKEY key;
            if (RegOpenKeyExA(HKEY_CURRENT_USER, gSelectedStateKeyName, 0, KEY_READ, &key) != ERROR_SUCCESS)
            {
                // We don't bother logging an error because it isn't that important...
                return;
            }
            Defer { RegCloseKey(key); };

            DWORD bufferLength = MAX_PATH;
            char buffer[MAX_PATH] = {};

            if (RegQueryValueExA(key, "szTab", NULL, NULL, reinterpret_cast<BYTE*>(&buffer[0]), &bufferLength) != ERROR_SUCCESS) return;

            gTabToStartFromSessionLoad = GetTabIndexWithThisFileName(buffer);
        }
    }
    #endif // PLATFORM_WIN32

    #if defined(PLATFORM_WIN32)
    TEINAPI void SaveSessionTabs ()
    {
        // Clear the old session tabs so that we have a fresh start for saving.
        RegDeleteKeyA(HKEY_CURRENT_USER, gSelectedStateKeyName);
        RegDeleteKeyA(HKEY_CURRENT_USER, gTabStateKeyName);

        // SAVE THE PREVIOUS SESSIONS TABS
        {
            DWORD disp;
            HKEY key;
            LSTATUS ret = RegCreateKeyExA(HKEY_CURRENT_USER, gTabStateKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &disp);
            if (ret != ERROR_SUCCESS)
            {
                // We don't bother logging an error because it isn't that important...
                return;
            }
            Defer { RegCloseKey(key); };

            int index = 0;
            for (auto& tab: gEditor.tabs)
            {
                if (!tab.name.empty())
                {
                    RegSetValueExA(key, std::to_string(index).c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(tab.name.c_str()), static_cast<DWORD>(tab.name.length()+1));
                    index++;
                }
            }
        }
        // SAVE THE SELECTED TAB
        {
            DWORD disp;
            HKEY key;
            LSTATUS ret = RegCreateKeyExA(HKEY_CURRENT_USER, gSelectedStateKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &disp);
            if (ret != ERROR_SUCCESS)
            {
                // We don't bother logging an error because it isn't that important...
                return;
            }
            Defer { RegCloseKey(key); };

            if (AreThereAnyTabs())
            {
                std::string name(gEditor.tabs.at(gEditor.currentTab).name);
                RegSetValueExA(key, "szTab", 0, REG_SZ, reinterpret_cast<const BYTE*>(name.c_str()), static_cast<DWORD>(name.length()+1));
            }
        }
    }
    #endif // PLATFORM_WIN32
}

TEINAPI void InitEditor (int argc, char** argv)
{
    gEditor.tabs.clear();
    gEditor.currentTab = gInvalidTab;

    gEditor.cooldownTimer = 0;

    gEditor.gridVisible = true;
    gEditor.isPanning = false;
    gEditor.dialogBox = false;

    InitLevelEditor();
    InitMapEditor();

    // Handle restoring levels/maps from a previous instance that crashed.
    std::vector<std::string> restoreFiles = Internal::GetRestoreFiles();
    bool deniedRestore = false;
    if (!restoreFiles.empty())
    {
        if (ShowAlert("Restore", "Would you like to attempt to restore tabs?", AlertType::Info, AlertButton::YesNo, "WINMAIN") == AlertResult::Yes)
        {
            for (auto& fileName: restoreFiles)
            {
                if (!Internal::RestoreTab(fileName))
                {
                    LogError(ErrorLevel::Med, "Failed to restore '%s'!", fileName.c_str());
                    CloseCurrentTab();
                }
                else
                {
                    remove(fileName.c_str()); // We can remove the restore file after it's loaded.
                }
            }
        }
        else
        {
            deniedRestore = true;
        }
    }
    // Restore previous tabs from an instance that did not crash.
    else
    {
        Internal::LoadSessionTabs();
    }

    // Load the files that have been passed in as command line arguments.
    if (argc > 1)
    {
        for (int i=1; i<argc; ++i)
        {
            if (!DoesFileExist(argv[i]))
            {
                std::string msg(FormatString("Could not find file '%s'!", argv[i]));
                ShowAlert("Error", msg, AlertType::Error, AlertButton::Ok, "WINMAIN");
            }
            else
            {
                std::string file(argv[i]);
                std::string ext(file.substr(file.find_last_of(".")));
                Tab* tab = NULL;
                if (ext == ".lvl") LevelDropFile(tab, file);
                else if (ext == ".csv") MapDropFile(tab, file);
            }
        }
    }

    // Otherwise just create an empty level tab and use that instead.
    if (!AreThereAnyTabs()) CreateNewLevelTabAndFocus();
}

TEINAPI void QuitEditor ()
{
    Internal::SaveSessionTabs();

    if (gEditor.cooldownTimer) SDL_RemoveTimer(gEditor.cooldownTimer);
    if (gEditor.backupTimer) SDL_RemoveTimer(gEditor.backupTimer);
}

TEINAPI void DoEditor ()
{
    // If the user was in a specific tab from a previous session we set to it
    // here. We do this here because it needs to be after a call to do_tab_bar
    // otherwise we will not be able to scroll correctly to the current tab.
    if (gTabToStartFromSessionLoad != gInvalidTab)
    {
        SetCurrentTab(gTabToStartFromSessionLoad);
        gTabToStartFromSessionLoad = gInvalidTab;
    }

    if (!AreThereAnyTabs())
    {
        SetCursorType(Cursor::Arrow);
        return;
    }

    switch (GetCurrentTab().type)
    {
        case (TabType::Level): DoLevelEditor(); break;
        case (TabType::Map): DoMapEditor(); break;
    }
}

TEINAPI void HandleEditorEvents ()
{
    Tab* tab = NULL;

    if (gMainEvent.type == SDL_DROPFILE)
    {
        if (GetWindowIDFromName("WINMAIN") == gMainEvent.drop.windowID)
        {
            std::string file(gMainEvent.drop.file);
            std::string ext(file.substr(file.find_last_of(".")));
            if (ext == ".lvl") LevelDropFile(tab, file);
            else if (ext == ".csv") MapDropFile(tab, file);
        }
        SDL_free(gMainEvent.drop.file); // Docs say to free it!
    }

    if (!AreThereAnyTabs()) return;

    PushEditorCameraTransform();
    Defer { PopEditorCameraTransform(); };

    tab = &GetCurrentTab();

    switch (gMainEvent.type)
    {
        case (SDL_USEREVENT):
        {
            switch (gMainEvent.user.code)
            {
                case (EditorEvent::BackupTab):
                {
                    // Go and backup every single tab that is currently open.
                    for (auto& t: gEditor.tabs) BackupTab(t);
                } break;
                case (EditorEvent::Cooldown):
                {
                    gEditor.dialogBox = false;
                } break;
            }
        } break;
        case (SDL_QUIT):
        {
            SavePromptAllEditorTabs();
        } break;
    }

    switch (gMainEvent.type)
    {
        case (SDL_MOUSEWHEEL):
        {
            // Handle camera zoom!
            if (!IsKeyModStateActive(KMOD_CTRL)) return;

            if (gMainEvent.wheel.y > 0) tab->camera.zoom += (gEditorZoomIncrement * tab->camera.zoom); // Zoom in.
            else if (gMainEvent.wheel.y < 0) tab->camera.zoom -= (gEditorZoomIncrement * tab->camera.zoom); // Zoom out.

            // Make sure the editor camera zoom stays within reasonable boundaries.
            if (tab->type == TabType::Level) tab->camera.zoom = std::clamp(tab->camera.zoom, gMinLevelEditorZoom, gMaxLevelEditorZoom);
            else tab->camera.zoom = std::clamp(tab->camera.zoom, gMinMapEditorZoom, gMaxMapEditorZoom);
        } break;
        case (SDL_MOUSEMOTION):
        {
            // Handle the camera panning.
            if (gEditor.isPanning)
            {
                tab->camera.x += static_cast<float>(gMainEvent.motion.xrel) / tab->camera.zoom;
                tab->camera.y += static_cast<float>(gMainEvent.motion.yrel) / tab->camera.zoom;
            }
        } break;
        case (SDL_KEYDOWN):
        case (SDL_KEYUP):
        {
            gEditor.isPanning = IsKeyCodeActive(SDLK_SPACE);
        } break;
        case (SDL_MOUSEBUTTONDOWN):
        case (SDL_MOUSEBUTTONUP):
        {
            // Do not handle these events whilst we are cooling down!
            if (gEditor.dialogBox) return;

            bool pressed = (gMainEvent.button.state == SDL_PRESSED);
            if (pressed && IsThereAHitUiElement()) return;

            if (gMainEvent.button.button == SDL_BUTTON_MIDDLE)
            {
                gEditor.isPanning = pressed;
            }
        } break;
    }

    switch (GetCurrentTab().type)
    {
        case (TabType::Level): HandleLevelEditorEvents(); break;
        case (TabType::Map): HandleMapEditorEvents(); break;
    }
}

TEINAPI void UpdateBackupTimer ()
{
    // Remove the old backup timer if there is one currently present.
    if (gEditor.backupTimer) SDL_RemoveTimer(gEditor.backupTimer);

    // We do not fail on this because the editor will still work without this
    // system. We just let the user know that the feature is not available.
    if (gEditorSettings.autoBackup)
    {
        if (gEditorSettings.backupInterval > 0)
        {
            // Avoid any issues with overflows.
            U32 backupInterval = static_cast<U32>(gEditorSettings.backupInterval)*1000;
            if (static_cast<U64>(gEditorSettings.backupInterval)*1000 > INT_MAX) backupInterval = INT_MAX;
            gEditor.backupTimer = SDL_AddTimer(backupInterval, Internal::BackupCallback, NULL);
            if (!gEditor.backupTimer)
            {
                LogError(ErrorLevel::Med, "Failed to setup backup timer system! (%s)", SDL_GetError());
            }
        }
    }
}

TEINAPI void SetCurrentTab (size_t index)
{
    // If there are no tabs then there is nothing to set.
    if (gEditor.tabs.empty()) return;

    if (index >= gEditor.tabs.size()) index = gEditor.tabs.size()-1;

    // NOTE: Kind of a bit hacky to have these here...
    if (gEditor.currentTab != index)
    {
        gLevelEditor.toolState = ToolState::Idle;
        gMapEditor.pressed = false;
        gMapEditor.leftPressed = false;
    }

    gEditor.currentTab = index;

    // Update the title of the actual window.
    SetMainWindowSubtitleForTab(GetCurrentTab().name);

    MaybeScrollTabBar();
}

TEINAPI Tab& GetCurrentTab ()
{
    return gEditor.tabs.at(gEditor.currentTab);
}

TEINAPI Tab& GetTabAtIndex (size_t index)
{
    if (index >= gEditor.tabs.size()) index = gEditor.tabs.size()-1;
    return gEditor.tabs.at(index);
}

TEINAPI bool AreThereAnyTabs ()
{
    return !gEditor.tabs.empty();
}

TEINAPI void IncrementTab ()
{
    if (AreThereAnyTabs())
    {
        ++gEditor.currentTab;
        if (gEditor.currentTab >= gEditor.tabs.size())
        {
            gEditor.currentTab = 0;
        }
        SetMainWindowSubtitleForTab(GetCurrentTab().name);
        MaybeScrollTabBar();

        // NOTE: Kind of a bit hacky to have these here...
        gLevelEditor.toolState = ToolState::Idle;
        gMapEditor.pressed = false;
        gMapEditor.leftPressed = false;
    }
}
TEINAPI void DecrementTab ()
{
    if (AreThereAnyTabs())
    {
        --gEditor.currentTab;
        if (gEditor.currentTab == gInvalidTab)
        {
            gEditor.currentTab = gEditor.tabs.size()-1;
        }
        SetMainWindowSubtitleForTab(GetCurrentTab().name);
        MaybeScrollTabBar();

        // NOTE: Kind of a bit hacky to have these here...
        gLevelEditor.toolState = ToolState::Idle;
        gMapEditor.pressed = false;
        gMapEditor.leftPressed = false;
    }
}

TEINAPI void SetMainWindowSubtitleForTab (const std::string& subtitle)
{
    SetMainWindowSubtitle((subtitle.empty()) ? "Untitled" : subtitle);
}

TEINAPI bool AreThereAnyLevelTabs ()
{
    for (auto tab: gEditor.tabs) if (tab.type == TabType::Level) return true;
    return false;
}
TEINAPI bool AreThereAnyMapTabs ()
{
    for (auto tab: gEditor.tabs) if (tab.type == TabType::Map) return true;
    return false;
}

TEINAPI void CreateNewLevelTabAndFocus (int w, int h)
{
    Tab& tab = Internal::CreateNewTabAndFocus(TabType::Level);
    // Level-specific initialization stuff.
    for (auto& active: tab.tileLayerActive) active = true;
    tab.levelHistory.currentPosition = -1;
    tab.toolInfo.select.cachedSize = 0;
    CreateBlankLevel(tab.level, w, h);
}
TEINAPI void CreateNewMapTabAndFocus ()
{
    Tab& tab = Internal::CreateNewTabAndFocus(TabType::Map);
    // Map-specific initialization stuff.
    tab.mapHistory.currentPosition = -1;
    NewMapHistoryState(tab.map);
    tab.mapNodeInfo.activePos = IVec2(0,0);
    tab.mapNodeInfo.active = NULL;
    tab.mapNodeInfo.cursor = 0;
    tab.mapNodeInfo.select = 0;
    tab.mapNodeInfo.selecting = false;
}

TEINAPI bool CurrentTabIsLevel ()
{
    if (!AreThereAnyTabs()) return false;
    return (GetCurrentTab().type == TabType::Level);
}
TEINAPI bool CurrentTabIsMap ()
{
    if (!AreThereAnyTabs()) return false;
    return (GetCurrentTab().type == TabType::Map);
}

TEINAPI void CloseTab (size_t index)
{
    if (index >= gEditor.tabs.size()) return;

    if (SaveChangesPrompt(gEditor.tabs.at(index)) != AlertResult::Cancel)
    {
        if (gEditor.closedTabs.empty() || gEditor.closedTabs.back() != gEditor.tabs.at(index).name)
        {
            gEditor.closedTabs.push_back(gEditor.tabs.at(index).name);
        }
        gEditor.tabs.erase(gEditor.tabs.begin()+index);

        // NOTE: Kind of a bit hacky to have these here...
        if (gEditor.currentTab == index)
        {
            gLevelEditor.toolState = ToolState::Idle;
            gMapEditor.pressed = false;
            gMapEditor.leftPressed = false;
        }

        // If this is the case then there are no more tabs left.
        if (gEditor.tabs.empty())
        {
            gEditor.currentTab = gInvalidTab;
            SetMainWindowSubtitle("");
        }
        else
        {
            // Ensure that the current tab value stays within the bounds.
            if (gEditor.currentTab >= gEditor.tabs.size()) SetCurrentTab(gEditor.tabs.size()-1);
            else SetMainWindowSubtitleForTab(GetCurrentTab().name);
        }
    }
}

TEINAPI void CloseCurrentTab ()
{
    if (!AreThereAnyTabs()) return;
    else CloseTab(gEditor.currentTab);
}
TEINAPI void CloseAllTabs ()
{
    if (!AreThereAnyTabs()) return;
    while (!gEditor.tabs.empty()) CloseTab(0);
}

TEINAPI size_t GetTabIndexWithThisFileName (std::string fileName)
{
    for (size_t i=0; i<gEditor.tabs.size(); ++i)
    {
        const auto& tab = gEditor.tabs.at(i);
        if (tab.name == fileName) return i;
    }
    return gInvalidTab;
}

TEINAPI void PushEditorCameraTransform ()
{
    const Tab& tab = GetCurrentTab();

    PushMatrix(MatrixMode::Projection);
    PushMatrix(MatrixMode::ModelView);

    float hw = GetViewport().w / 2;
    float hh = GetViewport().h / 2;

    float hsw = (GetViewport().w / tab.camera.zoom) / 2;
    float hsh = (GetViewport().h / tab.camera.zoom) / 2;

    float l = hw - hsw;
    float r = hw + hsw;
    float b = hh + hsh;
    float t = hh - hsh;

    SetOrthographic(l,r,b,t);

    Translate(tab.camera.x, tab.camera.y);
}
TEINAPI void PopEditorCameraTransform ()
{
    PopMatrix(MatrixMode::Projection);
    PopMatrix(MatrixMode::ModelView);
}

TEINAPI AlertResult SaveChangesPrompt (Tab& tab)
{
    // Prompts user to save changes before permanently losing a level/map.
    // If there are no unsaved changes then the prompt is not presented.
    if (!tab.unsavedChanges) return AlertResult::Invalid;

    std::string tabName((tab.name.empty()) ? "Untitled" : StripFilePath(tab.name));
    std::string msg(FormatString("'%s' has unsaved changes!\nWould you like to save?", tabName.c_str()));
    AlertResult result = ShowAlert("Unsaved Changes", msg, AlertType::Warning, AlertButton::YesNoCancel, "WINMAIN");
    if (result == AlertResult::Yes)
    {
        // The save was cancelled or there was an error so we cancel the action
        // the user was going to perform in order to maintain the level/map data.
        switch (tab.type)
        {
            case (TabType::Level): if (!LevelEditorSave(tab)) return AlertResult::Cancel; break;
            case (TabType::Map): if (!SaveMapTab(tab)) return AlertResult::Cancel; break;
        }
    }

    // Return the result in case caller wants to handle.
    return result;
}

TEINAPI void BackupTab (Tab& tab)
{
    switch (tab.type)
    {
        case (TabType::Level): BackupLevelTab(tab.level, tab.name); break;
        case (TabType::Map): BackupMapTab(tab, tab.name); break;
    }
}

TEINAPI bool IsCurrentTabEmpty ()
{
    if (!AreThereAnyTabs()) return false;
    const Tab& tab = GetCurrentTab();
    switch (tab.type)
    {
        case (TabType::Level): return IsCurrentLevelEmpty();
        case (TabType::Map): return IsCurrentMapEmpty();
    }
    return false;
}

TEINAPI void EditorSelectAll ()
{
    if (!AreThereAnyTabs()) return;
    Tab& tab = GetCurrentTab();
    switch (tab.type)
    {
        case (TabType::Level): LevelEditorSelectAll(); break;
        case (TabType::Map): MapEditorSelectAll(); break;
    }
}

TEINAPI void EditorPaste ()
{
    if (!AreThereAnyTabs()) return;
    Tab& tab = GetCurrentTab();
    switch (tab.type)
    {
        case (TabType::Level): LevelEditorPaste(); break;
        case (TabType::Map): MapEditorPaste(); break;
    }
}

TEINAPI bool SavePromptAllEditorTabs ()
{
    // Go through all tabs and make sure that they get saved if the user wants.
    for (auto& t: gEditor.tabs)
    {
        if (SaveChangesPrompt(t) == AlertResult::Cancel)
        {
            gMainRunning = true;
            return false;
        }
    }
    return true;
}

TEINAPI void OpenRecentlyClosedTab ()
{
    if (gEditor.closedTabs.empty()) return;
    std::string name(gEditor.closedTabs.back());
    gEditor.closedTabs.pop_back();
    if (DoesFileExist(name))
    {
        std::string ext(name.substr(name.find_last_of(".")));
        Tab* tab = NULL;
        if (ext == ".lvl") LevelDropFile(tab, name);
        else if (ext == ".csv") MapDropFile(tab, name);
    }
}

TEINAPI void SaveRestoreFiles ()
{
    for (size_t i=0; i<gEditor.tabs.size(); ++i)
    {
        std::string fileName;
        if (gEditor.tabs.at(i).type == TabType::Level) fileName = ".lvl.restore" + std::to_string(i);
        else if (gEditor.tabs.at(i).type == TabType::Map) fileName = ".csv.restore" + std::to_string(i);
        fileName = MakePathAbsolute(fileName);
        if (gEditor.tabs.at(i).type == TabType::Level) SaveRestoreLevel(gEditor.tabs.at(i), fileName);
        else if (gEditor.tabs.at(i).type == TabType::Map) SaveRestoreMap(gEditor.tabs.at(i), fileName);
    }
}
