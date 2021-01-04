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

#include <filesystem>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include <SDL2/SDL.h>

// Header Includes
#include "Utility.hpp"

// Source Includes
#include "Utility.cpp"
