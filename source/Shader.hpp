struct Shader
{
    GLuint program;
};

// SHOULD NOT BE USED DIRECTLY -- USE THE ASSET MANAGER INSTEAD!!!
EditorAPI void LoadShader (Shader& shader, std::string fileName);
EditorAPI void FreeShader (Shader& shader);
