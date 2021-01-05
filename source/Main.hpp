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
#include <stack>

#include <glm/glm.hpp>

#include <SDL2/SDL.h>

#include <GON.hpp>
#include <GON.cpp>

#include <glad/glad.h>
#include <glad/glad.c>

// Header Includes
#include "Utility.hpp"
#include "Logger.hpp"
#include "Settings.hpp"
#include "Window.hpp"

// Source Includes
#include "Utility.cpp"
#include "Logger.cpp"
#include "Settings.cpp"
#include "Window.cpp"
