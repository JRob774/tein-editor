Internal void Internal_CreateTexture (Texture& texture, int width, int height, int bytesPerPixel, void* rawData)
{
    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &texture.handle);
    glBindTexture(GL_TEXTURE_2D, texture.handle);

    // @Improve: Right now there is no way to specify how a texture should wrap or be filtered.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width,height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawData);

    texture.w = static_cast<float>(width);
    texture.h = static_cast<float>(height);
}

EditorAPI void LoadTexture (Texture& texture, std::string fileName)
{
    constexpr int BytesPerPixel = 4;
    int width,height,bytesPerPixel;
    U8* rawData = stbi_load(fileName.c_str(), &width,&height,&bytesPerPixel,BytesPerPixel); // We force all textures to 4-channel RGBA.
    if (!rawData) {
        LogSingleSystemMessage("texture", "Failed to load texture from file '%s'!", fileName.c_str());
    } else {
        Defer { stbi_image_free(rawData); };
        Internal_CreateTexture(texture, width,height,BytesPerPixel, rawData);
    }
}

EditorAPI void FreeTexture (Texture& texture)
{
    glDeleteTextures(1, &texture.handle);
}
