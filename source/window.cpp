/*******************************************************************************
 * Window manager for creating and handling multiple different app windows.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr const char* WINDOW_STATE_KEY_NAME = "Software\\TheEndEditor\\WindowPlacement";

static std::vector<std::string> restore_list;
static std::map<std::string, Window> windows;

static unsigned int main_thread_id;
static bool        window_resizing;
static bool     from_manual_resize; // Hack used to solve a stupid flashing window bug with the New/Resize window.

/* -------------------------------------------------------------------------- */

TEINAPI bool internal__are_any_subwindows_open ()
{
    for (auto it: windows)
    {
        if (it.first != "WINMAIN" && !is_window_hidden(it.first)) return true;
    }
    return false;
}

TEINAPI void internal__push_quit_event ()
{
    SDL_Event e;
    SDL_zero(e);

    e.type           = SDL_QUIT;
    e.quit.type      = SDL_QUIT;
    e.quit.timestamp = SDL_GetTicks();

    SDL_PushEvent(&e);
}

/* -------------------------------------------------------------------------- */

TEINAPI int internal__resize_window (void* main_window_thread_id, SDL_Event* event)
{
    // We only care about window resizing events, ignore everything else!
    if (event->type == SDL_WINDOWEVENT)
    {
        if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
            event->window.event == SDL_WINDOWEVENT_RESIZED)
        {
            Window& window = get_window_from_id(event->window.windowID);

            float old_width  = window.width;
            float old_height = window.height;
            window.width     = static_cast<float>(event->window.data1);
            window.height    = static_cast<float>(event->window.data2);

            // If not on main thread leave early as it would be unsafe otherwise.
            // See the top of the init_window() function for a clear explanation.
            if (*reinterpret_cast<unsigned int*>(main_window_thread_id) == get_thread_id())
            {
                // Force a redraw on resize, which looks nicer than the usual glitchy
                // looking screen content when a program's window is usually resized.
                window_resizing = true;
                if (!from_manual_resize)
                {
                    if (window.resize_callback)
                    {
                        if (old_width != event->window.data1 || old_height != event->window.data2)
                        {
                            window.resize_callback();
                        }
                    }
                }
                else
                {
                    from_manual_resize = false;
                }
                window_resizing = false;
            }
        }
    }

    return 0;
}

/* -------------------------------------------------------------------------- */

#if defined(PLATFORM_WIN32)
TEINAPI void internal__load_window_state ()
{
    HKEY key;
    LSTATUS ret = RegOpenKeyExA(HKEY_CURRENT_USER, WINDOW_STATE_KEY_NAME, 0, KEY_READ, &key);
    if (ret != ERROR_SUCCESS)
    {
        // We don't bother logging an error because it isn't that important...
        return;
    }
    Defer { RegCloseKey(key); };

    DWORD dwX, dwY, dwW, dwH, dwMaximized, dwDisplayIndex;
    DWORD len = sizeof(DWORD);

    // We return so we don't set potentially invalid data as the window state.
    if (RegQueryValueExA(key, "dwBoundsX"     , 0, 0, reinterpret_cast<BYTE*>(&dwX           ), &len) != ERROR_SUCCESS) return;
    if (RegQueryValueExA(key, "dwBoundsY"     , 0, 0, reinterpret_cast<BYTE*>(&dwY           ), &len) != ERROR_SUCCESS) return;
    if (RegQueryValueExA(key, "dwBoundsW"     , 0, 0, reinterpret_cast<BYTE*>(&dwW           ), &len) != ERROR_SUCCESS) return;
    if (RegQueryValueExA(key, "dwBoundsH"     , 0, 0, reinterpret_cast<BYTE*>(&dwH           ), &len) != ERROR_SUCCESS) return;
    if (RegQueryValueExA(key, "dwMaximized"   , 0, 0, reinterpret_cast<BYTE*>(&dwMaximized   ), &len) != ERROR_SUCCESS) return;
    if (RegQueryValueExA(key, "dwDisplayIndex", 0, 0, reinterpret_cast<BYTE*>(&dwDisplayIndex), &len) != ERROR_SUCCESS) return;

    int x = dwX;
    int y = dwY;
    int w = dwW;
    int h = dwH;

    SDL_Window* win = windows.at("WINMAIN").window;
    SDL_Rect display_bounds;
    if (SDL_GetDisplayBounds(dwDisplayIndex, &display_bounds) < 0)
    {
        // We don't bother logging an error because it isn't that important...
        return;
    }

    // Make sure the window is not out of bounds at all.
    if ( x    <  display_bounds.x) x = display_bounds.x;
    if ( y    <  display_bounds.y) y = display_bounds.y;
    if ((y+h) >= display_bounds.h) y = display_bounds.h - h;
    if ((x+w) >= display_bounds.w) x = display_bounds.w - w;

    set_window_size("WINMAIN", w, h);
    set_window_pos("WINMAIN", x, y);

    if (dwMaximized)
    {
        SDL_MaximizeWindow(win);
    }
}
#else
#error internal__load_window_state not implemented on the current platform!
#endif

#if defined(PLATFORM_WIN32)
TEINAPI void internal__save_window_state ()
{
    DWORD disp;
    HKEY  key;
    LSTATUS ret = RegCreateKeyExA(HKEY_CURRENT_USER, WINDOW_STATE_KEY_NAME, 0,
        NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &disp);
    if (ret != ERROR_SUCCESS)
    {
        // We don't bother logging an error because it isn't that important...
        return;
    }
    Defer { RegCloseKey(key); };

    SDL_Window* win = windows.at("WINMAIN").window;

    int x             = MAIN_WINDOW_X;
    int y             = MAIN_WINDOW_Y;
    int w             = MAIN_WINDOW_BASE_W;
    int h             = MAIN_WINDOW_BASE_H;
    int maximized     = SDL_GetWindowFlags(win)&SDL_WINDOW_MAXIMIZED;
    int display_index = SDL_GetWindowDisplayIndex(win);

    // We do restore window so we ge the actual window pos and size.
    SDL_RestoreWindow(win);

    SDL_GetWindowPosition(win, &x,&y);
    SDL_GetWindowSize(win, &w,&h);

    DWORD dwX            = x;
    DWORD dwY            = y;
    DWORD dwW            = w;
    DWORD dwH            = h;
    DWORD dwMaximized    = maximized;
    DWORD dwDisplayIndex = display_index;

    RegSetValueExA(key, "dwBoundsX"     , 0, REG_DWORD, reinterpret_cast<BYTE*>(&dwX           ), sizeof(dwX           ));
    RegSetValueExA(key, "dwBoundsY"     , 0, REG_DWORD, reinterpret_cast<BYTE*>(&dwY           ), sizeof(dwY           ));
    RegSetValueExA(key, "dwBoundsW"     , 0, REG_DWORD, reinterpret_cast<BYTE*>(&dwW           ), sizeof(dwW           ));
    RegSetValueExA(key, "dwBoundsH"     , 0, REG_DWORD, reinterpret_cast<BYTE*>(&dwH           ), sizeof(dwH           ));
    RegSetValueExA(key, "dwMaximized"   , 0, REG_DWORD, reinterpret_cast<BYTE*>(&dwMaximized   ), sizeof(dwMaximized   ));
    RegSetValueExA(key, "dwDisplayIndex", 0, REG_DWORD, reinterpret_cast<BYTE*>(&dwDisplayIndex), sizeof(dwDisplayIndex));
}
#else
#error internal__save_window_state not implemented on the current platform!
#endif

/* -------------------------------------------------------------------------- */

TEINAPI bool create_window (std::string name, std::string title, int x, int y, int w, int h, int min_w, int min_h, u32 flags)
{
    if (windows.find(name) != windows.end())
    {
        LOG_ERROR(ERR_MAX, "Window with name \"%s\" already exists!", name.c_str());
        return false;
    }

    windows.insert(std::pair<std::string, Window>(name, Window()));
    Window& window = windows.at(name);

    // These are the required flags for all of our application sub-windows.
    flags |= (SDL_WINDOW_HIDDEN|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_OPENGL);
    window.window = SDL_CreateWindow(title.c_str(), x, y, w, h, flags);
    if (!window.window)
    {
        LOG_ERROR(ERR_MIN, "Failed to create window! (%s)", SDL_GetError());
        return false;
    }

    window.id = SDL_GetWindowID(window.window);
    if (!window.id)
    {
        LOG_ERROR(ERR_MIN, "Failed to get window ID! (%s)", SDL_GetError());
        return false;
    }

    // Only set a minimum size if both values are a valid size.
    if (min_w && min_h)
    {
        set_window_min_size(name, min_w, min_h);
    }

    // We use SDL_GetWindowSize to cache the final size of a window in case
    // any flags such as SDL_WINDOW_MAXIMIZED were used on window creation.
    int final_width;
    int final_height;

    SDL_GetWindowSize(window.window, &final_width, &final_height);

    window.width = static_cast<float>(final_width);
    window.height = static_cast<float>(final_height);

    // Default to false and it will get handled by the window event system.
    window.focus = false;
    window.mouse = false;

    LOG_DEBUG("Created Window %s", name.c_str());
    return true;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool is_window_hidden (std::string name)
{
    return (SDL_GetWindowFlags(windows.at(name).window) & SDL_WINDOW_HIDDEN);
}

TEINAPI bool is_window_focused (std::string name)
{
    return windows.at(name).focus;
}

/* -------------------------------------------------------------------------- */

TEINAPI void show_window (std::string name)
{
    set_window_pos(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(windows.at(name).window);
}

TEINAPI void raise_window (std::string name)
{
    SDL_RaiseWindow(windows.at(name).window);
}

TEINAPI void hide_window (std::string name)
{
    SDL_HideWindow(windows.at(name).window);
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_window_title (std::string name, std::string title)
{
    SDL_SetWindowTitle(windows.at(name).window, title.c_str());
}

TEINAPI void set_window_min_size (std::string name, int w, int h)
{
    SDL_SetWindowMinimumSize(windows.at(name).window, w, h);
}

TEINAPI void set_window_max_size (std::string name, int w, int h)
{
    SDL_SetWindowMaximumSize(windows.at(name).window, w, h);
}

TEINAPI void set_window_pos (std::string name, int x, int y)
{
    SDL_SetWindowPosition(windows.at(name).window, x, y);
}

TEINAPI void set_window_size (std::string name, int w, int h)
{
    SDL_SetWindowSize(windows.at(name).window, w, h);
}

#if defined(PLATFORM_WIN32)
TEINAPI void set_window_child (std::string name)
{
    HWND hwnd = internal__win32_get_window_handle(get_window(name).window);
    LONG old = GetWindowLongA(hwnd, GWL_EXSTYLE);

    SetWindowLongA(hwnd, GWL_EXSTYLE, old|WS_EX_TOOLWINDOW);
}
#else
#error set_window_child not implemented on the current platform!
#endif

/* -------------------------------------------------------------------------- */

TEINAPI bool init_window ()
{
    // The SDL docs say that event watchers can potentially be called on a
    // separate thread this means that, if this is the case, the redraw on
    // resize is potentially unstable. As a result, we pass in the ID for
    // the main thread and then when in the resize handler check to see if
    // its thread ID is the same -- if it is then we can redraw safely.
    main_thread_id = get_thread_id();

    // The SDL docs say that this should be done before creation of the window!
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,                                            8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,                                              0);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL,                                      1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,                                   3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,                                   0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

    #if defined(BUILD_DEBUG)
    std::string main_title(format_string("[DEBUG] %s (%d.%d.%d)",
        MAIN_WINDOW_TITLE, APP_VER_MAJOR,APP_VER_MINOR,APP_VER_PATCH));
    #else
    std::string main_title(format_string("%s (%d.%d.%d)",
        MAIN_WINDOW_TITLE, APP_VER_MAJOR,APP_VER_MINOR,APP_VER_PATCH));
    #endif // BUILD_DEBUG

    if (!create_window("WINMAIN", main_title,
        MAIN_WINDOW_X,MAIN_WINDOW_Y,MAIN_WINDOW_BASE_W,MAIN_WINDOW_BASE_H,
        MAIN_WINDOW_MIN_W,MAIN_WINDOW_MIN_H, MAIN_WINDOW_FLAGS))
    {
        LOG_ERROR(ERR_MAX, "Failed to create the main application window!");
        return false;
    }

    get_window("WINMAIN").close_callback = []()
    {
        internal__push_quit_event();
    };

    // This is a useful feature of SDL's that will allow us to redraw our
    // screen whilst it is being resized, which is much more aesthetically
    // appealing when compared to the usual garbage visuals on a resize.
    SDL_AddEventWatch(internal__resize_window, &main_thread_id);

    return true;
}

TEINAPI void quit_window ()
{
    // Note: We remove this so when we SDL_RestoreWindow in internal__save_window_state
    // we don't end up invoking the resize handler function internal__resize_window.
    SDL_DelEventWatch(internal__resize_window, &main_thread_id);

    hide_window("WINMAIN");
    internal__save_window_state();

    for (auto it: windows) SDL_DestroyWindow(it.second.window);
    windows.clear();
}

/* -------------------------------------------------------------------------- */

TEINAPI void handle_window_events ()
{
    if (main_event.type != SDL_WINDOWEVENT) return;

    std::string window_name = get_window_name_from_id(main_event.window.windowID);
    Window& window = get_window_from_id(main_event.window.windowID);

    switch (main_event.window.event)
    {
        case (SDL_WINDOWEVENT_FOCUS_GAINED):
        {
            // Special case for the main window to ensure all sub-windows stay on top of it.
            if (window_name == "WINMAIN" && internal__are_any_subwindows_open())
            {
                for (auto it: windows)
                {
                    if (it.first != "WINMAIN") raise_window(it.first);
                }
            }
            else
            {
                window.focus = true;
            }
        } break;
        case (SDL_WINDOWEVENT_FOCUS_LOST):
        {
            window.focus = false;
        } break;

        case (SDL_WINDOWEVENT_MINIMIZED):
        {
            if (window_name == "WINMAIN")
            {
                for (auto it: windows)
                {
                    if (it.first != "WINMAIN" && !is_window_hidden(it.first))
                    {
                        restore_list.push_back(it.first);
                        hide_window(it.first);
                    }
                }
            }
        } break;
        case (SDL_WINDOWEVENT_RESTORED):
        {
            if (window_name == "WINMAIN")
            {
                for (auto it: windows)
                {
                    if (it.first != "WINMAIN")
                    {
                        if (std::find(restore_list.begin(), restore_list.end(), it.first) != restore_list.end())
                        {
                            show_window(it.first);
                        }
                    }
                }
                restore_list.clear();
            }
        } break;

        case (SDL_WINDOWEVENT_CLOSE):
        {
            if (window.close_callback) window.close_callback();
        } break;

        case (SDL_WINDOWEVENT_ENTER):
        {
            window.mouse = true;
        } break;
        case (SDL_WINDOWEVENT_LEAVE):
        {
            window.mouse = false;
        } break;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_main_window_subtitle (std::string subtitle)
{
    #if defined(BUILD_DEBUG)
    std::string main_title(format_string("[DEBUG] %s (%d.%d.%d)",
        MAIN_WINDOW_TITLE, APP_VER_MAJOR,APP_VER_MINOR,APP_VER_PATCH));
    #else
    std::string main_title(format_string("%s (%d.%d.%d)",
        MAIN_WINDOW_TITLE, APP_VER_MAJOR,APP_VER_MINOR,APP_VER_PATCH));
    #endif

    if (!subtitle.empty())
    {
        main_title += " | ";
        main_title += subtitle;
    }

    SDL_SetWindowTitle(windows.at("WINMAIN").window, main_title.c_str());
}

/* -------------------------------------------------------------------------- */

TEINAPI void show_main_window ()
{
    internal__load_window_state();
    SDL_ShowWindow(windows.at("WINMAIN").window);
}

/* -------------------------------------------------------------------------- */

TEINAPI Window& get_focused_window ()
{
    for (auto& it: windows) if (it.second.focus) return it.second;
    return windows.at("WINMAIN");
}

TEINAPI Window& get_window (std::string name)
{
    assert(windows.find(name) != windows.end());
    return windows.at(name);
}

TEINAPI Window& get_window_from_id (Window_ID id)
{
    for (auto& it: windows) if (it.second.id == id) return it.second;
    return windows.at("WINMAIN");
}

TEINAPI Window_ID get_window_id (std::string name)
{
    return windows.at(name).id;
}

TEINAPI std::string get_window_name_from_id (Window_ID id)
{
    for (auto& it: windows) if (it.second.id == id) return it.first;
    return std::string();
}

/* -------------------------------------------------------------------------- */

TEINAPI bool is_a_window_resizing ()
{
    return window_resizing;
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
