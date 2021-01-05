struct Texture
{
    GLuint handle;
    float w,h; // @Improve: These should probably be ints?
};

// SHOULD NOT BE USED DIRECTLY -- USE THE ASSET MANAGER INSTEAD!!!
EditorAPI void LoadTexture (Texture& texture, std::string fileName);
EditorAPI void FreeTexture (Texture& texture);
