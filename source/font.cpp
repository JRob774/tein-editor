/*******************************************************************************
 * Load truetype font data into a packed atlas for easy rendering.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float TAB_LENGTH_IN_SPACES  = 4;
static constexpr float FONT_GLYPH_CACHE_PADS = 6;

/* -------------------------------------------------------------------------- */

TEINAPI bool internal__set_font_point_size (Font& fnt, int pt)
{
    // We use the current display's DPI to determine the point size.
    float hdpi, vdpi;
    if (SDL_GetDisplayDPI(0, NULL, &hdpi, &vdpi) != 0)
    {
        LOG_ERROR(ERR_MIN, "Failed to determine display DPI! (%s)", SDL_GetError());
        return false;
    }

    FT_UInt hres = static_cast<FT_UInt>(hdpi);
    FT_UInt vres = static_cast<FT_UInt>(vdpi);

    FT_F26Dot6 pt_height = pt * 64;

    if (FT_Set_Char_Size(fnt.face, 0, pt_height, hres, vres) != 0)
    {
        LOG_ERROR(ERR_MIN, "Failed to set font glyph size!");
        return false;
    }

    return true;
}

TEINAPI bool internal__create_font (Font& fnt, int pt, float csz)
{
    // Make sure the glyph cache size is within the GL texture bounds.
    // If not we set it to that size and log a low-priority error.
    GLfloat cache_size = std::min(get_max_texture_size(), csz);
    if (cache_size < csz)
    {
        LOG_ERROR(ERR_MIN, "Font cache size shrunk to %f!", cache_size);
    }

    if (!internal__set_font_point_size(fnt, pt)) return false;
    fnt.current_pt_size = pt;

    // An 8-bit buffer that we can easily write our rasterized glyph data
    // into before building a GPU texture using our built-in texture code.
    size_t cache_row = static_cast<size_t>(cache_size);
    size_t cache_bytes = cache_row * cache_row;

    // We use calloc to fill the buffer with black instead of garbage.
    u8* buffer = cstd_calloc(u8, cache_bytes);
    if (!buffer)
    {
        LOG_ERROR(ERR_MIN, "Failed to create glyph buffer!");
        return false;
    }
    Defer { cstd_free(buffer); };

    fnt.has_kerning = FT_HAS_KERNING(fnt.face);
    fnt.color       = vec4(1,1,1,1);
    fnt.line_gap.insert({ pt, static_cast<float>(fnt.face->size->metrics.height >> 6) });

    // Go through glyph-by-glyph and bake all of the bitmaps to the final
    // glyph cache texture so that they can easily and quickly be rendered.
    //
    // Our current primitive method of packing involves simply moving from
    // left-to-right and top-to-bottom packing glyphs until we hit end up
    // hitting the size of the cache and move down to the next glyph row.

    FT_GlyphSlot slot = fnt.face->glyph;
    FT_Bitmap* bitmap = &slot->bitmap;
    vec2       cursor = vec2(0,0);

    fnt.glyphs.insert(std::pair<int, std::vector<Font_Glyph>>(pt,
        std::vector<Font_Glyph>(TOTAL_GLYPH_COUNT)));
    assert(fnt.glyphs.at(fnt.current_pt_size).size() == TOTAL_GLYPH_COUNT);

    for (int i=0; i<TOTAL_GLYPH_COUNT; ++i)
    {
        int index = FT_Get_Char_Index(fnt.face, i);
        if (FT_Load_Glyph(fnt.face, index, FT_LOAD_RENDER) != 0)
        {
            LOG_ERROR(ERR_MIN, "Failed to load glyph '%c'!", i);
            return false;
        }

        float bitmap_advance = static_cast<float>(slot->advance.x >> 6);
        float bitmap_left    = static_cast<float>(slot->bitmap_left);
        float bitmap_top     = static_cast<float>(slot->bitmap_top);
        float bitmap_width   = static_cast<float>(bitmap->width);
        float bitmap_height  = static_cast<float>(bitmap->rows);

        // Move down a row if we have reached the edge of the cache.
        if (cursor.x + bitmap_width >= cache_size)
        {
            cursor.y += (fnt.line_gap[pt] + FONT_GLYPH_CACHE_PADS);
            cursor.x = 0.0f;
            // If we hit the bottom edge then we are out of space.
            if (cursor.y + bitmap_height >= cache_size)
            {
                LOG_ERROR(ERR_MIN, "Font cache too small!");
                return false;
            }
        }

        fnt.glyphs[pt][i].advance   = bitmap_advance;
        fnt.glyphs[pt][i].bearing.x = bitmap_left;
        fnt.glyphs[pt][i].bearing.y = bitmap_top;
        fnt.glyphs[pt][i].bounds.x  = cursor.x;
        fnt.glyphs[pt][i].bounds.y  = cursor.y;
        fnt.glyphs[pt][i].bounds.w  = bitmap_width;
        fnt.glyphs[pt][i].bounds.h  = bitmap_height+1;

        // Write the bitmap content into our cache buffer line-by-line.
        size_t cx = static_cast<size_t>(cursor.x), cy = static_cast<size_t>(cursor.y);
        for (FT_UInt y=0; y<bitmap->rows; ++y)
        {
            void* dst = buffer + ((cy+y)*cache_row+cx);
            void* src = bitmap->buffer + (y*bitmap->pitch);

            memcpy(dst, src, bitmap->pitch);
        }

        // Move to the right to pack the next glyph for the font.
        cursor.x += bitmap_width + FONT_GLYPH_CACHE_PADS;
    }

    // We can now convert the buffer into a GPU accelerated texture.
    int cache_w = static_cast<int>(cache_size);
    int cache_h = static_cast<int>(cache_size);

    fnt.cache.insert(std::pair<int,Texture>(pt, Texture()));
    return create_texture(fnt.cache[pt], cache_w, cache_h, 1, buffer);
}

/* -------------------------------------------------------------------------- */

TEINAPI bool load_font_from_data (Font& fnt, const std::vector<u8>& file_data, std::vector<int> pt, float csz)
{
    assert(pt.size());

    fnt.data.assign(file_data.begin(), file_data.end());

    const FT_Byte* buffer = &fnt.data[0];
    FT_Long size = static_cast<FT_Long>(fnt.data.size());

    if (FT_New_Memory_Face(freetype, buffer, size, 0, &fnt.face) != 0)
    {
        LOG_ERROR(ERR_MIN, "Failed to load font from data!");
        return false;
    }

    for (auto p: pt)
    {
        if (!internal__create_font(fnt, p, csz)) return false;
    }

    set_font_point_size(fnt, pt.at(0));
    return true;
}

TEINAPI bool load_font_from_file (Font& fnt, std::string file_name, std::vector<int> pt, float csz)
{
    assert(pt.size());

    // Build an absolute path to the file based on the executable location.
    file_name = make_path_absolute(file_name);

    if (FT_New_Face(freetype, file_name.c_str(), 0, &fnt.face) != 0)
    {
        LOG_ERROR(ERR_MIN, "Failed to load font '%s'!", file_name.c_str());
        return false;
    }

    for (auto p: pt)
    {
        if (!internal__create_font(fnt, p, csz)) return false;
    }

    set_font_point_size(fnt, pt.at(0));
    return true;
}

/* -------------------------------------------------------------------------- */

TEINAPI void free_font (Font& fnt)
{
    for (auto cache: fnt.cache) free_texture(cache.second);

    FT_Done_Face(fnt.face);

    fnt.cache.clear   ();
    fnt.data.clear    ();
    fnt.line_gap.clear();
    fnt.glyphs.clear  ();

    fnt.current_pt_size = 0;
}

/* -------------------------------------------------------------------------- */

TEINAPI float get_font_kerning (const Font& fnt, int c, int& i, int& p)
{
    FT_Vector kern = {};
    i = FT_Get_Char_Index(fnt.face, c);
    if (fnt.has_kerning && i && p)
    {
        FT_Get_Kerning(fnt.face, p, i, FT_KERNING_DEFAULT, &kern);
    }
    p = i;
    return static_cast<float>(kern.x >> 6);
}

TEINAPI float get_font_tab_width (const Font& fnt)
{
    return (fnt.glyphs.at(fnt.current_pt_size).at(32).advance * TAB_LENGTH_IN_SPACES);
}

TEINAPI float get_glyph_advance (const Font& fnt, int c, int& i, int& p)
{
    auto& glyphs = fnt.glyphs.at(fnt.current_pt_size);
    if (c >= 0 && c < TOTAL_GLYPH_COUNT)
    {
        return (glyphs.at(c).advance + get_font_kerning(fnt, c, i, p));
    }
    return 0;
}

/* -------------------------------------------------------------------------- */

TEINAPI float get_text_width (const Font& fnt, std::string text)
{
    float max_width = 0;
    float width = 0;

    int i = 0;
    int p = 0;

    for (const char* c=text.c_str(); *c; ++c)
    {
        switch (*c)
        {
            default:     { width += get_glyph_advance(fnt, *c, i, p); } break;
            case ('\t'): { width += get_font_tab_width(fnt);          } break;
            case ('\n'):
            {
                if (width > max_width)
                {
                    max_width = width;
                }
                width = 0;
            } break;
        }
    }

    return std::max(max_width, width);
}

TEINAPI float get_text_height (const Font& fnt, std::string text)
{
    if (text.empty()) return 0.0f;
    float height = fnt.line_gap.at(fnt.current_pt_size);
    for (const char* c=text.c_str(); *c; ++c)
    {
        if (*c == '\n') height += fnt.line_gap.at(fnt.current_pt_size);
    }
    return height;
}

/* -------------------------------------------------------------------------- */

TEINAPI float get_text_width_scaled (const Font& fnt, std::string text)
{
    return (get_text_width(fnt, text) * get_font_draw_scale());
}

TEINAPI float get_text_height_scaled (const Font& fnt, std::string text)
{
    return (get_text_height(fnt, text) * get_font_draw_scale());
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_font_point_size (Font& fnt, int pt)
{
    assert(fnt.glyphs.find(pt) != fnt.glyphs.end());
    internal__set_font_point_size(fnt, pt);
    fnt.current_pt_size = pt;
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
