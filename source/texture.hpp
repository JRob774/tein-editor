/*******************************************************************************
 * Load/create image data from file/memory into 2D OpenGL textures.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

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
