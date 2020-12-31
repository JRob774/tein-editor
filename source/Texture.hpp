enum TextureWrap: GLenum
{
    TEXTURE_WRAP_REPEAT = GL_REPEAT,
    TEXTURE_WRAP_MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
    TEXTURE_WRAP_CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE
};

struct Texture
{
    GLuint handle;
    float w,h;
    Vec4 color;
};

TEINAPI bool LoadTextureFromData (Texture& tex, const std::vector<U8>& fileData, TextureWrap wrap = TEXTURE_WRAP_CLAMP_TO_EDGE);
TEINAPI bool LoadTextureFromFile (Texture& tex, std::string            fileName, TextureWrap wrap = TEXTURE_WRAP_CLAMP_TO_EDGE);

TEINAPI void FreeTexture (Texture& tex);

// RGBA ordering of pixel components is expected when using CreateTexture.
TEINAPI bool CreateTexture (Texture& tex, int w, int h, int bpp, void* data, TextureWrap wrap = TEXTURE_WRAP_CLAMP_TO_EDGE);
