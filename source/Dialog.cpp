static constexpr size_t gDialogBufferSize = UINT16_MAX+1;

namespace Internal
{
    TEINAPI U32 DialogCooldownCallback (U32 interval, void* userData)
    {
        PushEditorEvent(EDITOR_EVENT_COOLDOWN, NULL, NULL);
        return 0;
    }

    TEINAPI void SetDialogCooldown ()
    {
        gEditor.cooldownTimer = SDL_AddTimer(60, DialogCooldownCallback, NULL);
        if (!gEditor.cooldownTimer) LogError(ERR_MIN, "Failed to setup dialog cooldown! (%s)", SDL_GetError());
    }
}

#if defined(PLATFORM_WIN32)
TEINAPI std::vector<std::string> OpenDialog (DialogType type, bool multiselect)
{
    gEditor.dialogBox = true; // Used to prevent dialog box clicks from carrying into the editor.

    Defer { Internal::SetDialogCooldown(); };

    const char* filter = NULL;
    const char* title = NULL;
    const char* ext = NULL;

    switch (type)
    {
        case (DialogType::LVL):
        {
            filter = "All Files (*.*)\0*.*\0LVL Files (*.lvl)\0*.lvl\0";
            title = "Open";
            ext = "lvl";
        } break;
        case (DialogType::CSV):
        {
            filter = "All Files (*.*)\0*.*\0CSV Files (*.csv)\0*.csv\0";
            title = "Open";
            ext = "csv";
        } break;
        case (DialogType::LVL_CSV):
        {
            filter = "All Files (*.*)\0*.*\0Supported Files (*.lvl; *.csv)\0*.lvl;*.csv\0CSV Files (*.csv)\0*.csv\0LVL Files (*.lvl)\0*.lvl\0";
            title = "Open";
            ext = "lvl";
        } break;
        case (DialogType::GPAK):
        {
            filter = "All Files (*.*)\0*.*\0GPAK Files (*.gpak)\0*.gpak\0";
            title = "Unpack";
            ext = "gpak";
        } break;
        case (DialogType::EXE):
        {
            filter = "All Files (*.*)\0*.*\0EXE Files (*.exe)\0*.exe\0";
            title = "Open";
            ext = "exe";
        } break;
    }

    char fileBuffer[gDialogBufferSize] = {};

    OPENFILENAMEA openFileName = {};
    openFileName.hwndOwner = Internal::Win32GetWindowHandle(GetWindowFromName("WINMAIN").window);
    openFileName.lStructSize  = sizeof(openFileName);
    openFileName.nFilterIndex = 2;
    openFileName.lpstrFilter = filter;
    openFileName.lpstrFile = fileBuffer;
    openFileName.nMaxFile = gDialogBufferSize;
    openFileName.lpstrDefExt = ext;
    openFileName.lpstrTitle = title;
    openFileName.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_EXPLORER|OFN_NOCHANGEDIR;

    if (multiselect)
    {
        openFileName.Flags |= OFN_ALLOWMULTISELECT;
    }

    std::vector<std::string> files;
    if (!GetOpenFileNameA(&openFileName))
    {
        DWORD error = CommDlgExtendedError();
        if (error) // Zero means the user cancelled -- not an actual error!
        {
            LogError(ERR_MED, "Failed to open file! (Error: 0x%X)", error);
        }
    }
    else
    {
        const char* pos = fileBuffer;
        if (*pos != '\0')
        {
            // Determines if there is just one file or multiple
            if (IsPath(pos))
            {
                // First part is always the path.
                std::string path(FixPathSlashes(pos));
                path.push_back('/');
                // Then it is followed by all the names.
                pos += openFileName.nFileOffset;
                while (*pos)
                {
                    files.push_back(path + std::string(pos));
                    pos += strlen(pos)+1;
                }
            }
            else
            {
                files.push_back(FixPathSlashes(pos));
            }
        }
    }

    return files;
}
#endif // PLATFORM_WIN32

#if defined(PLATFORM_WIN32)
TEINAPI std::string SaveDialog (DialogType type)
{
    assert(type != DialogType::LVL_CSV);

    gEditor.dialogBox = true; // Used to prevent dialog box clicks from carrying into the editor.

    Defer { Internal::SetDialogCooldown(); };

    const char* filter = NULL;
    const char* title = NULL;
    const char* ext = NULL;

    switch (type)
    {
        case (DialogType::LVL):
        {
            filter = "All Files (*.*)\0*.*\0LVL Files (*.lvl)\0*.lvl\0";
            title = "Save As";
            ext = "lvl";
        } break;
        case (DialogType::CSV):
        {
            filter = "All Files (*.*)\0*.*\0CSV Files (*.csv)\0*.csv\0";
            title = "Save As";
            ext = "csv";
        } break;
        case (DialogType::GPAK):
        {
            filter = "All Files (*.*)\0*.*\0GPAK Files (*.gpak)\0*.gpak\0";
            title = "Pack";
            ext = "gpak";
        } break;
        case (DialogType::EXE):
        {
            filter = "All Files (*.*)\0*.*\0EXE Files (*.exe)\0*.exe\0";
            title = "Save As";
            ext = "exe";
        } break;
    }

    char fileBuffer[gDialogBufferSize] = {};

    OPENFILENAMEA openFileName = {};
    openFileName.hwndOwner = Internal::Win32GetWindowHandle(GetWindowFromName("WINMAIN").window);
    openFileName.lStructSize = sizeof(openFileName);
    openFileName.nFilterIndex = 2;
    openFileName.lpstrFilter = filter;
    openFileName.lpstrFile = fileBuffer;
    openFileName.nMaxFile = gDialogBufferSize;
    openFileName.lpstrDefExt = ext;
    openFileName.lpstrTitle = title;
    openFileName.Flags = OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT|OFN_NOREADONLYRETURN|OFN_NOCHANGEDIR;

    std::string file;
    if (!GetSaveFileNameA(&openFileName))
    {
        DWORD error = CommDlgExtendedError();
        if (error) // Zero means the user cancelled -- not an actual error!
        {
            LogError(ERR_MED, "Failed to save file! (Error: 0x%X)", error);
        }
    }
    else
    {
        file = FixPathSlashes(fileBuffer);
    }

    return file;
}
#endif // PLATFORM_WIN32

#if defined(PLATFORM_WIN32)
TEINAPI std::vector<std::string> PathDialog (bool multiselect)
{
    gEditor.dialogBox = true; // Used to prevent dialog box clicks from carrying into the editor.

    Defer { Internal::SetDialogCooldown(); };

    std::vector<std::string> paths;

    IFileOpenDialog* fileDialog = NULL;
    if (!SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileDialog))))
    {
        LogError(ERR_MED, "Failed to create the folder dialog!");
        return paths;
    }
    Defer { fileDialog->Release(); };

    DWORD options;
    if (!SUCCEEDED(fileDialog->GetOptions(&options)))
    {
        LogError(ERR_MED, "Failed to get folder dialog options!");
        return paths;
    }

    if (multiselect) options |= FOS_ALLOWMULTISELECT;
    options |= FOS_PICKFOLDERS;

    fileDialog->SetOptions(options);
    if (!SUCCEEDED(fileDialog->Show(NULL)))
    {
        // No error because the user may have only cancelled...
        return paths;
    }

    IShellItemArray* shellItemArray = NULL;
    if (!SUCCEEDED(fileDialog->GetResults(&shellItemArray)))
    {
        LogError(ERR_MED, "Failed to create shell item array!");
        return paths;
    }
    Defer { shellItemArray->Release(); };

    LPWSTR result = NULL;
    DWORD itemCount = 0;

    IShellItem* shellItem = NULL;
    shellItemArray->GetCount(&itemCount);

    for (DWORD i=0; i<itemCount; ++i)
    {
        shellItemArray->GetItemAt(i, &shellItem);
        if (shellItem)
        {
            shellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &result);
            if (result)
            {
                std::wstring widePath(result);
                CoTaskMemFree(result);
                paths.push_back(std::string());
                std::string& path = paths.back();
                int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &widePath[0], static_cast<int>(widePath.size()), NULL, 0, NULL, NULL);
                path.resize(sizeNeeded, 0);
                WideCharToMultiByte(CP_UTF8, 0, &widePath[0], static_cast<int>(widePath.size()), &path[0], sizeNeeded, NULL, NULL);
            }
        }
    }

    return paths;
}
#endif // PLATFORM_WIN32
