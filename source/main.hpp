#if defined(PLATFORM_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <objbase.h>
#endif // PLATFORM_WIN32

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstdarg>
#include <cassert>

#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <stack>

#include <SDL2/SDL.h>

#include <GON.hpp>
#include <GON.cpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>

#include <glad/glad.h>
#include <glad/glad.c>

#include <glm/glm.hpp>

// NOTE: This header goes up here so that we can use EditorAPI just below this include.
#include "Utility.hpp"

#ifndef IM_ASSERT_USER_ERROR
EditorAPI void LogSingleSystemMessage (std::string system, const char* format, ...); // Predeclare for ImGui!
#define IM_ASSERT_USER_ERROR(exp,msg) do { if (!(exp)) LogSingleSystemMessage("imgui", (msg)); } while (0)
#endif
#include <imconfig.h>
#include <imgui.h>
#include <imgui.cpp>
#include <imgui_demo.cpp>
#include <imgui_widgets.cpp>
#include <imgui_draw.cpp>
#include <imgui_tables.cpp>
#include <imgui_stdlib.h>
#include <imgui_stdlib.cpp>
#include <backend/imgui_impl_sdl.h>
#include <backend/imgui_impl_sdl.cpp>
#include <backend/imgui_impl_opengl3.h>
#include <backend/imgui_impl_opengl3.cpp>

// Header Includes
#include "logger.hpp"
#include "window.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertex_buffer.hpp"
#include "asset_manager.hpp"
#include "user_interface.hpp"
#include "settings.hpp"
#include "hotbar.hpp"
#include "tiles_widget.hpp"
#include "tools_widget.hpp"
#include "application.hpp"

// Source Includes
#include "utility.cpp"
#include "logger.cpp"
#include "window.cpp"
#include "shader.cpp"
#include "texture.cpp"
#include "vertex_buffer.cpp"
#include "asset_manager.cpp"
#include "user_interface.cpp"
#include "settings.cpp"
#include "hotbar.cpp"
#include "tiles_widget.cpp"
#include "tools_widget.cpp"
#include "application.cpp"
