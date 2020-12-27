/*******************************************************************************
 * Load/create image data from file/memory into 2D OpenGL textures.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

TEINAPI bool load_texture_from_data (Texture& tex, const std::vector<u8>& file_data, Texture_Wrap wrap)
{
    const stbi_uc* buffer = &file_data[0];
    int size = static_cast<int>(file_data.size());

    int w, h, bpp;
    u8* raw_data = stbi_load_from_memory(buffer, size, &w, &h, &bpp, 0);
    if (!raw_data)
    {
        LOG_ERROR(ERR_MIN, "Failed to load texture from data!");
        return false;
    }
    Defer { stbi_image_free(raw_data); };

    return create_texture(tex, w, h, bpp, raw_data, wrap);
}

TEINAPI bool load_texture_from_file (Texture& tex, std::string file_name, Texture_Wrap wrap)
{
    // Build an absolute path to the file based on the executable location.
    file_name = make_path_absolute(file_name);

    int w, h, bpp;
    u8* raw_data = stbi_load(file_name.c_str(), &w, &h, &bpp, 0);
    if (!raw_data)
    {
        LOG_ERROR(ERR_MIN, "Failed to load texture '%s'!", file_name.c_str());
        return false;
    }
    Defer { stbi_image_free(raw_data); };

    return create_texture(tex, w, h, bpp, raw_data, wrap);
}

TEINAPI void free_texture (Texture& tex)
{
    glDeleteTextures(1, &tex.handle);
}

TEINAPI bool create_texture (Texture& tex, int w, int h, int bpp, void* data, Texture_Wrap wrap)
{
    // Bytes-per-pixel needs to be one of these otherwise we can't use.
    assert(bpp == 1 || bpp == 2 || bpp == 3 || bpp == 4);

    int max_texture_size = static_cast<int>(get_max_texture_size());
    if (w > max_texture_size || h > max_texture_size)
    {
        LOG_ERROR(ERR_MIN, "Texture size %dx%d too large for GPU!", w,h);
        return false;
    }

    if (!data)
    {
        LOG_ERROR(ERR_MIN, "No texture data passed for creation!");
        return false;
    }

    // The caller should always pass in pixel data that is RGBA ordered.
    // So it's just a matter of determinig how many components we have.
    GLenum format;
    switch (bpp)
    {
    default:
    case(1): format = GL_RED;  break;
    case(2): format = GL_RG;   break;
    case(3): format = GL_RGB;  break;
    case(4): format = GL_RGBA; break;
    }

    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &tex.handle);
    glBindTexture(GL_TEXTURE_2D, tex.handle);

    #if defined(RENDERER_USE_MIPMAPS)
    GLenum filter = GL_LINEAR_MIPMAP_LINEAR;
    #else
    GLenum filter = GL_LINEAR;
    #endif // RENDERER_USE_MIPMAPS

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,       wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,       wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    GLint   level_of_detail = 0;
    GLint   internal_format = GL_RGBA;
    GLsizei texture_width   = w;
    GLsizei texture_height  = h;
    GLint   border          = 0; // Docs say must be zero!
    GLenum  type            = GL_UNSIGNED_BYTE;

    glTexImage2D(GL_TEXTURE_2D, level_of_detail, internal_format,
      texture_width, texture_height, border, format, type, data);
    #if defined(RENDERER_USE_MIPMAPS)
    glGenerateMipmap(GL_TEXTURE_2D);
    #endif // RENDERER_USE_MIPMAPS

    tex.w = static_cast<float>(w), tex.h = static_cast<float>(h);
    tex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

    return true;
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
