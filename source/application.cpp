/*******************************************************************************
 * Main application initalization/termination and main execution loop.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

TEINAPI void internal__dump_debug_application_info ()
{
    int num_display_modes = SDL_GetNumVideoDisplays();
    int num_video_drivers = SDL_GetNumVideoDrivers();

    LogDebug("Platform: %s", SDL_GetPlatform());
    if (num_display_modes > 0)
    {
        BeginDebugSection("Displays:");
        for (int i=0; i<num_display_modes; ++i)
        {
            SDL_DisplayMode display_mode = {};
            if (SDL_GetCurrentDisplayMode(i, &display_mode) == 0)
            {
                const char* name = SDL_GetDisplayName(i);
                int w            = display_mode.w;
                int h            = display_mode.h;
                int hz           = display_mode.refresh_rate;
                LogDebug("(%d) %s %dx%d %dHz", i, name, w, h, hz);
            }
        }
        EndDebugSection();
    }
    if (num_video_drivers > 0)
    {
        BeginDebugSection("Drivers:");
        for (int i=0; i<num_video_drivers; ++i)
        {
            const char* name = SDL_GetVideoDriver(i);
            LogDebug("(%d) %s", i, name);
        }
        EndDebugSection();
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void init_application (int argc, char** argv)
{
    BeginDebugTimer("init_application");

    // We set this here at program start so any fatal calls to LogError can
    // set this to false and we will never enter the main application loop.
    main_running = true;

    get_resource_location();

    BeginDebugSection("Editor:");
    LogDebug("Version %d.%d.%d", APP_VER_MAJOR,APP_VER_MINOR,APP_VER_PATCH);
    #if defined(BUILD_DEBUG)
    LogDebug("Build: Debug");
    #else
    LogDebug("Build: Release");
    #endif
    #if defined(ARCHITECTURE_32BIT)
    LogDebug("Architecture: x86");
    #endif
    #if defined(ARCHITECTURE_64BIT)
    LogDebug("Architecture: x64");
    #endif
    EndDebugSection();

    BeginDebugSection("Initialization:");

    if (!InitErrorSystem())
    {
        LogError(ERR_MAX, "Failed to setup the error system!");
        return;
    }

    U32 sdl_flags = SDL_INIT_VIDEO|SDL_INIT_TIMER;
    if (SDL_Init(sdl_flags) != 0)
    {
        LogError(ERR_MAX, "Failed to initialize SDL! (%s)", SDL_GetError());
        return;
    }
    else LogDebug("Initialized SDL2 Library");

    if (FT_Init_FreeType(&freetype) != 0)
    {
        LogError(ERR_MAX, "Failed to initialize FreeType!");
        return;
    }
    else LogDebug("Initialized FreeType2 Library");

    internal__dump_debug_application_info();

    if (!init_resource_manager()) { LogError(ERR_MAX, "Failed to setup the resource manager!"); return; }
    if (!init_ui_system       ()) { LogError(ERR_MAX, "Failed to setup the UI system!"       ); return; }
    if (!InitWindow           ()) { LogError(ERR_MAX, "Failed to setup the window system!"   ); return; }

    if (!RegisterWindow("WINPREFERENCES", "Preferences"     , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 570,480, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create preferences window!" ); return; }
    if (!RegisterWindow("WINCOLOR"      , "Color Picker"    , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 250,302, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create color picker window!"); return; }
    if (!RegisterWindow("WINNEW"        , "New"             , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 230,126, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create new window!"         ); return; }
    if (!RegisterWindow("WINRESIZE"     , "Resize"          , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 230,200, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create resize window!"      ); return; }
    if (!RegisterWindow("WINABOUT"      , "About"           , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 440, 96, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create about window!"       ); return; }
    if (!RegisterWindow("WINUNPACK"     , "Unpack"          , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 360, 80, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create GPAK unpack window!" ); return; }
    if (!RegisterWindow("WINPACK"       , "Pack"            , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 360, 80, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create GPAK unpack window!" ); return; }
    if (!RegisterWindow("WINPATH"       , "Locate Game"     , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 440,100, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create path window!"        ); return; }
    if (!RegisterWindow("WINUPDATE"     , "Update Available", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 370,440, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LogError(ERR_MAX, "Failed to create update window!"      ); return; }

    GetWindowFromName("WINPREFERENCES"). closeCallback = []() { cancel_preferences   (); };
    GetWindowFromName("WINCOLOR"      ). closeCallback = []() { cancel_color_picker  (); };
    GetWindowFromName("WINNEW"        ). closeCallback = []() { cancel_new           (); };
    GetWindowFromName("WINRESIZE"     ). closeCallback = []() { cancel_resize        (); };
    GetWindowFromName("WINABOUT"      ). closeCallback = []() { HideWindow("WINABOUT" ); };
    GetWindowFromName("WINUNPACK"     ). closeCallback = []() { cancel_unpack        (); };
    GetWindowFromName("WINPACK"       ). closeCallback = []() { cancel_pack          (); };
    GetWindowFromName("WINPATH"       ). closeCallback = []() { cancel_path          (); };
    GetWindowFromName("WINUPDATE"     ). closeCallback = []() { HideWindow("WINUPDATE"); };
    GetWindowFromName("WINMAIN"       ).resizeCallback = []() { do_application       (); };

    SetWindowChild("WINPREFERENCES");
    SetWindowChild("WINCOLOR");
    SetWindowChild("WINNEW");
    SetWindowChild("WINRESIZE");
    SetWindowChild("WINABOUT");
    SetWindowChild("WINUNPACK");
    SetWindowChild("WINPACK");
    SetWindowChild("WINPATH");
    SetWindowChild("WINUPDATE");

    if (!init_renderer           ()) { LogError(ERR_MAX, "Failed to setup the renderer!"      ); return; }
    if (!load_editor_settings    ()) { LogError(ERR_MED, "Failed to load editor settings!"    );         }
    if (!load_editor_key_bindings()) { LogError(ERR_MED, "Failed to load editor key bindings!");         }
    if (!load_editor_resources   ()) { LogError(ERR_MAX, "Failed to load editor resources!"   ); return; }
    if (!init_tile_panel         ()) { LogError(ERR_MAX, "Failed to setup the tile panel!"    ); return; }

    init_layer_panel   ();
    init_color_picker  ();
    init_palette_lookup();

    init_editor(argc, argv);

    check_for_updates();

    // Now that setup has been complete we can show the complete window.
    // See the 'window.hpp' file for why we initially hide the window.
    //
    // We don't bother showing if any of the setup functions resulted in
    // a fatal failure as it would look ugly to briefly flash the window.
    if (main_running) ShowMainWindow();

    // We do this so we do an extra redraw on start-up making sure certain
    // things end up being initialized/setup. This fixes the scrollbars
    // appearing in the control panel sub-panels when they are not needed.
    should_push_ui_redraw_event = true;

    // We don't do this in our debug builds because it will get annoying.
    #if !defined(BUILD_DEBUG)
    if (are_there_updates()) open_update_window_timed();
    #endif // !BUILD_DEBUG

    EndDebugSection();

    EndDebugTimer();
    DumpDebugTimerResult();
}

TEINAPI void quit_application ()
{
    LogDebug("quit_application()");

    quit_editor();

    free_editor_cursors();
    free_editor_resources();

    quit_renderer();
    QuitWindow();

    QuitDebugSystem();
    QuitErrorSystem();

    FT_Done_FreeType(freetype);
    SDL_Quit();
}

/* -------------------------------------------------------------------------- */

TEINAPI void do_application ()
{
    ClearDebugTimerResult();
    Defer { DumpDebugTimerResult(); };

    set_render_target(&GetWindowFromName("WINMAIN"));
    set_viewport(0, 0, get_render_target_w(), get_render_target_h());

    render_clear(ui_color_medium);

    Quad p1, p2;

    p1.x = gWindowBorder;
    p1.y = gWindowBorder;
    p1.w = get_viewport().w - (gWindowBorder * 2);
    p1.h = get_viewport().h - (gWindowBorder * 2);

    begin_panel(p1, UI_NONE, ui_color_ex_dark);

    do_hotbar();

    p2.x =                           1;
    p2.y = HOTBAR_HEIGHT           + 1;
    p2.w = get_viewport().w        - 2;
    p2.h = get_viewport().h - p2.y - 1;

    begin_panel(p2, UI_NONE);

    do_control_panel();
    do_toolbar();
    do_tab_bar();
    do_editor();
    do_status_bar();

    end_panel();
    end_panel();

    do_tooltip();

    render_present();

    if (!IsWindowHidden("WINPREFERENCES"))
    {
        set_render_target(&GetWindowFromName("WINPREFERENCES"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_preferences_menu();
        render_present();
    }

    if (!IsWindowHidden("WINCOLOR"))
    {
        set_render_target(&GetWindowFromName("WINCOLOR"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_color_picker();
        render_present();
    }

    if (!IsWindowHidden("WINABOUT"))
    {
        set_render_target(&GetWindowFromName("WINABOUT"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_about();
        render_present();
    }

    if (!IsWindowHidden("WINNEW"))
    {
        set_render_target(&GetWindowFromName("WINNEW"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_new();
        render_present();
    }

    if (!IsWindowHidden("WINRESIZE"))
    {
        set_render_target(&GetWindowFromName("WINRESIZE"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_resize();
        render_present();
    }

    if (!IsWindowHidden("WINUNPACK"))
    {
        set_render_target(&GetWindowFromName("WINUNPACK"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_unpack();
        render_present();
    }

    if (!IsWindowHidden("WINPACK"))
    {
        set_render_target(&GetWindowFromName("WINPACK"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_pack();
        render_present();
    }

    if (!IsWindowHidden("WINPATH"))
    {
        set_render_target(&GetWindowFromName("WINPATH"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_path();
        render_present();
    }

    if (!IsWindowHidden("WINUPDATE"))
    {
        set_render_target(&GetWindowFromName("WINUPDATE"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_update();
        render_present();
    }

    // IMPORTANT: Otherwise the UI will not redraw very well!
    if (should_push_ui_redraw_event)
    {
        PushEditorEvent(EDITOR_EVENT_UI_REDRAW, NULL, NULL);
        should_push_ui_redraw_event = false;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI bool handle_application_events ()
{
    // We wait for events so we don't waste CPU and GPU power.
    if (!SDL_WaitEvent(&main_event))
    {
        LogError(ERR_MED, "Error waiting for events! (%s)", SDL_GetError());
        return false;
    }

    reset_ui_state();

    // We need to poll events afterwards so that we can process
    // multiple events that may have occurred on the same frame.
    do
    {
        if (main_event.type == SDL_QUIT)
        {
            main_running = false;
        }

        #if defined(BUILD_DEBUG)
        generate_texture_atlases();
        pack_textures();
        #endif

        HandleWindowEvents();
        handle_key_binding_events();
        handle_ui_events();
        handle_tile_panel_events();
        handle_tab_bar_events();
        handle_editor_events();
        handle_preferences_menu_events();
        handle_color_picker_events();
        handle_new_events();
        handle_resize_events();
        handle_tooltip_events();
        handle_about_events();
        handle_path_events();
        handle_update_events();
    }
    while (SDL_PollEvent(&main_event));

    return true;
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
