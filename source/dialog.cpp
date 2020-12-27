/*******************************************************************************
 * Functionality for platform-specific open/save/path dialog operations.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr size_t DIALOG_BUFFER_SIZE = UINT16_MAX+1;

/* -------------------------------------------------------------------------- */

FILDEF u32 internal__dialog_cooldown_callback (u32 interval, void* user_data)
{
    push_editor_event(EDITOR_EVENT_COOLDOWN, NULL, NULL);
    return 0;
}

FILDEF void internal__set_dialog_cooldown ()
{
    editor.cooldown_timer = SDL_AddTimer(60, internal__dialog_cooldown_callback, NULL);
    if (!editor.cooldown_timer)
    {
        LOG_ERROR(ERR_MIN, "Failed to setup dialog cooldown! (%s)", SDL_GetError());
    }
}

/* -------------------------------------------------------------------------- */

#if defined(PLATFORM_WIN32)
STDDEF std::vector<std::string> open_dialog (Dialog_Type type, bool multiselect)
{
    // NOTE: Used to prevent dialog box clicks from carrying into the editor.
    editor.dialog_box = true;

    defer { internal__set_dialog_cooldown(); };

    const char* filter = NULL;
    const char* title  = NULL;
    const char* ext    = NULL;

    switch (type)
    {
    case (Dialog_Type::LVL    ): { filter = "All Files (*.*)\0*.*\0LVL Files (*.lvl)\0*.lvl\0";                                                                        title = "Open";   ext = "lvl";  } break;
    case (Dialog_Type::CSV    ): { filter = "All Files (*.*)\0*.*\0CSV Files (*.csv)\0*.csv\0";                                                                        title = "Open";   ext = "csv";  } break;
    case (Dialog_Type::LVL_CSV): { filter = "All Files (*.*)\0*.*\0Supported Files (*.lvl; *.csv)\0*.lvl;*.csv\0CSV Files (*.csv)\0*.csv\0LVL Files (*.lvl)\0*.lvl\0"; title = "Open";   ext = "lvl";  } break;
    case (Dialog_Type::GPAK   ): { filter = "All Files (*.*)\0*.*\0GPAK Files (*.gpak)\0*.gpak\0";                                                                     title = "Unpack"; ext = "gpak"; } break;
    case (Dialog_Type::EXE    ): { filter = "All Files (*.*)\0*.*\0EXE Files (*.exe)\0*.exe\0";                                                                        title = "Open";   ext = "exe";  } break;
    }

    char file_buffer[DIALOG_BUFFER_SIZE] = {};

    OPENFILENAMEA open_file_name = {};

    open_file_name.hwndOwner    = internal__win32_get_window_handle(get_window("WINMAIN").window);
    open_file_name.lStructSize  = sizeof(OPENFILENAMEA);
    open_file_name.nFilterIndex = 2;
    open_file_name.lpstrFilter  = filter;
    open_file_name.lpstrFile    = file_buffer;
    open_file_name.nMaxFile     = DIALOG_BUFFER_SIZE;
    open_file_name.lpstrDefExt  = ext;
    open_file_name.lpstrTitle   = title;
    open_file_name.Flags        = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_EXPLORER|OFN_NOCHANGEDIR;

    if (multiselect)
    {
        open_file_name.Flags |= OFN_ALLOWMULTISELECT;
    }

    std::vector<std::string> files;
    if (!GetOpenFileNameA(&open_file_name))
    {
        DWORD error = CommDlgExtendedError();
        if (error) // Zero means the user cancelled -- not an actual error!
        {
            LOG_ERROR(ERR_MED, "Failed to open file! (Error: 0x%X)", error);
        }
    }
    else
    {
        const char* pos = file_buffer;
        if (*pos != '\0')
        {
            // Determines if there is just one file or multiple
            if (is_path(pos))
            {
                // First part is always the path.
                std::string path(fix_path_slashes(pos));
                path.push_back('/');
                // Then it is followed by all the names.
                pos += open_file_name.nFileOffset;
                while (*pos)
                {
                    files.push_back(path + std::string(pos));
                    pos += strlen(pos)+1;
                }
            }
            else
            {
                files.push_back(fix_path_slashes(pos));
            }
        }
    }

    return files;
}
#else
#error open_dialog not implemented on the current platform!
#endif

/* -------------------------------------------------------------------------- */

#if defined(PLATFORM_WIN32)
STDDEF std::string save_dialog (Dialog_Type type)
{
    ASSERT(type != Dialog_Type::LVL_CSV);

    // NOTE: Used to prevent dialog box clicks from carrying into the editor.
    editor.dialog_box = true;

    defer { internal__set_dialog_cooldown(); };

    const char* filter = NULL;
    const char* title  = NULL;
    const char* ext    = NULL;

    switch (type)
    {
    case (Dialog_Type::LVL ): { filter = "All Files (*.*)\0*.*\0LVL Files (*.lvl)\0*.lvl\0";    title = "Save As"; ext = "lvl";  } break;
    case (Dialog_Type::CSV ): { filter = "All Files (*.*)\0*.*\0CSV Files (*.csv)\0*.csv\0";    title = "Save As"; ext = "csv";  } break;
    case (Dialog_Type::GPAK): { filter = "All Files (*.*)\0*.*\0GPAK Files (*.gpak)\0*.gpak\0"; title = "Pack";    ext = "gpak"; } break;
    case (Dialog_Type::EXE ): { filter = "All Files (*.*)\0*.*\0EXE Files (*.exe)\0*.exe\0";    title = "Save As"; ext = "exe";  } break;
    }

    char file_buffer[DIALOG_BUFFER_SIZE] = {};

    OPENFILENAMEA open_file_name = {};

    open_file_name.hwndOwner    = internal__win32_get_window_handle(get_window("WINMAIN").window);
    open_file_name.lStructSize  = sizeof(OPENFILENAMEA);
    open_file_name.nFilterIndex = 2;
    open_file_name.lpstrFilter  = filter;
    open_file_name.lpstrFile    = file_buffer;
    open_file_name.nMaxFile     = DIALOG_BUFFER_SIZE;
    open_file_name.lpstrDefExt  = ext;
    open_file_name.lpstrTitle   = title;
    open_file_name.Flags        = OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT|OFN_NOREADONLYRETURN|OFN_NOCHANGEDIR;

    std::string file;
    if (!GetSaveFileNameA(&open_file_name))
    {
        DWORD error = CommDlgExtendedError();
        if (error) // Zero means the user cancelled -- not an actual error!
        {
            LOG_ERROR(ERR_MED, "Failed to save file! (Error: 0x%X)", error);
        }
    }
    else
    {
        file = fix_path_slashes(file_buffer);
    }

    return file;
}
#else
#error save_dialog not implemented on the current platform!
#endif

/* -------------------------------------------------------------------------- */

#if defined(PLATFORM_WIN32)
STDDEF std::vector<std::string> path_dialog (bool multiselect)
{
    // NOTE: Used to prevent dialog box clicks from carrying into the editor.
    editor.dialog_box = true;

    defer { internal__set_dialog_cooldown(); };

    std::vector<std::string> paths;

    IFileOpenDialog* file_dialog = NULL;
    if (!SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&file_dialog))))
    {
        LOG_ERROR(ERR_MED, "Failed to create the folder dialog!");
        return paths;
    }
    defer { file_dialog->Release(); };

    DWORD options;
    if (!SUCCEEDED(file_dialog->GetOptions(&options)))
    {
        LOG_ERROR(ERR_MED, "Failed to get folder dialog options!");
        return paths;
    }

    if (multiselect) options |= FOS_ALLOWMULTISELECT;
    options |= FOS_PICKFOLDERS;

    file_dialog->SetOptions(options);
    if (!SUCCEEDED(file_dialog->Show(NULL)))
    {
        // No error because the user may have only cancelled...
        return paths;
    }

    IShellItemArray* shell_item_array = NULL;
    if (!SUCCEEDED(file_dialog->GetResults(&shell_item_array)))
    {
        LOG_ERROR(ERR_MED, "Failed to create shell item array!");
        return paths;
    }
    defer { shell_item_array->Release(); };

    LPWSTR result     = NULL;
    DWORD  item_count = 0;

    IShellItem* shell_item = NULL;
    shell_item_array->GetCount(&item_count);

    for (DWORD i=0; i<item_count; ++i)
    {
        shell_item_array->GetItemAt(i, &shell_item);
        if (shell_item)
        {
            shell_item->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &result);
            if (result)
            {
                std::wstring wide_path(result);
                CoTaskMemFree(result);

                paths.push_back(std::string());
                std::string& path = paths.back();

                int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wide_path[0], static_cast<int>(wide_path.size()), NULL, 0, NULL, NULL);
                path.resize(size_needed, 0);
                WideCharToMultiByte(CP_UTF8, 0, &wide_path[0], static_cast<int>(wide_path.size()), &path[0], size_needed, NULL, NULL);
            }
        }
    }

    return paths;
}
#else
#error path_dialog not implemented on the current platform!
#endif

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
