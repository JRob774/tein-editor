/*******************************************************************************
 * Cross-platform entry-point for the editor application.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

#include <compiler.h>
#include <platform.h>
#include <standard.h>

/* -------------------------------------------------------------------------- */

#if defined(PLATFORM_WIN32) /**************************************************/

#include <windows.h>
#include <dbghelp.h>
#include <shlwapi.h>
#include <shobjidl_core.h>

#endif /* PLATFORM_WIN32 ******************************************************/

/* -------------------------------------------------------------------------- */

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstring>
#include <cstdarg>

#include <type_traits>
#include <algorithm>
#include <exception>
#include <atomic>
#include <fstream>
#include <sstream>

#include <vector>
#include <array>
#include <map>
#include <deque>
#include <string>

/* -------------------------------------------------------------------------- */

#include <external.hpp>
#include <version.hpp>

#define OPENGL_LOAD_GL_VERSION_MAJOR 3
#define OPENGL_LOAD_GL_VERSION_MINOR 0

#define OPENGL_USE_COMPATIBILITY_PROFILE

#include <opengl.hpp>

#include <utility.hpp>
#include <alert.hpp>
#include <debug.hpp>
#include <error.hpp>
#include <dialog.hpp>
#include <custom_events.hpp>
#include <window.hpp>
#include <cursor.hpp>
#include <key_bindings.hpp>
#include <settings.hpp>
#include <vertex_buffer.hpp>
#include <renderer.hpp>
#include <texture.hpp>
#include <texture_atlas.hpp>
#include <font.hpp>
#include <shader.hpp>
#include <resource_manager.hpp>
#include <user_interface.hpp>
#include <level.hpp>
#include <map.hpp>
#include <gpak.hpp>
#include <hotbar.hpp>
#include <toolbar.hpp>
#include <tooltip.hpp>
#include <tile_panel.hpp>
#include <layer_panel.hpp>
#include <control_panel.hpp>
#include <new_dialog.hpp>
#include <resize_dialog.hpp>
#include <path_dialog.hpp>
#include <tab_bar.hpp>
#include <palette.hpp>
#include <level_editor.hpp>
#include <map_editor.hpp>
#include <editor.hpp>
#include <status_bar.hpp>
#include <color_picker.hpp>
#include <preferences_menu.hpp>
#include <about.hpp>
#include <update.hpp>
#include <application.hpp>

/* -------------------------------------------------------------------------- */

#include <utility.cpp>
#include <alert.cpp>
#include <debug.cpp>
#include <error.cpp>
#include <dialog.cpp>
#include <custom_events.cpp>
#include <window.cpp>
#include <cursor.cpp>
#include <key_bindings.cpp>
#include <settings.cpp>
#include <vertex_buffer.cpp>
#include <renderer.cpp>
#include <texture.cpp>
#include <texture_atlas.cpp>
#include <font.cpp>
#include <shader.cpp>
#include <resource_manager.cpp>
#include <user_interface.cpp>
#include <level.cpp>
#include <map.cpp>
#include <gpak.cpp>
#include <hotbar.cpp>
#include <toolbar.cpp>
#include <tooltip.cpp>
#include <tile_panel.cpp>
#include <layer_panel.cpp>
#include <control_panel.cpp>
#include <new_dialog.cpp>
#include <resize_dialog.cpp>
#include <path_dialog.cpp>
#include <tab_bar.cpp>
#include <palette.cpp>
#include <level_editor.cpp>
#include <map_editor.cpp>
#include <editor.cpp>
#include <status_bar.cpp>
#include <color_picker.cpp>
#include <preferences_menu.cpp>
#include <about.cpp>
#include <update.cpp>

/* -------------------------------------------------------------------------- */

#if defined(BUILD_DEBUG) /*****************************************************/

#include <utility/generate_atlas.hpp>
#include <utility/pack_texture.hpp>

#endif /* BUILD_DEBUG *********************************************************/

/* -------------------------------------------------------------------------- */

#include <application.cpp>

/* -------------------------------------------------------------------------- */

int main (int argc, char** argv)
{
    error_terminate_callback = quit_application;
    error_maximum_callback = save_restore_files;

    // We defer so that this always gets scalled on scope exit no matter what.
    defer { quit_application(); };

    init_application(argc, argv);
    while (main_running && handle_application_events())
    {
        do_application();
    }

    return 0;
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