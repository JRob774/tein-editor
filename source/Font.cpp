static constexpr float gTabLengthInSpaces = 4;
static constexpr float gFontGlyphCachePad = 6;

namespace Internal
{
    TEINAPI bool SetFontPointSize (Font& font, int pointSize)
    {
        // We use the current display's DPI to determine the point size.
        float hDPI,vDPI;
        if (SDL_GetDisplayDPI(0, NULL, &hDPI, &vDPI) != 0)
        {
            LogError(ERR_MIN, "Failed to determine display DPI! (%s)", SDL_GetError());
            return false;
        }

        FT_UInt hRes = static_cast<FT_UInt>(hDPI);
        FT_UInt vRes = static_cast<FT_UInt>(vDPI);

        FT_F26Dot6 pointHeight = pointSize * 64;

        if (FT_Set_Char_Size(font.face, 0, pointHeight, hRes, vRes) != 0)
        {
            LogError(ERR_MIN, "Failed to set font glyph size!");
            return false;
        }

        return true;
    }

    TEINAPI bool CreateFont (Font& font, int pointSize, float cacheSize)
    {
        // Make sure the glyph cache size is within the GL texture bounds.
        // If not we set it to that size and log a low-priority error.
        GLfloat finalCacheSize = std::min(GetMaxTextureSize(), cacheSize);
        if (finalCacheSize < cacheSize)
        {
            LogError(ERR_MIN, "Font cache size shrunk to %f!", finalCacheSize);
        }

        if (!Internal::SetFontPointSize(font, pointSize)) return false;
        font.currentPointSize = pointSize;

        // An 8-bit buffer that we can easily write our rasterized glyph data
        // into before building a GPU texture using our built-in texture code.
        size_t cacheRow = static_cast<size_t>(finalCacheSize);
        size_t cacheBytes = cacheRow * cacheRow;

        U8* buffer = Malloc(U8, cacheBytes);
        if (!buffer)
        {
            LogError(ERR_MIN, "Failed to create glyph buffer!");
            return false;
        }
        Defer { Free(buffer); };

        memset(buffer, 0, sizeof(U8)*cacheBytes); // We don't want the buffer filled with garbage data!

        font.hasKerning = FT_HAS_KERNING(font.face);
        font.color = Vec4(1,1,1,1);
        font.lineGap.insert({ pointSize, static_cast<float>(font.face->size->metrics.height >> 6) });

        // Go through glyph-by-glyph and bake all of the bitmaps to the final
        // glyph cache texture so that they can easily and quickly be rendered.
        //
        // Our current primitive method of packing involves simply moving from
        // left-to-right and top-to-bottom packing glyphs until we hit end up
        // hitting the size of the cache and move down to the next glyph row.

        FT_GlyphSlot slot = font.face->glyph;
        FT_Bitmap* bitmap = &slot->bitmap;

        Vec2 cursor = Vec2(0,0);

        font.glyphs.insert({ pointSize, std::vector<FontGlyph>(gTotalGlyphCount) });
        assert(font.glyphs.at(font.currentPointSize).size() == gTotalGlyphCount);

        for (int i=0; i<gTotalGlyphCount; ++i)
        {
            int index = FT_Get_Char_Index(font.face, i);
            if (FT_Load_Glyph(font.face, index, FT_LOAD_RENDER) != 0)
            {
                LogError(ERR_MIN, "Failed to load glyph '%c'!", i);
                return false;
            }

            float bitmapAdvance = static_cast<float>(slot->advance.x >> 6);
            float bitmapLeft = static_cast<float>(slot->bitmap_left);
            float bitmapTop = static_cast<float>(slot->bitmap_top);
            float bitmapWidth = static_cast<float>(bitmap->width);
            float bitmapHeight = static_cast<float>(bitmap->rows);

            // Move down a row if we have reached the edge of the cache.
            if (cursor.x + bitmapWidth >= finalCacheSize)
            {
                cursor.y += (font.lineGap[pointSize] + gFontGlyphCachePad);
                cursor.x = 0.0f;
                // If we hit the bottom edge then we are out of space.
                if (cursor.y + bitmapHeight >= finalCacheSize)
                {
                    LogError(ERR_MIN, "Font cache too small!");
                    return false;
                }
            }

            font.glyphs[pointSize][i].advance   = bitmapAdvance;
            font.glyphs[pointSize][i].bearing.x = bitmapLeft;
            font.glyphs[pointSize][i].bearing.y = bitmapTop;
            font.glyphs[pointSize][i].bounds.x  = cursor.x;
            font.glyphs[pointSize][i].bounds.y  = cursor.y;
            font.glyphs[pointSize][i].bounds.w  = bitmapWidth;
            font.glyphs[pointSize][i].bounds.h  = bitmapHeight+1;

            // Write the bitmap content into our cache buffer line-by-line.
            size_t cx = static_cast<size_t>(cursor.x), cy = static_cast<size_t>(cursor.y);
            for (FT_UInt y=0; y<bitmap->rows; ++y)
            {
                void* dst = buffer + ((cy+y)*cacheRow+cx);
                void* src = bitmap->buffer + (y*bitmap->pitch);
                memcpy(dst, src, bitmap->pitch);
            }

            // Move to the right to pack the next glyph for the font.
            cursor.x += bitmapWidth + gFontGlyphCachePad;
        }

        // We can now convert the buffer into a GPU accelerated texture.
        int cacheWidth = static_cast<int>(finalCacheSize);
        int cacheHeight = static_cast<int>(finalCacheSize);

        font.cache.insert({ pointSize, Texture() });
        return CreateTexture(font.cache[pointSize], cacheWidth, cacheHeight, 1, buffer);
    }
}

TEINAPI bool LoadFontFromData (Font& font, const std::vector<U8>& fileData, std::vector<int> pointSizes, float cacheSize)
{
    assert(pointSizes.size());

    font.data.assign(fileData.begin(), fileData.end());

    const FT_Byte* buffer = &font.data[0];
    FT_Long size = static_cast<FT_Long>(font.data.size());

    if (FT_New_Memory_Face(gFreetype, buffer, size, 0, &font.face) != 0)
    {
        LogError(ERR_MIN, "Failed to load font from data!");
        return false;
    }

    for (auto pointSize: pointSizes) if (!Internal::CreateFont(font, pointSize, cacheSize)) return false;
    SetFontPointSize(font, pointSizes.at(0));

    return true;
}

TEINAPI bool LoadFontFromFile (Font& font, std::string fileName, std::vector<int> pointSizes, float cacheSize)
{
    assert(pointSizes.size());

    // Build an absolute path to the file based on the executable location.
    fileName = MakePathAbsolute(fileName);

    if (FT_New_Face(gFreetype, fileName.c_str(), 0, &font.face) != 0)
    {
        LogError(ERR_MIN, "Failed to load font '%s'!", fileName.c_str());
        return false;
    }

    for (auto pointSize: pointSizes) if (!Internal::CreateFont(font, pointSize, cacheSize)) return false;
    SetFontPointSize(font, pointSizes.at(0));

    return true;
}

TEINAPI void FreeFont (Font& font)
{
    for (auto cache: font.cache) FreeTexture(cache.second);

    FT_Done_Face(font.face);

    font.cache.clear ();
    font.data.clear();
    font.lineGap.clear();
    font.glyphs.clear();

    font.currentPointSize = 0;
}

TEINAPI float GetFontKerning (const Font& font, int c, int& i, int& p)
{
    FT_Vector kerning = {};
    i = FT_Get_Char_Index(font.face, c);
    if (font.hasKerning && i && p)
    {
        FT_Get_Kerning(font.face, p, i, FT_KERNING_DEFAULT, &kerning);
    }
    p = i;
    return static_cast<float>(kerning.x >> 6);
}

TEINAPI float GetFontTabWidth (const Font& font)
{
    return (font.glyphs.at(font.currentPointSize).at(32).advance * gTabLengthInSpaces);
}

TEINAPI float GetGlyphAdvance (const Font& font, int c, int& i, int& p)
{
    auto& glyphs = font.glyphs.at(font.currentPointSize);
    if (c >= 0 && c < gTotalGlyphCount) return (glyphs.at(c).advance + GetFontKerning(font, c, i, p));
    return 0;
}

TEINAPI float GetTextWidth (const Font& font, std::string text)
{
    float maxWidth = 0;
    float width = 0;

    int i = 0;
    int p = 0;

    for (const char* c=text.c_str(); *c; ++c)
    {
        switch (*c)
        {
            case ('\n'):
            {
                if (width > maxWidth) maxWidth = width;
                width = 0;
            } break;
            case ('\t'):
            {
                width += GetFontTabWidth(font);
            } break;
            default:
            {
                width += GetGlyphAdvance(font, *c, i, p);
            } break;
        }
    }

    return std::max(maxWidth, width);
}

TEINAPI float GetTextHeight (const Font& font, std::string text)
{
    if (text.empty()) return 0.0f;
    float height = font.lineGap.at(font.currentPointSize);
    for (const char* c=text.c_str(); *c; ++c)
    {
        if (*c == '\n') height += font.lineGap.at(font.currentPointSize);
    }
    return height;
}

TEINAPI float GetTextWidthScaled (const Font& font, std::string text)
{
    return (GetTextWidth(font, text) * GetFontDrawScale());
}

TEINAPI float GetTextHeightScaled (const Font& font, std::string text)
{
    return (GetTextHeight(font, text) * GetFontDrawScale());
}

TEINAPI void SetFontPointSize (Font& font, int pointSize)
{
    assert(font.glyphs.find(pointSize) != font.glyphs.end());
    Internal::SetFontPointSize(font, pointSize);
    font.currentPointSize = pointSize;
}
