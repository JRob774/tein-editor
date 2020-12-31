enum class TextureWrap: GLenum
{
    Repeat = GL_REPEAT,
    MirroredRepeat = GL_MIRRORED_REPEAT,
    ClampToEdge = GL_CLAMP_TO_EDGE
};

struct Texture
{
    GLuint handle;
    float w,h;
    Vec4 color;
};

TEINAPI bool LoadTextureFromData (Texture& tex, const std::vector<U8>& fileData, TextureWrap wrap = TextureWrap::ClampToEdge);
TEINAPI bool LoadTextureFromFile (Texture& tex, std::string fileName, TextureWrap wrap = TextureWrap::ClampToEdge);

TEINAPI void FreeTexture (Texture& tex);

// RGBA ordering of pixel components is expected when using CreateTexture.
TEINAPI bool CreateTexture (Texture& tex, int w, int h, int bpp, void* data, TextureWrap wrap = TextureWrap::ClampToEdge);
