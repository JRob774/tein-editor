/*******************************************************************************
 * Core editor functionality with features shared across the map/level editors.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr const char* TAB_STATE_KEY_NAME = "Software\\TheEndEditor\\PreviousSessionTabs";
static constexpr const char* SELECTED_STATE_KEY_NAME = "Software\\TheEndEditor\\SelectedTab";

static size_t tab_to_start_from_session_load = INVALID_TAB;

/* -------------------------------------------------------------------------- */

TEINAPI Tab& internal__create_new_tab_and_focus (Tab_Type type)
{
    size_t location;

    if (editor.current_tab == INVALID_TAB) location = 0; // No tabs!
    else location = editor.current_tab+1;

    editor.tabs.insert(editor.tabs.begin()+location, Tab());
    Tab& tab = editor.tabs.at(location);

    tab.type            = type;
    tab.camera.x        = 0;
    tab.camera.y        = 0;
    tab.camera.zoom     = 1;
    tab.unsaved_changes = false;

    // Set this newly added tab to be the current tab for the editor.
    editor.current_tab = location;

    set_main_window_subtitle_for_tab(tab.name);

    maybe_scroll_tab_bar();

    return tab;
}

TEINAPI u32 internal__backup_callback (u32 interval, void* user_data)
{
    push_editor_event(EDITOR_EVENT_BACKUP_TAB, NULL, NULL);

    // This tells SDL to setup the timer again to run with the new interval.
    // In this case we are just using the exact same interval as previously.
    return interval;
}

TEINAPI std::vector<std::string> internal__get_restore_files ()
{
    std::vector<std::string> files;
    list_path_files(get_executable_path(), files);

    // Remove any listed files that are not .restore files.
    std::vector<std::string> restores;
    for (auto& file: files)
    {
        std::string file_name(strip_file_path(file));
        if (insensitive_compare(".restore", file_name.substr(4, strlen(".restore")))) // NOTE: 4 because that is the length of ".csv" and ".lvl".
        {
            restores.push_back(file);
        }
    }

    return restores;
}

TEINAPI bool internal__restore_tab (std::string file_name)
{
    std::string type(strip_file_path(file_name).substr(0, 4)); // NOTE: 4 because that is the length of ".csv" and ".lvl".
    if (type == ".lvl")
    {
        create_new_level_tab_and_focus();
        Tab& tab = get_current_tab();
        return load_restore_level(tab, file_name);
    }
    if (type == ".csv")
    {
        create_new_map_tab_and_focus();
        Tab& tab = get_current_tab();
        return load_restore_map(tab, file_name);
    }
    return false;
}

#if defined(PLATFORM_WIN32)
TEINAPI void internal__load_session_tabs ()
{
    // LOAD THE PREVIOUS SESSION TABS
    {
        HKEY key;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, TAB_STATE_KEY_NAME, 0, KEY_READ, &key) != ERROR_SUCCESS)
        {
            // We don't bother logging an error because it isn't that important...
            return;
        }
        defer { RegCloseKey(key); };

        constexpr DWORD VALUE_NAME_LEN = 32767; // Docs say that is the max size of a registry value name.
        char value_name[VALUE_NAME_LEN] = {};
        LSTATUS ret = ERROR_SUCCESS;
        DWORD index = 0;
        while (ret == ERROR_SUCCESS)
        {
            DWORD value_name_len = VALUE_NAME_LEN;
            DWORD type;
            DWORD value_size;
            ret = RegEnumValueA(key, index, value_name, &value_name_len, NULL, &type, NULL, &value_size);
            if (ret == ERROR_SUCCESS)
            {
                if (value_size)
                {
                    value_name_len = VALUE_NAME_LEN;
                    std::string buffer;
                    buffer.resize(value_size-1);
                    ret = RegEnumValueA(key, index, value_name, &value_name_len, NULL, &type, reinterpret_cast<BYTE*>(&buffer[0]), &value_size);
                    // Load the actual level/map now that we have the name.
                    if (does_file_exist(buffer))
                    {
                        std::string ext(buffer.substr(buffer.find_last_of(".")));
                        Tab* tab = NULL;
                        if      (ext == ".lvl") level_drop_file(tab, buffer);
                        else if (ext == ".csv") map_drop_file  (tab, buffer);
                    }

                }
            }
            index++;
        }
    }

    // FOCUS ON PREVIOUSLY FOCUSED TAB
    {
        HKEY key;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, SELECTED_STATE_KEY_NAME, 0, KEY_READ, &key) != ERROR_SUCCESS)
        {
            // We don't bother logging an error because it isn't that important...
            return;
        }
        defer { RegCloseKey(key); };

        DWORD buffer_length = MAX_PATH;
        char buffer[MAX_PATH] = {};

        if (RegQueryValueExA(key, "szTab", NULL, NULL, reinterpret_cast<BYTE*>(&buffer[0]), &buffer_length) != ERROR_SUCCESS) return;

        tab_to_start_from_session_load = get_tab_index_with_this_file_name(buffer);
    }
}
#else
#error internal__load_session_tabs not implemented on the current platform!
#endif

#if defined(PLATFORM_WIN32)
TEINAPI void internal__save_session_tabs ()
{
    // Clear the old session tabs so that we have a fresh start for saving.
    RegDeleteKeyA(HKEY_CURRENT_USER, SELECTED_STATE_KEY_NAME);
    RegDeleteKeyA(HKEY_CURRENT_USER, TAB_STATE_KEY_NAME);

    // SAVE THE PREVIOUS SESSIONS TABS
    {
        DWORD disp;
        HKEY  key;
        LSTATUS ret = RegCreateKeyExA(HKEY_CURRENT_USER, TAB_STATE_KEY_NAME, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &disp);
        if (ret != ERROR_SUCCESS)
        {
            // We don't bother logging an error because it isn't that important...
            return;
        }
        defer { RegCloseKey(key); };

        int index = 0;
        for (auto& tab: editor.tabs)
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
        HKEY  key;
        LSTATUS ret = RegCreateKeyExA(HKEY_CURRENT_USER, SELECTED_STATE_KEY_NAME, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &disp);
        if (ret != ERROR_SUCCESS)
        {
            // We don't bother logging an error because it isn't that important...
            return;
        }
        defer { RegCloseKey(key); };

        if (are_there_any_tabs())
        {
            std::string name(editor.tabs.at(editor.current_tab).name);
            RegSetValueExA(key, "szTab", 0, REG_SZ, reinterpret_cast<const BYTE*>(name.c_str()), static_cast<DWORD>(name.length()+1));
        }
    }
}
#else
#error internal__save_session_tabs not implemented on the current platform!
#endif

/* -------------------------------------------------------------------------- */

TEINAPI void init_editor (int argc, char** argv)
{
    editor.tabs.clear();
    editor.current_tab = INVALID_TAB;

    editor.cooldown_timer = 0;

    editor.grid_visible =  true;
    editor.is_panning   = false;
    editor.dialog_box   = false;

    init_level_editor();
    init_map_editor();

    // Handle restoring levels/maps from a previous instance that crashed.
    std::vector<std::string> restore_files = internal__get_restore_files();
    bool denied_restore = false;
    if (!restore_files.empty())
    {
        if (show_alert("Restore", "Would you like to attempt to restore tabs?",
            ALERT_TYPE_INFO, ALERT_BUTTON_YES_NO, "WINMAIN") == ALERT_RESULT_YES)
        {
            for (auto& file_name: restore_files)
            {
                if (!internal__restore_tab(file_name))
                {
                    LOG_ERROR(ERR_MED, "Failed to restore '%s'!", file_name.c_str());
                    close_current_tab();
                }
                else
                {
                    remove(file_name.c_str()); // We can remove the restore file after it's loaded.
                }
            }
        }
        else
        {
            denied_restore = true;
        }
    }
    // Restore previous tabs from an instance that did not crash.
    else
    {
        internal__load_session_tabs();
    }

    // Load the files that have been passed in as command line arguments.
    if (argc > 1)
    {
        for (int i=1; i<argc; ++i)
        {
            if (!does_file_exist(argv[i]))
            {
                std::string msg(format_string("Could not find file '%s'!", argv[i]));
                show_alert("Error", msg, ALERT_TYPE_ERROR, ALERT_BUTTON_OK, "WINMAIN");
            }
            else
            {
                std::string file(argv[i]);
                std::string ext(file.substr(file.find_last_of(".")));
                Tab* tab = NULL;
                if      (ext == ".lvl") level_drop_file(tab, file);
                else if (ext == ".csv") map_drop_file  (tab, file);
            }
        }
    }
    // Otherwise just create an empty level tab and use that instead.
    if (!are_there_any_tabs())
    {
        create_new_level_tab_and_focus();
    }
}

TEINAPI void quit_editor ()
{
    internal__save_session_tabs();

    if (editor.cooldown_timer) SDL_RemoveTimer(editor.cooldown_timer);
    if (editor.backup_timer)   SDL_RemoveTimer(editor.backup_timer);
}

/* -------------------------------------------------------------------------- */

TEINAPI void do_editor ()
{
    // If the user was in a specific tab from a previous session we set to it
    // here. We do this here because it needs to be after a call to do_tab_bar
    // otherwise we will not be able to scroll correctly to the current tab.
    if (tab_to_start_from_session_load != INVALID_TAB)
    {
        set_current_tab(tab_to_start_from_session_load);
        tab_to_start_from_session_load = INVALID_TAB;
    }

    if (!are_there_any_tabs())
    {
        set_cursor(Cursor::ARROW);
        return;
    }

    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): do_level_editor(); break;
        case (Tab_Type::MAP  ): do_map_editor  (); break;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void handle_editor_events ()
{
    Tab* tab = NULL;

    if (main_event.type == SDL_DROPFILE)
    {
        if (get_window_id("WINMAIN") == main_event.drop.windowID)
        {
            std::string file(main_event.drop.file);
            std::string ext(file.substr(file.find_last_of(".")));
            if      (ext == ".lvl") level_drop_file(tab, file);
            else if (ext == ".csv") map_drop_file  (tab, file);
        }
        SDL_free(main_event.drop.file); // Docs say to free it!
    }

    if (!are_there_any_tabs()) return;

    push_editor_camera_transform();
    defer { pop_editor_camera_transform(); };

    tab = &get_current_tab();

    switch (main_event.type)
    {
        case (SDL_USEREVENT):
        {
            switch (main_event.user.code)
            {
                case (EDITOR_EVENT_BACKUP_TAB): {
                    // Go and backup every single tab that is currently open.
                    for (auto& t: editor.tabs)
                    {
                        backup_tab(t);
                    }
                } break;
                case (EDITOR_EVENT_COOLDOWN):
                {
                    editor.dialog_box = false;
                } break;
            }
        } break;
        case (SDL_QUIT):
        {
            save_prompt_all_editor_tabs();
        } break;
    }

    switch (main_event.type)
    {
        case (SDL_MOUSEWHEEL):
        {
            // Handle camera zoom!
            if (!is_key_mod_state_active(KMOD_CTRL)) return;

            if      (main_event.wheel.y > 0) tab->camera.zoom += (EDITOR_ZOOM_INCREMENT * tab->camera.zoom); // Zoom in.
            else if (main_event.wheel.y < 0) tab->camera.zoom -= (EDITOR_ZOOM_INCREMENT * tab->camera.zoom); // Zoom out.

            // Make sure the editor camera zoom stays within reasonable boundaries.
            if (tab->type == Tab_Type::LEVEL)
            {
                tab->camera.zoom = std::clamp(tab->camera.zoom, MIN_LVL_EDITOR_ZOOM, MAX_LVL_EDITOR_ZOOM);
            }
            else
            {
                tab->camera.zoom = std::clamp(tab->camera.zoom, MIN_MAP_EDITOR_ZOOM, MAX_MAP_EDITOR_ZOOM);
            }
        } break;
        case (SDL_MOUSEMOTION):
        {
            // Handle the camera panning.
            if (editor.is_panning)
            {
                tab->camera.x += static_cast<float>(main_event.motion.xrel) / tab->camera.zoom;
                tab->camera.y += static_cast<float>(main_event.motion.yrel) / tab->camera.zoom;
            }
        } break;
        case (SDL_KEYDOWN):
        case (SDL_KEYUP):
        {
            editor.is_panning = is_key_code_active(SDLK_SPACE);
        } break;
        case (SDL_MOUSEBUTTONDOWN):
        case (SDL_MOUSEBUTTONUP):
        {
            // Do not handle these events whilst we are cooling down!
            if (editor.dialog_box) return;

            bool pressed = (main_event.button.state == SDL_PRESSED);
            if (pressed && is_there_a_hit_ui_element()) return;

            if (main_event.button.button == SDL_BUTTON_MIDDLE)
            {
                editor.is_panning = pressed;
            }
        } break;
    }

    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): handle_level_editor_events(); break;
        case (Tab_Type::MAP  ): handle_map_editor_events  (); break;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void update_backup_timer ()
{
    // Remove the old backup timer if there is one currently present.
    if (editor.backup_timer) SDL_RemoveTimer(editor.backup_timer);

    // We do not fail on this because the editor will still work without this
    // system. We just let the user know that the feature is not available.
    if (editor_settings.auto_backup)
    {
        if (editor_settings.backup_interval > 0)
        {
            // Avoid any issues with overflows.
            u32 backup_interval = static_cast<u32>(editor_settings.backup_interval)*1000;
            if (static_cast<u64>(editor_settings.backup_interval)*1000 > INT_MAX)
            {
                backup_interval = INT_MAX;
            }

            editor.backup_timer = SDL_AddTimer(backup_interval, internal__backup_callback, NULL);
            if (!editor.backup_timer)
            {
                LOG_ERROR(ERR_MED, "Failed to setup backup timer system! (%s)", SDL_GetError());
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_current_tab (size_t index)
{
    // If there are no tabs then there is nothing to set.
    if (editor.tabs.empty()) return;

    if (index >= editor.tabs.size())
    {
        index = editor.tabs.size()-1;
    }

    // NOTE: Kind of a bit hacky to have these here...
    if (editor.current_tab != index)
    {
        level_editor.tool_state = Tool_State::IDLE;
        map_editor.pressed = false;
        map_editor.left_pressed = false;
    }

    editor.current_tab = index;

    // Update the title of the actual window.
    set_main_window_subtitle_for_tab(get_current_tab().name);

    maybe_scroll_tab_bar();
}

TEINAPI Tab& get_current_tab ()
{
    return editor.tabs.at(editor.current_tab);
}

TEINAPI Tab& get_tab_at_index (size_t index)
{
    if (index >= editor.tabs.size()) index = editor.tabs.size()-1;
    return editor.tabs.at(index);
}

TEINAPI bool are_there_any_tabs ()
{
    return !editor.tabs.empty();
}

/* -------------------------------------------------------------------------- */

TEINAPI void increment_tab ()
{
    if (are_there_any_tabs())
    {
        ++editor.current_tab;
        if (editor.current_tab >= editor.tabs.size())
        {
            editor.current_tab = 0;
        }
        set_main_window_subtitle_for_tab(get_current_tab().name);
        maybe_scroll_tab_bar();

        // NOTE: Kind of a bit hacky to have these here...
        level_editor.tool_state = Tool_State::IDLE;
        map_editor.pressed = false;
        map_editor.left_pressed = false;
    }
}
TEINAPI void decrement_tab ()
{
    if (are_there_any_tabs())
    {
        --editor.current_tab;
        if (editor.current_tab == INVALID_TAB)
        {
            editor.current_tab = editor.tabs.size()-1;
        }
        set_main_window_subtitle_for_tab(get_current_tab().name);
        maybe_scroll_tab_bar();

        // NOTE: Kind of a bit hacky to have these here...
        level_editor.tool_state = Tool_State::IDLE;
        map_editor.pressed = false;
        map_editor.left_pressed = false;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_main_window_subtitle_for_tab (const std::string& subtitle)
{
    set_main_window_subtitle((subtitle.empty()) ? "Untitled" : subtitle);
}

/* -------------------------------------------------------------------------- */

TEINAPI bool are_there_any_level_tabs ()
{
    for (auto tab: editor.tabs)
    {
        if (tab.type == Tab_Type::LEVEL) return true;
    }
    return false;
}
TEINAPI bool are_there_any_map_tabs ()
{
    for (auto tab: editor.tabs)
    {
        if (tab.type == Tab_Type::MAP) return true;
    }
    return false;
}

/* -------------------------------------------------------------------------- */

TEINAPI void create_new_level_tab_and_focus (int w, int h)
{
    Tab& tab = internal__create_new_tab_and_focus(Tab_Type::LEVEL);

    // Level-specific initialization stuff.
    for (auto& active: tab.tile_layer_active) active = true;
    tab.level_history.current_position = -1;
    tab.tool_info.select.cached_size   =  0;
    create_blank_level(tab.level, w, h);
}

TEINAPI void create_new_map_tab_and_focus ()
{
    Tab& tab = internal__create_new_tab_and_focus(Tab_Type::MAP);

    // Map-specific initialization stuff.
    tab.map_history.current_position = -1;
    new_map_history_state(tab.map);

    tab.map_node_info.active_pos = ivec2(0,0);
    tab.map_node_info.active     = NULL;
    tab.map_node_info.cursor     = 0;
    tab.map_node_info.select     = 0;
    tab.map_node_info.selecting  = false;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool current_tab_is_level ()
{
    if (!are_there_any_tabs()) return false;
    return (get_current_tab().type == Tab_Type::LEVEL);
}
TEINAPI bool current_tab_is_map ()
{
    if (!are_there_any_tabs()) return false;
    return (get_current_tab().type == Tab_Type::MAP);
}

/* -------------------------------------------------------------------------- */

TEINAPI void close_tab (size_t index)
{
    if (index >= editor.tabs.size()) return;

    if (save_changes_prompt(editor.tabs.at(index)) != ALERT_RESULT_CANCEL)
    {
        if (editor.closed_tabs.empty() || editor.closed_tabs.back() != editor.tabs.at(index).name)
        {
            editor.closed_tabs.push_back(editor.tabs.at(index).name);
        }
        editor.tabs.erase(editor.tabs.begin()+index);

        // NOTE: Kind of a bit hacky to have these here...
        if (editor.current_tab == index)
        {
            level_editor.tool_state = Tool_State::IDLE;
            map_editor.pressed = false;
            map_editor.left_pressed = false;
        }

        // If this is the case then there are no more tabs left.
        if (editor.tabs.empty())
        {
            editor.current_tab = INVALID_TAB;
            set_main_window_subtitle("");
        }
        else
        {
            // Ensure that the current tab value stays within the bounds.
            if (editor.current_tab >= editor.tabs.size())
            {
                set_current_tab(editor.tabs.size()-1);
            }
            else
            {
                set_main_window_subtitle_for_tab(get_current_tab().name);
            }
        }
    }
}

TEINAPI void close_current_tab ()
{
    if (!are_there_any_tabs()) return;
    else close_tab(editor.current_tab);
}

TEINAPI void close_all_tabs ()
{
    if (!are_there_any_tabs()) return;
    while (!editor.tabs.empty()) close_tab(0);
}

/* -------------------------------------------------------------------------- */

TEINAPI size_t get_tab_index_with_this_file_name (std::string file_name)
{
    for (size_t i=0; i<editor.tabs.size(); ++i)
    {
        const auto& tab = editor.tabs.at(i);
        if (tab.name == file_name) return i;
    }
    return INVALID_TAB;
}

/* -------------------------------------------------------------------------- */

TEINAPI void push_editor_camera_transform ()
{
    const Tab& tab = get_current_tab();

    push_matrix(Matrix_Mode::PROJECTION);
    push_matrix(Matrix_Mode::MODELVIEW);

    float hw = get_viewport().w / 2;
    float hh = get_viewport().h / 2;

    float hsw = (get_viewport().w / tab.camera.zoom) / 2;
    float hsh = (get_viewport().h / tab.camera.zoom) / 2;

    float l = hw - hsw;
    float r = hw + hsw;
    float b = hh + hsh;
    float t = hh - hsh;

    set_orthographic(l, r, b, t);

    translate(tab.camera.x, tab.camera.y);
}

TEINAPI void pop_editor_camera_transform ()
{
    pop_matrix(Matrix_Mode::PROJECTION);
    pop_matrix(Matrix_Mode::MODELVIEW);
}

/* -------------------------------------------------------------------------- */

TEINAPI int save_changes_prompt (Tab& tab)
{
    // Prompts user to save changes before permanently losing a level/map.
    // If there are no unsaved changes then the prompt is not presented.
    if (!tab.unsaved_changes) return ALERT_RESULT_INVALID;

    std::string tab_name((tab.name.empty()) ? "Untitled" : strip_file_path(tab.name));
    std::string msg(format_string("'%s' has unsaved changes!\nWould you like to save?", tab_name.c_str()));
    int result = show_alert("Unsaved Changes", msg, ALERT_TYPE_WARNING, ALERT_BUTTON_YES_NO_CANCEL, "WINMAIN");
    if (result == ALERT_RESULT_YES)
    {
        // The save was cancelled or there was an error so we cancel the action
        // the user was going to perform in order to maintain the level/map data.
        switch (tab.type)
        {
            case (Tab_Type::LEVEL): if (!le_save     (tab)) return ALERT_RESULT_CANCEL; break;
            case (Tab_Type::MAP  ): if (!save_map_tab(tab)) return ALERT_RESULT_CANCEL; break;
        }
    }

    // Return the result in case caller wants to handle.
    return result;
}

/* -------------------------------------------------------------------------- */

TEINAPI void backup_tab (Tab& tab)
{
    switch (tab.type)
    {
        case (Tab_Type::LEVEL): backup_level_tab(tab.level, tab.name); break;
        case (Tab_Type::MAP  ): backup_map_tab  (tab,       tab.name); break;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI bool is_current_tab_empty ()
{
    if (!are_there_any_tabs()) return false;
    const Tab& tab = get_current_tab();
    switch (tab.type)
    {
        case (Tab_Type::LEVEL): return is_current_level_empty();
        case (Tab_Type::MAP  ): return is_current_map_empty();
    }
    return false;
}

/* -------------------------------------------------------------------------- */

TEINAPI void editor_select_all ()
{
    if (!are_there_any_tabs()) return;
    Tab& tab = get_current_tab();
    switch (tab.type)
    {
        case (Tab_Type::LEVEL): le_select_all(); break;
        case (Tab_Type::MAP  ): me_select_all(); break;
    }
}

TEINAPI void editor_paste ()
{
    if (!are_there_any_tabs()) return;
    Tab& tab = get_current_tab();
    switch (tab.type)
    {
        case (Tab_Type::LEVEL): le_paste(); break;
        case (Tab_Type::MAP  ): me_paste(); break;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI bool save_prompt_all_editor_tabs ()
{
    // Go through all tabs and make sure that they get saved if the user wants.
    for (auto& t: editor.tabs)
    {
        if (save_changes_prompt(t) == ALERT_RESULT_CANCEL)
        {
            main_running = true;
            return false;
        }
    }
    return true;
}

/* -------------------------------------------------------------------------- */

TEINAPI void open_recently_closed_tab ()
{
    if (editor.closed_tabs.empty()) return;
    std::string name(editor.closed_tabs.back());
    editor.closed_tabs.pop_back();
    if (does_file_exist(name))
    {
        std::string ext(name.substr(name.find_last_of(".")));
        Tab* tab = NULL;
        if      (ext == ".lvl") level_drop_file(tab, name);
        else if (ext == ".csv") map_drop_file  (tab, name);
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void save_restore_files ()
{
    for (size_t i=0; i<editor.tabs.size(); ++i)
    {
        std::string file_name;
        if      (editor.tabs.at(i).type == Tab_Type::LEVEL) file_name = ".lvl.restore" + std::to_string(i);
        else if (editor.tabs.at(i).type == Tab_Type::MAP  ) file_name = ".csv.restore" + std::to_string(i);
        file_name = make_path_absolute(file_name);
        if      (editor.tabs.at(i).type == Tab_Type::LEVEL) save_restore_level(editor.tabs.at(i), file_name);
        else if (editor.tabs.at(i).type == Tab_Type::MAP  ) save_restore_map  (editor.tabs.at(i), file_name);
    }
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
