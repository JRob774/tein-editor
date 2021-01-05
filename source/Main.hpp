#if defined(PLATFORM_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
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

// Header Includes
#include "Utility.hpp"
#include "Logger.hpp"
#include "Settings.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "VertexBuffer.hpp"
#include "AssetManager.hpp"
#include "Window.hpp"

// Source Includes
#include "Utility.cpp"
#include "Logger.cpp"
#include "Settings.cpp"
#include "Shader.cpp"
#include "Texture.cpp"
#include "VertexBuffer.cpp"
#include "AssetManager.cpp"
#include "Window.cpp"
