static constexpr float gDefaultFontGlyphCacheSize = 1024;
static constexpr int   gTotalGlyphCount           = 128;
static constexpr int   gSmallFontPointSize        = 9;
static constexpr int   gLargeFontPointSize        = 14;

static FT_Library gFreetype;

struct FontGlyph
{
    Vec2 bearing;
    Quad bounds;
    float advance;
};

struct Font
{
    // This is an internal variable only used if the font is being loaded
    // from data. This is because FT_New_Memory_Face does not copy over
    // the data passed into it so the pointer provided must remain valid
    // until we are done with the font face and finally call FT_Done_Face.
    std::vector<U8> data;

    std::map<int,std::vector<FontGlyph>> glyphs;

    FT_Face face;
    std::map<int,Texture> cache;

    bool hasKerning;
    Vec4 color;

    std::map<int,float> lineGap;

    // Should only ever be set using the SetFontPointSize function!
    int currentPointSize;
};

TEINAPI bool LoadFontFromData (Font& font, const std::vector<U8>& fileData, std::vector<int> pointSizes = { gSmallFontPointSize }, float cacheSize = gDefaultFontGlyphCacheSize);
TEINAPI bool LoadFontFromFile (Font& font, std::string            fileName, std::vector<int> pointSizes = { gSmallFontPointSize }, float cacheSize = gDefaultFontGlyphCacheSize);
TEINAPI void FreeFont         (Font& font);

TEINAPI float GetFontKerning  (const Font& font, int c, int& i, int& p);
TEINAPI float GetFontTabWidth (const Font& font);
TEINAPI float GetGlyphAdvance (const Font& font, int c, int& i, int& p);

TEINAPI float GetTextWidth  (const Font& font, std::string text);
TEINAPI float GetTextHeight (const Font& font, std::string text);

TEINAPI float GetTextWidthScaled  (const Font& font, std::string text);
TEINAPI float GetTextHeightScaled (const Font& font, std::string text);

TEINAPI void SetFontPointSize (Font& font, int pointSize);
