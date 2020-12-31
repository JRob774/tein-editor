namespace Internal
{
    TEINAPI void DumpDebugApplicationInfo ()
    {
        int numDisplayModes = SDL_GetNumVideoDisplays();
        int numVideoDrivers = SDL_GetNumVideoDrivers();

        LogDebug("Platform: %s", SDL_GetPlatform());
        if (numDisplayModes > 0)
        {
            BeginDebugSection("Displays:");
            for (int i=0; i<numDisplayModes; ++i)
            {
                SDL_DisplayMode displayMode = {};
                if (SDL_GetCurrentDisplayMode(i, &displayMode) == 0)
                {
                    const char* name = SDL_GetDisplayName(i);
                    int w = displayMode.w;
                    int h = displayMode.h;
                    int hz = displayMode.refresh_rate;
                    LogDebug("(%d) %s %dx%d %dHz", i, name, w,h, hz);
                }
            }
            EndDebugSection();
        }
        if (numVideoDrivers > 0)
        {
            BeginDebugSection("Drivers:");
            for (int i=0; i<numVideoDrivers; ++i)
            {
                const char* name = SDL_GetVideoDriver(i);
                LogDebug("(%d) %s", i, name);
            }
            EndDebugSection();
        }
    }
}

TEINAPI void InitApplication (int argc, char** argv)
{
    BeginDebugTimer("InitApplication");

    // We set this here at program start so any fatal calls to LogError can
    // set this to false and we will never enter the main application loop.
    gMainRunning = true;

    GetResourceLocation();

    BeginDebugSection("Editor:");
    LogDebug("Version %d.%d.%d", gAppVerMajor,gAppVerMinor,gAppVerPatch);
    #if defined(BUILD_DEBUG)
    LogDebug("Build: Debug");
    #else
    LogDebug("Build: Release");
    #endif // BUILD_DEBUG
    EndDebugSection();

    BeginDebugSection("Initialization:");

    if (!InitErrorSystem())
    {
        LogError(ERR_MAX, "Failed to setup the error system!");
        return;
    }

    U32 sdlFLags = SDL_INIT_VIDEO|SDL_INIT_TIMER;
    if (SDL_Init(sdlFLags) != 0)
    {
        LogError(ERR_MAX, "Failed to initialize SDL! (%s)", SDL_GetError());
        return;
    }
    else LogDebug("Initialized SDL2 Library");

    if (FT_Init_FreeType(&gFreetype) != 0)
    {
        LogError(ERR_MAX, "Failed to initialize FreeType!");
        return;
    }
    else LogDebug("Initialized FreeType2 Library");

    Internal::DumpDebugApplicationInfo();

    if (!InitResourceManager())
    {
        LogError(ERR_MAX, "Failed to setup the resource manager!");
        return;
    }
    if (!InitUiSystem())
    {
        LogError(ERR_MAX, "Failed to setup the UI system!");
        return;
    }
    if (!InitWindow())
    {
        LogError(ERR_MAX, "Failed to setup the window system!");
        return;
    }

    if (!RegisterWindow("WINPREFERENCES", "Preferences", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 570,480, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create preferences window!"); return; }
    if (!RegisterWindow("WINCOLOR", "Color Picker", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 250,302, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create color picker window!"); return; }
    if (!RegisterWindow("WINNEW", "New", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 230,126, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create new window!"); return; }
    if (!RegisterWindow("WINRESIZE", "Resize", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 230,200, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create resize window!"); return; }
    if (!RegisterWindow("WINABOUT", "About", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 440,96, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create about window!"); return; }
    if (!RegisterWindow("WINUNPACK", "Unpack", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 360,80, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create GPAK unpack window!"); return; }
    if (!RegisterWindow("WINPACK", "Pack", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 360,80, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create GPAK unpack window!"); return; }
    if (!RegisterWindow("WINPATH", "Locate Game", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 440,100, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create path window!"); return; }

    GetWindowFromName("WINPREFERENCES").closeCallback = [](){ CancelPreferences(); };
    GetWindowFromName("WINCOLOR").closeCallback = [](){ CancelColorPicker(); };
    GetWindowFromName("WINNEW").closeCallback = [](){ CancelNew(); };
    GetWindowFromName("WINRESIZE").closeCallback = [](){ CancelResize(); };
    GetWindowFromName("WINABOUT").closeCallback = [](){ HideWindow("WINABOUT"); };
    GetWindowFromName("WINUNPACK").closeCallback = [](){ CancelUnpack(); };
    GetWindowFromName("WINPACK").closeCallback = [](){ CancelPack(); };
    GetWindowFromName("WINPATH").closeCallback = [](){ CancelPath(); };
    GetWindowFromName("WINMAIN").resizeCallback = [](){ DoApplication(); };

    SetWindowChild("WINPREFERENCES");
    SetWindowChild("WINCOLOR");
    SetWindowChild("WINNEW");
    SetWindowChild("WINRESIZE");
    SetWindowChild("WINABOUT");
    SetWindowChild("WINUNPACK");
    SetWindowChild("WINPACK");
    SetWindowChild("WINPATH");

    if (!InitRenderer())
    {
        LogError(ERR_MAX, "Failed to setup the renderer!");
        return;
    }
    if (!LoadEditorSettings ())
    {
        LogError(ERR_MED, "Failed to load editor settings!");
    }
    if (!LoadEditorKeyBindings())
    {
        LogError(ERR_MED, "Failed to load editor key bindings!");
    }
    if (!LoadEditorResources())
    {
        LogError(ERR_MAX, "Failed to load editor resources!");
        return;
    }
    if (!InitTilePanel())
    {
        LogError(ERR_MAX, "Failed to setup the tile panel!");
        return;
    }

    InitLayerPanel();
    InitColorPicker();
    InitPaletteLookup();

    InitEditor(argc, argv);

    // Now that setup has been complete we can show the complete window.
    // See the 'window.hpp' file for why we initially hide the window.
    //
    // We don't bother showing if any of the setup functions resulted in
    // a fatal failure as it would look ugly to briefly flash the window.
    if (gMainRunning) ShowMainWindow();

    // We do this so we do an extra redraw on start-up making sure certain
    // things end up being initialized/setup. This fixes the scrollbars
    // appearing in the control panel sub-panels when they are not needed.
    gShouldPushUiRedrawEvent = true;

    EndDebugSection();

    EndDebugTimer();
    DumpDebugTimerResult();
}

TEINAPI void QuitApplication ()
{
    LogDebug("QuitApplication");

    QuitEditor();

    FreeEditorCursors();
    FreeEditorResources();

    QuitRenderer();
    QuitWindow();

    QuitDebugSystem();
    QuitErrorSystem();

    FT_Done_FreeType(gFreetype);
    SDL_Quit();
}

TEINAPI void DoApplication ()
{
    ClearDebugTimerResult();
    Defer { DumpDebugTimerResult(); };

    SetRenderTarget(&GetWindowFromName("WINMAIN"));
    SetViewport(0, 0, GetRenderTargetWidth(), GetRenderTargetHeight());

    RenderClear(gUiColorMedium);

    Quad p1, p2;

    p1.x = gWindowBorder;
    p1.y = gWindowBorder;
    p1.w = GetViewport().w - (gWindowBorder * 2);
    p1.h = GetViewport().h - (gWindowBorder * 2);

    BeginPanel(p1, UI_NONE, gUiColorExDark);

    DoHotbar();

    p2.x = 1;
    p2.y = gHotbarHeight + 1;
    p2.w = GetViewport().w - 2;
    p2.h = GetViewport().h - p2.y - 1;

    BeginPanel(p2, UI_NONE);

    DoControlPanel();
    DoToolbar();
    DoTabBar();
    DoEditor();
    DoStatusBar();

    EndPanel();
    EndPanel();

    DoTooltip();

    RenderPresent();

    if (!IsWindowHidden("WINPREFERENCES"))
    {
        SetRenderTarget(&GetWindowFromName("WINPREFERENCES"));
        SetViewport(0, 0, GetRenderTargetWidth(), GetRenderTargetHeight());
        RenderClear(gUiColorMedium);
        DoPreferencesMenu();
        RenderPresent();
    }

    if (!IsWindowHidden("WINCOLOR"))
    {
        SetRenderTarget(&GetWindowFromName("WINCOLOR"));
        SetViewport(0, 0, GetRenderTargetWidth(), GetRenderTargetHeight());
        RenderClear(gUiColorMedium);
        DoColorPicker();
        RenderPresent();
    }

    if (!IsWindowHidden("WINABOUT"))
    {
        SetRenderTarget(&GetWindowFromName("WINABOUT"));
        SetViewport(0, 0, GetRenderTargetWidth(), GetRenderTargetHeight());
        RenderClear(gUiColorMedium);
        DoAbout();
        RenderPresent();
    }

    if (!IsWindowHidden("WINNEW"))
    {
        SetRenderTarget(&GetWindowFromName("WINNEW"));
        SetViewport(0, 0, GetRenderTargetWidth(), GetRenderTargetHeight());
        RenderClear(gUiColorMedium);
        DoNew();
        RenderPresent();
    }

    if (!IsWindowHidden("WINRESIZE"))
    {
        SetRenderTarget(&GetWindowFromName("WINRESIZE"));
        SetViewport(0, 0, GetRenderTargetWidth(), GetRenderTargetHeight());
        RenderClear(gUiColorMedium);
        DoResize();
        RenderPresent();
    }

    if (!IsWindowHidden("WINUNPACK"))
    {
        SetRenderTarget(&GetWindowFromName("WINUNPACK"));
        SetViewport(0, 0, GetRenderTargetWidth(), GetRenderTargetHeight());
        RenderClear(gUiColorMedium);
        DoUnpack();
        RenderPresent();
    }

    if (!IsWindowHidden("WINPACK"))
    {
        SetRenderTarget(&GetWindowFromName("WINPACK"));
        SetViewport(0, 0, GetRenderTargetWidth(), GetRenderTargetHeight());
        RenderClear(gUiColorMedium);
        DoPack();
        RenderPresent();
    }

    if (!IsWindowHidden("WINPATH"))
    {
        SetRenderTarget(&GetWindowFromName("WINPATH"));
        SetViewport(0, 0, GetRenderTargetWidth(), GetRenderTargetHeight());
        RenderClear(gUiColorMedium);
        DoPath();
        RenderPresent();
    }

    // IMPORTANT: Otherwise the UI will not redraw very well!
    if (gShouldPushUiRedrawEvent)
    {
        PushEditorEvent(EDITOR_EVENT_UI_REDRAW, NULL, NULL);
        gShouldPushUiRedrawEvent = false;
    }
}

TEINAPI bool HandleApplicationEvents ()
{
    // We wait for events so we don't waste CPU and GPU power.
    if (!SDL_WaitEvent(&gMainEvent))
    {
        LogError(ERR_MED, "Error waiting for events! (%s)", SDL_GetError());
        return false;
    }

    ResetUiState();

    // We need to poll events afterwards so that we can process
    // multiple events that may have occurred on the same frame.
    do
    {
        if (gMainEvent.type == SDL_QUIT) gMainRunning = false;

        #if defined(BUILD_DEBUG)
        generate_texture_atlases();
        pack_textures();
        #endif // BUILD_DEBUG

        HandleWindowEvents();
        HandleKeyBindingEvents();
        HandleUiEvents();
        HandleTilePanelEvents();
        HandleTabBarEvents();
        HandleEditorEvents();
        HandlePreferencesMenuEvents();
        HandleColorPickerEvents();
        HandleNewEvents();
        HandleResizeEvents();
        HandleTooltipEvents();
        HandleAboutEvents();
        HandlePathEvents();
    }
    while (SDL_PollEvent(&gMainEvent));

    return true;
}
