TEINAPI bool LoadTextureFromData (Texture& tex, const std::vector<U8>& fileData, TextureWrap wrap)
{
    const stbi_uc* buffer = &fileData[0];
    int size = static_cast<int>(fileData.size());

    int w,h,bpp;
    U8* rawData = stbi_load_from_memory(buffer, size, &w,&h,&bpp,0);
    if (!rawData) {
        LogError(ErrorLevel::Min, "Failed to load texture from data!");
        return false;
    }
    Defer { stbi_image_free(rawData); };

    return CreateTexture(tex, w,h,bpp, rawData, wrap);
}

TEINAPI bool LoadTextureFromFile (Texture& tex, std::string fileName, TextureWrap wrap)
{
    // Build an absolute path to the file based on the executable location.
    fileName = MakePathAbsolute(fileName);

    int w,h,bpp;
    U8* rawData = stbi_load(fileName.c_str(), &w,&h,&bpp,0);
    if (!rawData) {
        LogError(ErrorLevel::Min, "Failed to load texture '%s'!", fileName.c_str());
        return false;
    }
    Defer { stbi_image_free(rawData); };

    return CreateTexture(tex, w,h,bpp, rawData, wrap);
}

TEINAPI void FreeTexture (Texture& tex)
{
    glDeleteTextures(1, &tex.handle);
}

TEINAPI bool CreateTexture (Texture& tex, int w, int h, int bpp, void* data, TextureWrap wrap)
{
    // Bytes-per-pixel needs to be one of these otherwise we can't use.
    assert(bpp == 1 || bpp == 2 || bpp == 3 || bpp == 4);

    int maxTextureSize = static_cast<int>(GetMaxTextureSize());
    if (w > maxTextureSize || h > maxTextureSize) {
        LogError(ErrorLevel::Min, "Texture size %dx%d too large for GPU!", w,h);
        return false;
    }

    if (!data) {
        LogError(ErrorLevel::Min, "No texture data passed for creation!");
        return false;
    }

    // The caller should always pass in pixel data that is RGBA ordered.
    // So it's just a matter of determinig how many components we have.
    GLenum format;
    switch (bpp) {
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLenum>(wrap));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLenum>(wrap));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    GLint   level_of_detail = 0;
    GLint   internal_format = GL_RGBA;
    GLsizei texture_width   = w;
    GLsizei texture_height  = h;
    GLint   border          = 0; // Docs say must be zero!
    GLenum  type            = GL_UNSIGNED_BYTE;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w,h, 0, format, GL_UNSIGNED_BYTE, data);
    #if defined(RENDERER_USE_MIPMAPS)
    glGenerateMipmap(GL_TEXTURE_2D);
    #endif // RENDERER_USE_MIPMAPS

    tex.w = static_cast<float>(w);
    tex.h = static_cast<float>(h);

    tex.color = Vec4(1,1,1,1);

    return true;
}
