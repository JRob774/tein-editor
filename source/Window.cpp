static constexpr const char* gWindowStateKeyName = "Software\\TheEndEditor\\WindowPlacement";

static std::vector<std::string> gRestoreList;
static std::map<std::string, Window> gWindows;

static unsigned int gMainThreadID;
static bool gWindowResizing;
static bool gFromManualResize; // Hack used to solve a stupid flashing window bug with the New/Resize window.

namespace Internal
{
    TEINAPI bool AreAnySubWindowsOpen ()
    {
        for (auto [name,window]: gWindows) if (name != "WINMAIN" && !IsWindowHidden(name)) return true;
        return false;
    }

    TEINAPI void PushQuitEvent ()
    {
        SDL_Event e;
        SDL_zero(e);
        e.type = SDL_QUIT;
        e.quit.type = SDL_QUIT;
        e.quit.timestamp = SDL_GetTicks();
        SDL_PushEvent(&e);
    }

    TEINAPI int ResizeWindow (void* mainWindowThreadID, SDL_Event* event)
    {
        // We only care about window resizing events, ignore everything else!
        if (event->type == SDL_WINDOWEVENT)
        {
            if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                Window& window = GetWindowFromID(event->window.windowID);

                float oldWidth = window.width;
                float oldHeight = window.height;

                window.width = static_cast<float>(event->window.data1);
                window.height = static_cast<float>(event->window.data2);

                // If not on main thread leave early as it would be unsafe otherwise.
                // See the top of the init_window() function for a clear explanation.
                if (*reinterpret_cast<unsigned int*>(mainWindowThreadID) == GetThreadID())
                {
                    // Force a redraw on resize, which looks nicer than the usual glitchy
                    // looking screen content when a program's window is usually resized.
                    gWindowResizing = true;
                    if (!gFromManualResize)
                    {
                        if (window.resizeCallback)
                        {
                            if (oldWidth != event->window.data1 || oldHeight != event->window.data2)
                            {
                                window.resizeCallback();
                            }
                        }
                    }
                    else
                    {
                        gFromManualResize = false;
                    }

                    gWindowResizing = false;
                }
            }
        }

        return 0;
    }

    #if defined(PLATFORM_WIN32)
    TEINAPI void LoadWindowState ()
    {
        HKEY key;
        LSTATUS ret = RegOpenKeyExA(HKEY_CURRENT_USER, gWindowStateKeyName, 0, KEY_READ, &key);
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

        SDL_Window* win = gWindows.at("WINMAIN").window;
        SDL_Rect displayBounds;
        if (SDL_GetDisplayBounds(dwDisplayIndex, &displayBounds) < 0)
        {
            // We don't bother logging an error because it isn't that important...
            return;
        }

        // Make sure the window is not out of bounds at all.
        if ( x    <  displayBounds.x) x = displayBounds.x;
        if ( y    <  displayBounds.y) y = displayBounds.y;
        if ((y+h) >= displayBounds.h) y = displayBounds.h - h;
        if ((x+w) >= displayBounds.w) x = displayBounds.w - w;

        SetWindowSize("WINMAIN", w, h);
        SetWindowPos("WINMAIN", x, y);

        if (dwMaximized)
        {
            SDL_MaximizeWindow(win);
        }
    }
    #endif // PLATFORM_WIN32

    #if defined(PLATFORM_WIN32)
    TEINAPI void SaveWindowState ()
    {
        DWORD disp;
        HKEY key;
        LSTATUS ret = RegCreateKeyExA(HKEY_CURRENT_USER, gWindowStateKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &disp);
        if (ret != ERROR_SUCCESS)
        {
            // We don't bother logging an error because it isn't that important...
            return;
        }
        Defer { RegCloseKey(key); };

        SDL_Window* win = gWindows.at("WINMAIN").window;

        int x = gMainWindowX;
        int y = gMainWindowY;
        int w = gMainWindowBaseW;
        int h = gMainWindowBaseH;
        int maximized = SDL_GetWindowFlags(win) & SDL_WINDOW_MAXIMIZED;
        int displayIndex = SDL_GetWindowDisplayIndex(win);

        // We do restore window so we ge the actual window pos and size.
        SDL_RestoreWindow(win);

        SDL_GetWindowPosition(win, &x,&y);
        SDL_GetWindowSize(win, &w,&h);

        DWORD dwX = x;
        DWORD dwY = y;
        DWORD dwW = w;
        DWORD dwH = h;
        DWORD dwMaximized = maximized;
        DWORD dwDisplayIndex = displayIndex;

        RegSetValueExA(key, "dwBoundsX"     , 0, REG_DWORD, reinterpret_cast<BYTE*>(&dwX           ), sizeof(dwX           ));
        RegSetValueExA(key, "dwBoundsY"     , 0, REG_DWORD, reinterpret_cast<BYTE*>(&dwY           ), sizeof(dwY           ));
        RegSetValueExA(key, "dwBoundsW"     , 0, REG_DWORD, reinterpret_cast<BYTE*>(&dwW           ), sizeof(dwW           ));
        RegSetValueExA(key, "dwBoundsH"     , 0, REG_DWORD, reinterpret_cast<BYTE*>(&dwH           ), sizeof(dwH           ));
        RegSetValueExA(key, "dwMaximized"   , 0, REG_DWORD, reinterpret_cast<BYTE*>(&dwMaximized   ), sizeof(dwMaximized   ));
        RegSetValueExA(key, "dwDisplayIndex", 0, REG_DWORD, reinterpret_cast<BYTE*>(&dwDisplayIndex), sizeof(dwDisplayIndex));
    }
    #endif // PLATFORM_WIN32
}

//
// GLOBAL WINDOW FUNCTIONALITY
//

TEINAPI bool InitWindow ()
{
    // The SDL docs say that event watchers can potentially be called on a
    // separate thread this means that, if this is the case, the redraw on
    // resize is potentially unstable. As a result, we pass in the ID for
    // the main thread and then when in the resize handler check to see if
    // its thread ID is the same -- if it is then we can redraw safely.
    gMainThreadID = GetThreadID();

    // The SDL docs say that this should be done before creation of the window!
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

    #if defined(BuildDebug)
    std::string mainTitle(FormatString("[DEBUG] %s (%d.%d.%d)", gMainWindowTitle, gAppVerMajor,gAppVerMinor,gAppVerPatch));
    #else
    std::string mainTitle(FormatString("%s (%d.%d.%d)", gMainWindowTitle, gAppVerMajor,gAppVerMinor,gAppVerPatch));
    #endif // BuildDebug

    if (!RegisterWindow("WINMAIN", mainTitle, gMainWindowX,gMainWindowY,gMainWindowBaseW,gMainWindowBaseH,gMainWindowMinW,gMainWindowMinH,gMainWindowFlags))
    {
        LogError(ErrorLevel::Max, "Failed to create the main application window!");
        return false;
    }

    GetWindowFromName("WINMAIN").closeCallback = [](){ Internal::PushQuitEvent(); };

    // This is a useful feature of SDL's that will allow us to redraw our
    // screen whilst it is being resized, which is much more aesthetically
    // appealing when compared to the usual garbage visuals on a resize.
    SDL_AddEventWatch(Internal::ResizeWindow, &gMainThreadID);

    return true;
}

TEINAPI void QuitWindow ()
{
    // Note: We remove this so when we SDL_RestoreWindow in Internal::SaveWindowState
    // we don't end up invoking the resize handler function Internal::ResizeWindow.
    SDL_DelEventWatch(Internal::ResizeWindow, &gMainThreadID);
    HideWindow("WINMAIN");
    Internal::SaveWindowState();
    for (auto [name,window]: gWindows) SDL_DestroyWindow(window.window);
    gWindows.clear();
}

TEINAPI void HandleWindowEvents ()
{
    if (gMainEvent.type != SDL_WINDOWEVENT) return;

    std::string windowName = GetWindowNameFromID(gMainEvent.window.windowID);
    Window& window = GetWindowFromID(gMainEvent.window.windowID);

    switch (gMainEvent.window.event)
    {
        case (SDL_WINDOWEVENT_FOCUS_GAINED):
        {
            // Special case for the main window to ensure all sub-windows stay on top of it.
            if (windowName == "WINMAIN" && Internal::AreAnySubWindowsOpen())
            {
                for (auto [name,unused]: gWindows)
                {
                    if (name != "WINMAIN") RaiseWindow(name);
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
            if (windowName == "WINMAIN")
            {
                for (auto [name,unused]: gWindows)
                {
                    if (name != "WINMAIN" && !IsWindowHidden(name))
                    {
                        gRestoreList.push_back(name);
                        HideWindow(name);
                    }
                }
            }
        } break;
        case (SDL_WINDOWEVENT_RESTORED):
        {
            if (windowName == "WINMAIN")
            {
                for (auto [name,unused]: gWindows)
                {
                    if (name != "WINMAIN")
                    {
                        if (std::find(gRestoreList.begin(), gRestoreList.end(), name) != gRestoreList.end())
                        {
                            ShowWindow(name);
                        }
                    }
                }
                gRestoreList.clear();
            }
        } break;

        case (SDL_WINDOWEVENT_CLOSE):
        {
            if (window.closeCallback) window.closeCallback();
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

TEINAPI void SetMainWindowSubtitle (std::string subtitle)
{
    #if defined(BuildDebug)
    std::string mainTitle(FormatString("[DEBUG] %s (%d.%d.%d)", gMainWindowTitle, gAppVerMajor,gAppVerMinor,gAppVerPatch));
    #else
    std::string mainTitle(FormatString("%s (%d.%d.%d)", gMainWindowTitle, gAppVerMajor,gAppVerMinor,gAppVerPatch));
    #endif // BuildDebug

    if (!subtitle.empty())
    {
        mainTitle += " | ";
        mainTitle += subtitle;
    }

    SDL_SetWindowTitle(gWindows.at("WINMAIN").window, mainTitle.c_str());
}

TEINAPI void ShowMainWindow ()
{
    Internal::LoadWindowState();
    SDL_ShowWindow(gWindows.at("WINMAIN").window);
}

TEINAPI Window& GetFocusedWindow ()
{
    for (auto& [name,window]: gWindows) if (window.focus) return window;
    return gWindows.at("WINMAIN");
}

TEINAPI Window& GetWindowFromName (std::string name)
{
    assert(gWindows.find(name) != gWindows.end());
    return gWindows.at(name);
}
TEINAPI Window& GetWindowFromID (WindowID id)
{
    for (auto& [name,window]: gWindows) if (window.id == id) return window;
    return gWindows.at("WINMAIN");
}

TEINAPI WindowID GetWindowIDFromName (std::string name)
{
    return gWindows.at(name).id;
}
TEINAPI std::string GetWindowNameFromID (WindowID id)
{
    for (auto& [name,window]: gWindows) if (window.id == id) return name;
    return std::string();
}

TEINAPI bool IsAWindowResizing ()
{
    return gWindowResizing;
}

//
// INDIVIDUAL WINDOW FUNCTIONALITY
//

TEINAPI bool RegisterWindow (std::string name, std::string title, int x, int y, int w, int h, int minW, int minH, U32 flags)
{
    if (gWindows.find(name) != gWindows.end())
    {
        LogError(ErrorLevel::Max, "Window with name \"%s\" already exists!", name.c_str());
        return false;
    }

    gWindows.insert({ name, Window() });
    Window& window = gWindows.at(name);

    // These are the required flags for all of our application sub-windows.
    flags |= (SDL_WINDOW_HIDDEN|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_OPENGL);
    window.window = SDL_CreateWindow(title.c_str(), x, y, w, h, flags);
    if (!window.window)
    {
        LogError(ErrorLevel::Min, "Failed to create window! (%s)", SDL_GetError());
        return false;
    }

    window.id = SDL_GetWindowID(window.window);
    if (!window.id)
    {
        LogError(ErrorLevel::Min, "Failed to get window ID! (%s)", SDL_GetError());
        return false;
    }

    // Only set a minimum size if both values are a valid size.
    if (minW && minH) SetWindowMinSize(name, minW, minH);

    // We use SDL_GetWindowSize to cache the final size of a window in case
    // any flags such as SDL_WINDOW_MAXIMIZED were used on window creation.
    int finalWidth;
    int finalHeight;

    SDL_GetWindowSize(window.window, &finalWidth, &finalHeight);

    window.width = static_cast<float>(finalWidth);
    window.height = static_cast<float>(finalHeight);

    // Default to false and it will get handled by the window event system.
    window.focus = false;
    window.mouse = false;

    LogDebug("Created Window %s", name.c_str());
    return true;
}

TEINAPI bool IsWindowHidden (std::string name)
{
    return (SDL_GetWindowFlags(gWindows.at(name).window) & SDL_WINDOW_HIDDEN);
}
TEINAPI bool IsWindowFocused (std::string name)
{
    return gWindows.at(name).focus;
}

TEINAPI void ShowWindow (std::string name)
{
    SetWindowPos(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(gWindows.at(name).window);
}
TEINAPI void RaiseWindow (std::string name)
{
    SDL_RaiseWindow(gWindows.at(name).window);
}
TEINAPI void HideWindow (std::string name)
{
    SDL_HideWindow(gWindows.at(name).window);
}

TEINAPI void SetWindowTitle (std::string name, std::string title)
{
    SDL_SetWindowTitle(gWindows.at(name).window, title.c_str());
}

TEINAPI void SetWindowMinSize (std::string name, int w, int h)
{
    SDL_SetWindowMinimumSize(gWindows.at(name).window, w, h);
}
TEINAPI void SetWindowMaxSize (std::string name, int w, int h)
{
    SDL_SetWindowMaximumSize(gWindows.at(name).window, w, h);
}

TEINAPI void SetWindowPos (std::string name, int x, int y)
{
    SDL_SetWindowPosition(gWindows.at(name).window, x, y);
}
TEINAPI void SetWindowSize (std::string name, int w, int h)
{
    SDL_SetWindowSize(gWindows.at(name).window, w, h);
}

#if defined(PLATFORM_WIN32)
TEINAPI void SetWindowChild (std::string name)
{
    HWND hwnd = Internal::Win32GetWindowHandle(GetWindowFromName(name).window);
    LONG old = GetWindowLongA(hwnd, GWL_EXSTYLE);
    SetWindowLongA(hwnd, GWL_EXSTYLE, old|WS_EX_TOOLWINDOW);
}
#endif // PLATFORM_WIN32
