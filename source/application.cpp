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

    LOG_DEBUG("Platform: %s", SDL_GetPlatform());
    if (num_display_modes > 0)
    {
        begin_debug_section("Displays:");
        for (int i=0; i<num_display_modes; ++i)
        {
            SDL_DisplayMode display_mode = {};
            if (SDL_GetCurrentDisplayMode(i, &display_mode) == 0)
            {
                const char* name = SDL_GetDisplayName(i);
                int w            = display_mode.w;
                int h            = display_mode.h;
                int hz           = display_mode.refresh_rate;
                LOG_DEBUG("(%d) %s %dx%d %dHz", i, name, w, h, hz);
            }
        }
        end_debug_section();
    }
    if (num_video_drivers > 0)
    {
        begin_debug_section("Drivers:");
        for (int i=0; i<num_video_drivers; ++i)
        {
            const char* name = SDL_GetVideoDriver(i);
            LOG_DEBUG("(%d) %s", i, name);
        }
        end_debug_section();
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void init_application (int argc, char** argv)
{
    begin_debug_timer("init_application");

    // We set this here at program start so any fatal calls to LOG_ERROR can
    // set this to false and we will never enter the main application loop.
    main_running = true;

    get_resource_location();

    begin_debug_section("Editor:");
    LOG_DEBUG("Version %d.%d.%d", APP_VER_MAJOR,APP_VER_MINOR,APP_VER_PATCH);
    #if defined(BUILD_DEBUG)
    LOG_DEBUG("Build: Debug");
    #else
    LOG_DEBUG("Build: Release");
    #endif
    #if defined(ARCHITECTURE_32BIT)
    LOG_DEBUG("Architecture: x86");
    #endif
    #if defined(ARCHITECTURE_64BIT)
    LOG_DEBUG("Architecture: x64");
    #endif
    end_debug_section();

    begin_debug_section("Initialization:");

    if (!init_error_system())
    {
        LOG_ERROR(ERR_MAX, "Failed to setup the error system!");
        return;
    }

    U32 sdl_flags = SDL_INIT_VIDEO|SDL_INIT_TIMER;
    if (SDL_Init(sdl_flags) != 0)
    {
        LOG_ERROR(ERR_MAX, "Failed to initialize SDL! (%s)", SDL_GetError());
        return;
    }
    else LOG_DEBUG("Initialized SDL2 Library");

    if (FT_Init_FreeType(&freetype) != 0)
    {
        LOG_ERROR(ERR_MAX, "Failed to initialize FreeType!");
        return;
    }
    else LOG_DEBUG("Initialized FreeType2 Library");

    internal__dump_debug_application_info();

    if (!init_resource_manager()) { LOG_ERROR(ERR_MAX, "Failed to setup the resource manager!"); return; }
    if (!init_ui_system       ()) { LOG_ERROR(ERR_MAX, "Failed to setup the UI system!"       ); return; }
    if (!init_window          ()) { LOG_ERROR(ERR_MAX, "Failed to setup the window system!"   ); return; }

    if (!create_window("WINPREFERENCES", "Preferences"     , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 570,480, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LOG_ERROR(ERR_MAX, "Failed to create preferences window!" ); return; }
    if (!create_window("WINCOLOR"      , "Color Picker"    , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 250,302, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LOG_ERROR(ERR_MAX, "Failed to create color picker window!"); return; }
    if (!create_window("WINNEW"        , "New"             , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 230,126, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LOG_ERROR(ERR_MAX, "Failed to create new window!"         ); return; }
    if (!create_window("WINRESIZE"     , "Resize"          , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 230,200, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LOG_ERROR(ERR_MAX, "Failed to create resize window!"      ); return; }
    if (!create_window("WINABOUT"      , "About"           , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 440, 96, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LOG_ERROR(ERR_MAX, "Failed to create about window!"       ); return; }
    if (!create_window("WINUNPACK"     , "Unpack"          , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 360, 80, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LOG_ERROR(ERR_MAX, "Failed to create GPAK unpack window!" ); return; }
    if (!create_window("WINPACK"       , "Pack"            , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 360, 80, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LOG_ERROR(ERR_MAX, "Failed to create GPAK unpack window!" ); return; }
    if (!create_window("WINPATH"       , "Locate Game"     , SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 440,100, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LOG_ERROR(ERR_MAX, "Failed to create path window!"        ); return; }
    if (!create_window("WINUPDATE"     , "Update Available", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 370,440, 0,0, SDL_WINDOW_SKIP_TASKBAR)) { LOG_ERROR(ERR_MAX, "Failed to create update window!"      ); return; }

    get_window("WINPREFERENCES"). close_callback = []() { cancel_preferences    (); };
    get_window("WINCOLOR"      ). close_callback = []() { cancel_color_picker   (); };
    get_window("WINNEW"        ). close_callback = []() { cancel_new            (); };
    get_window("WINRESIZE"     ). close_callback = []() { cancel_resize         (); };
    get_window("WINABOUT"      ). close_callback = []() { hide_window("WINABOUT" ); };
    get_window("WINUNPACK"     ). close_callback = []() { cancel_unpack         (); };
    get_window("WINPACK"       ). close_callback = []() { cancel_pack           (); };
    get_window("WINPATH"       ). close_callback = []() { cancel_path           (); };
    get_window("WINUPDATE"     ). close_callback = []() { hide_window("WINUPDATE"); };
    get_window("WINMAIN"       ).resize_callback = []() { do_application        (); };

    set_window_child("WINPREFERENCES");
    set_window_child("WINCOLOR");
    set_window_child("WINNEW");
    set_window_child("WINRESIZE");
    set_window_child("WINABOUT");
    set_window_child("WINUNPACK");
    set_window_child("WINPACK");
    set_window_child("WINPATH");
    set_window_child("WINUPDATE");

    if (!init_renderer           ()) { LOG_ERROR(ERR_MAX, "Failed to setup the renderer!"      ); return; }
    if (!load_editor_settings    ()) { LOG_ERROR(ERR_MED, "Failed to load editor settings!"    );         }
    if (!load_editor_key_bindings()) { LOG_ERROR(ERR_MED, "Failed to load editor key bindings!");         }
    if (!load_editor_resources   ()) { LOG_ERROR(ERR_MAX, "Failed to load editor resources!"   ); return; }
    if (!init_tile_panel         ()) { LOG_ERROR(ERR_MAX, "Failed to setup the tile panel!"    ); return; }

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
    if (main_running) show_main_window();

    // We do this so we do an extra redraw on start-up making sure certain
    // things end up being initialized/setup. This fixes the scrollbars
    // appearing in the control panel sub-panels when they are not needed.
    should_push_ui_redraw_event = true;

    // We don't do this in our debug builds because it will get annoying.
    #if !defined(BUILD_DEBUG)
    if (are_there_updates()) open_update_window_timed();
    #endif // !BUILD_DEBUG

    end_debug_section();

    end_debug_timer();
    dump_debug_timer_results();
}

TEINAPI void quit_application ()
{
    LOG_DEBUG("quit_application()");

    quit_editor();

    free_editor_cursors();
    free_editor_resources();

    quit_renderer();
    quit_window();

    quit_debug_system();
    quit_error_system();

    FT_Done_FreeType(freetype);
    SDL_Quit();
}

/* -------------------------------------------------------------------------- */

TEINAPI void do_application ()
{
    clear_debug_timer_results();
    Defer { dump_debug_timer_results(); };

    set_render_target(&get_window("WINMAIN"));
    set_viewport(0, 0, get_render_target_w(), get_render_target_h());

    render_clear(ui_color_medium);

    Quad p1, p2;

    p1.x = WINDOW_BORDER;
    p1.y = WINDOW_BORDER;
    p1.w = get_viewport().w - (WINDOW_BORDER * 2);
    p1.h = get_viewport().h - (WINDOW_BORDER * 2);

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

    if (!is_window_hidden("WINPREFERENCES"))
    {
        set_render_target(&get_window("WINPREFERENCES"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_preferences_menu();
        render_present();
    }

    if (!is_window_hidden("WINCOLOR"))
    {
        set_render_target(&get_window("WINCOLOR"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_color_picker();
        render_present();
    }

    if (!is_window_hidden("WINABOUT"))
    {
        set_render_target(&get_window("WINABOUT"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_about();
        render_present();
    }

    if (!is_window_hidden("WINNEW"))
    {
        set_render_target(&get_window("WINNEW"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_new();
        render_present();
    }

    if (!is_window_hidden("WINRESIZE"))
    {
        set_render_target(&get_window("WINRESIZE"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_resize();
        render_present();
    }

    if (!is_window_hidden("WINUNPACK"))
    {
        set_render_target(&get_window("WINUNPACK"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_unpack();
        render_present();
    }

    if (!is_window_hidden("WINPACK"))
    {
        set_render_target(&get_window("WINPACK"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_pack();
        render_present();
    }

    if (!is_window_hidden("WINPATH"))
    {
        set_render_target(&get_window("WINPATH"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_path();
        render_present();
    }

    if (!is_window_hidden("WINUPDATE"))
    {
        set_render_target(&get_window("WINUPDATE"));
        set_viewport(0, 0, get_render_target_w(), get_render_target_h());
        render_clear(ui_color_medium);
        do_update();
        render_present();
    }

    // IMPORTANT: Otherwise the UI will not redraw very well!
    if (should_push_ui_redraw_event)
    {
        push_editor_event(EDITOR_EVENT_UI_REDRAW, NULL, NULL);
        should_push_ui_redraw_event = false;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI bool handle_application_events ()
{
    // We wait for events so we don't waste CPU and GPU power.
    if (!SDL_WaitEvent(&main_event))
    {
        LOG_ERROR(ERR_MED, "Error waiting for events! (%s)", SDL_GetError());
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

        handle_window_events();
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
