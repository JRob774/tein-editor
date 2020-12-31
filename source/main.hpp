#pragma once

#define PLATFORM_WIN32 1

// We do not intend on using the secure CRT.
#if defined(_MSC_VER)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

// We do this to disable assert in release.
#if !defined(BUILD_DEBUG)
#define NDEBUG
#endif

#if defined(PLATFORM_WIN32)
#include <windows.h>
#include <dbghelp.h>
#include <shlwapi.h>
#include <shobjidl_core.h>
#endif // PLATFORM_WIN32

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cassert>
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
#include <stack>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_STATIC
#define STBRP_STATIC
#include <stb_image_write.h>
#include <stb_image.h>
#include <stb_rect_pack.h>

#define CURL_STATICLIB
#include <curl/curl.h>

#include <gon.h>
#include <gon.cpp>

#include <nlohmann/json.hpp>

#include <glad/glad.c>
#include <glad/glad.h>

#include "utility.hpp"
#include "debug.hpp"
#include "error.hpp"
#include "dialog.hpp"
#include "custom_events.hpp"
#include "window.hpp"
#include "cursor.hpp"
#include "key_bindings.hpp"
#include "settings.hpp"
#include "vertex_buffer.hpp"
#include "renderer.hpp"
#include "texture.hpp"
#include "texture_atlas.hpp"
#include "font.hpp"
#include "shader.hpp"
#include "resource_manager.hpp"
#include "user_interface.hpp"
#include "level.hpp"
#include "map.hpp"
#include "gpak.hpp"
#include "hotbar.hpp"
#include "toolbar.hpp"
#include "tooltip.hpp"
#include "tile_panel.hpp"
#include "layer_panel.hpp"
#include "control_panel.hpp"
#include "new_dialog.hpp"
#include "resize_dialog.hpp"
#include "path_dialog.hpp"
#include "tab_bar.hpp"
#include "palette.hpp"
#include "level_editor.hpp"
#include "map_editor.hpp"
#include "editor.hpp"
#include "status_bar.hpp"
#include "color_picker.hpp"
#include "preferences_menu.hpp"
#include "about.hpp"
#include "update.hpp"
#include "application.hpp"

#include "utility.cpp"
#include "debug.cpp"
#include "error.cpp"
#include "dialog.cpp"
#include "custom_events.cpp"
#include "window.cpp"
#include "cursor.cpp"
#include "key_bindings.cpp"
#include "settings.cpp"
#include "vertex_buffer.cpp"
#include "renderer.cpp"
#include "texture.cpp"
#include "texture_atlas.cpp"
#include "font.cpp"
#include "shader.cpp"
#include "resource_manager.cpp"
#include "user_interface.cpp"
#include "level.cpp"
#include "map.cpp"
#include "gpak.cpp"
#include "hotbar.cpp"
#include "toolbar.cpp"
#include "tooltip.cpp"
#include "tile_panel.cpp"
#include "layer_panel.cpp"
#include "control_panel.cpp"
#include "new_dialog.cpp"
#include "resize_dialog.cpp"
#include "path_dialog.cpp"
#include "tab_bar.cpp"
#include "palette.cpp"
#include "level_editor.cpp"
#include "map_editor.cpp"
#include "editor.cpp"
#include "status_bar.cpp"
#include "color_picker.cpp"
#include "preferences_menu.cpp"
#include "about.cpp"
#include "update.cpp"

#if defined(BUILD_DEBUG)
#include "utility/generate_atlas.hpp"
#include "utility/pack_texture.hpp"
#endif // BUILD_DEBUG

#include "application.cpp"
