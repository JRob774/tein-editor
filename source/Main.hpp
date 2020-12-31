#define PLATFORM_WIN32 1

// We do not intend on using the secure CRT.
#if defined(_MSC_VER)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

// We do this to disable assert in release.
#if !defined(BuildDebug)
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

#include <gon.h>
#include <gon.cpp>

#include <glad/glad.c>
#include <glad/glad.h>

#include "Utility.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "Dialog.hpp"
#include "CustomEvents.hpp"
#include "Window.hpp"
#include "Cursor.hpp"
#include "KeyBindings.hpp"
#include "Settings.hpp"
#include "VertexBuffer.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"
#include "TextureAtlas.hpp"
#include "Font.hpp"
#include "Shader.hpp"
#include "ResourceManager.hpp"
#include "UserInterface.hpp"
#include "Level.hpp"
#include "Map.hpp"
#include "Gpak.hpp"
#include "Hotbar.hpp"
#include "Toolbar.hpp"
#include "Tooltip.hpp"
#include "TilePanel.hpp"
#include "LayerPanel.hpp"
#include "ControlPanel.hpp"
#include "NewDialog.hpp"
#include "ResizeDialog.hpp"
#include "PathDialog.hpp"
#include "TabBar.hpp"
#include "Palette.hpp"
#include "LevelEditor.hpp"
#include "MapEditor.hpp"
#include "Editor.hpp"
#include "StatusBar.hpp"
#include "ColorPicker.hpp"
#include "PreferencesMenu.hpp"
#include "About.hpp"
#include "Application.hpp"

#include "Utility.cpp"
#include "Debug.cpp"
#include "Error.cpp"
#include "Dialog.cpp"
#include "CustomEvents.cpp"
#include "Window.cpp"
#include "Cursor.cpp"
#include "KeyBindings.cpp"
#include "Settings.cpp"
#include "VertexBuffer.cpp"
#include "Renderer.cpp"
#include "Texture.cpp"
#include "TextureAtlas.cpp"
#include "Font.cpp"
#include "Shader.cpp"
#include "ResourceManager.cpp"
#include "UserInterface.cpp"
#include "Level.cpp"
#include "Map.cpp"
#include "Gpak.cpp"
#include "Hotbar.cpp"
#include "Toolbar.cpp"
#include "Tooltip.cpp"
#include "TilePanel.cpp"
#include "LayerPanel.cpp"
#include "ControlPanel.cpp"
#include "NewDialog.cpp"
#include "ResizeDialog.cpp"
#include "PathDialog.cpp"
#include "TabBar.cpp"
#include "Palette.cpp"
#include "LevelEditor.cpp"
#include "MapEditor.cpp"
#include "Editor.cpp"
#include "StatusBar.cpp"
#include "ColorPicker.cpp"
#include "PreferencesMenu.cpp"
#include "About.cpp"

#if defined(BuildDebug)
#include "utility/GenerateAtlas.hpp"
#include "utility/PackTexture.hpp"
#endif // BuildDebug

#include "Application.cpp"
