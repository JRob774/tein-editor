/*******************************************************************************
 * Load truetype font data into a packed atlas for easy rendering.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#ifndef FONT_HPP__ /*/////////////////////////////////////////////////////////*/
#define FONT_HPP__

#ifdef COMPILER_HAS_PRAGMA_ONCE
#pragma once
#endif

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr int   DEFAULT_FONT_GLYPH_POINT_SIZE =    9;
static constexpr float DEFAULT_FONT_GLYPH_CACHE_SIZE = 1024;

static constexpr int TOTAL_GLYPH_COUNT = 128;

static constexpr int SMALL_FONT_POINT_SIZE = DEFAULT_FONT_GLYPH_POINT_SIZE;
static constexpr int LARGE_FONT_POINT_SIZE = 14;

static FT_Library freetype;

struct Font_Glyph
{
    vec2  bearing;
    quad  bounds;
    float advance;
};

struct Font
{
    // This is an internal variable only used if the font is being loaded
    // from data. This is because FT_New_Memory_Face does not copy over
    // the data passed into it so the pointer provided must remain valid
    // until we are done with the font face and finally call FT_Done_Face.
    std::vector<u8> data;

    std::map<int, std::vector<Font_Glyph>> glyphs;

    FT_Face face;
    std::map<int, Texture> cache;

    bool has_kerning;
    vec4 color;

    std::map<int, float> line_gap;

    // Should only ever be set using the set_font_point_size function!
    int current_pt_size;
};

/* -------------------------------------------------------------------------- */

FILDEF bool load_font_from_data (Font& fnt, const std::vector<u8>& file_data, std::vector<int> pt = { DEFAULT_FONT_GLYPH_POINT_SIZE }, float csz = DEFAULT_FONT_GLYPH_CACHE_SIZE);
FILDEF bool load_font_from_file (Font& fnt, std::string            file_name, std::vector<int> pt = { DEFAULT_FONT_GLYPH_POINT_SIZE }, float csz = DEFAULT_FONT_GLYPH_CACHE_SIZE);
FILDEF void free_font           (Font& fnt);

FILDEF float get_font_kerning   (const Font& fnt, int c, int& i, int& p);
FILDEF float get_font_tab_width (const Font& fnt);
FILDEF float get_glyph_advance  (const Font& fnt, int c, int& i, int& p);

INLDEF float get_text_width  (const Font& fnt, std::string text);
INLDEF float get_text_height (const Font& fnt, std::string text);

FILDEF float get_text_width_scaled (const Font& fnt, std::string text);
FILDEF float get_text_width_scaled (const Font& fnt, std::string text);

FILDEF void set_font_point_size (Font& fnt, int pt);

/* -------------------------------------------------------------------------- */

#endif /* FONT_HPP__ /////////////////////////////////////////////////////////*/

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
