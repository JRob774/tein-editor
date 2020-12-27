/*******************************************************************************
 * Load/create image data from file/memory into 2D OpenGL textures.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#ifndef TEXTURE_HPP__ /*//////////////////////////////////////////////////////*/
#define TEXTURE_HPP__

#ifdef COMPILER_HAS_PRAGMA_ONCE
#pragma once
#endif

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

typedef GLenum Texture_Wrap;

static constexpr Texture_Wrap TEXTURE_WRAP_REPEAT          = GL_REPEAT;
static constexpr Texture_Wrap TEXTURE_WRAP_MIRRORED_REPEAT = GL_MIRRORED_REPEAT;
static constexpr Texture_Wrap TEXTURE_WRAP_CLAMP_TO_EDGE   = GL_CLAMP_TO_EDGE;

struct Texture
{
    GLuint handle;
    float  w;
    float  h;
    vec4   color;
};

STDDEF bool load_texture_from_data (Texture& tex, const std::vector<u8>& file_data, Texture_Wrap wrap = TEXTURE_WRAP_CLAMP_TO_EDGE);
FILDEF bool load_texture_from_file (Texture& tex, std::string            file_name, Texture_Wrap wrap = TEXTURE_WRAP_CLAMP_TO_EDGE);

FILDEF void free_texture (Texture& tex);

// RGBA ordering of pixel components is expected when using create_texture().
STDDEF bool create_texture (Texture& tex, int w, int h, int bpp, void* data, Texture_Wrap wrap = TEXTURE_WRAP_CLAMP_TO_EDGE);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

#endif /* TEXTURE_HPP__ //////////////////////////////////////////////////////*/

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
