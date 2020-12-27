/*******************************************************************************
 * Includes all external library code used by the application codebase.
 * Authored By Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#ifndef EXTERNAL_HPP__ /*/////////////////////////////////////////////////////*/
#define EXTERNAL_HPP__

#ifdef COMPILER_HAS_PRAGMA_ONCE
#pragma once
#endif

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

#if defined(COMPILER_MSVC)
#pragma warning(push, 0)
#endif

/* -------------------------------------------------------------------------- */

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <FT2/ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_STATIC
#define STBRP_STATIC
#include <STB/stb_image_write.h>
#include <STB/stb_image.h>
#include <STB/stb_rect_pack.h>

#define CURL_STATICLIB
#include <CURL/curl.h>

#include <GON/gon.hpp>
#include <GON/gon.cpp>

#include <JSON/json.hpp>

#include <GLAD/glad/glad.h>
#include <GLAD/glad/glad.c>

/* -------------------------------------------------------------------------- */

#if defined(COMPILER_MSVC)
#pragma warning(pop)
#endif

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

#endif /* EXTERNAL_HPP__ /////////////////////////////////////////////////////*/

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
