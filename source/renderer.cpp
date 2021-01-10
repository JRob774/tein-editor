//
// Vertex Buffer
//

EditorAPI void CreateVertexBuffer (VertexBuffer& buffer)
{
    glGenVertexArrays(1, &buffer.vao);
    glBindVertexArray(buffer.vao);

    glGenBuffers(1, &buffer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex,position)));
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex,texCoord)));
    glColorPointer(4, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex,color)));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
}

EditorAPI void FreeVertexBuffer (VertexBuffer& buffer)
{
    glDeleteVertexArrays(1, &buffer.vao);
    glDeleteBuffers(1, &buffer.vbo);
    buffer.verts.clear();
}

EditorAPI void DrawVertexBuffer (VertexBuffer& buffer, DrawMode mode)
{
    if (buffer.verts.empty()) return; // There's nothing to draw.
    glBindVertexArray(buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    GLsizeiptr size = buffer.verts.size() * sizeof(Vertex);
    glBufferData(GL_ARRAY_BUFFER, size, &buffer.verts[0], GL_DYNAMIC_DRAW);
    glDrawArrays(static_cast<GLenum>(mode), 0, static_cast<GLsizei>(buffer.verts.size()));
}

EditorAPI void ClearVertexBuffer (VertexBuffer& buffer)
{
    buffer.verts.clear();
}

//
// Shader
//

Internal GLuint Internal_CompileShader (std::string& source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    const char* cSource = source.c_str();
    glShaderSource(shader, 1, &cSource, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::string infoLog;
        infoLog.resize(infoLogLength);
        glGetShaderInfoLog(shader, infoLogLength, NULL, &infoLog[0]);
        LogSingleSystemMessage("shader", "Failed to compile shader:\n%s", infoLog.c_str());
    }

    return shader;
}

Internal void Internal_CreateShader (Shader& shader, std::stringstream& stream)
{
    std::string line;

    std::string vertSource;
    std::string fragSource;

    // By default we put the contents into both shaders. It is only once we reach one of the
    // attribute lines [VertProgram], [FragProgram], etc. that we place lines in a specific shader.
    bool inVertProgram = true;
    bool inFragProgram = true;

    while (std::getline(stream, line)) {
        // Trim leading whitespace.
        line.erase(0,line.find_first_not_of(" \t\n\r\f\v"));

        // Handle our attributes.
        if      (line == "[VertProgram]") inVertProgram = true, inFragProgram = false;
        else if (line == "[FragProgram]") inVertProgram = false, inFragProgram = true;
        else {
            // Add lines to the appropriate shader sources.
            if (inVertProgram) vertSource += line + "\n";
            if (inFragProgram) fragSource += line + "\n";
        }
    }

    GLuint vert = Internal_CompileShader(vertSource, GL_VERTEX_SHADER);
    GLuint frag = Internal_CompileShader(fragSource, GL_FRAGMENT_SHADER);

    Defer { glDeleteShader(vert); };
    Defer { glDeleteShader(frag); };

    shader.program = glCreateProgram();
    glAttachShader(shader.program, vert);
    glAttachShader(shader.program, frag);
    glLinkProgram(shader.program);

    GLint success;
    glGetProgramiv(shader.program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint infoLogLength;
        glGetProgramiv(shader.program, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::string infoLog;
        infoLog.resize(infoLogLength);
        glGetProgramInfoLog(shader.program, infoLogLength, NULL, &infoLog[0]);
        LogSingleSystemMessage("shader", "Failed to link shader:\n%s", infoLog.c_str());
    }
}

EditorAPI void LoadShader (Shader& shader, std::string fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open()) {
        LogSingleSystemMessage("shader", "Failed to load shader from file '%s'!", fileName.c_str());
    } else {
        std::stringstream stream;
        stream << file.rdbuf();
        Internal_CreateShader(shader, stream);
    }
}

EditorAPI void FreeShader (Shader& shader)
{
    glDeleteProgram(shader.program);
}

//
// Texture
//

Internal void Internal_CreateTexture (Texture& texture, int width, int height, int bytesPerPixel, void* rawData)
{
    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &texture.handle);
    glBindTexture(GL_TEXTURE_2D, texture.handle);

    // @Improve: Right now there is no way to specify how a texture should wrap or be filtered?
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

//
// Framebuffer
//

EditorAPI void CreateFramebuffer (Framebuffer& framebuffer, float width, float height)
{
    ResizeFramebuffer(framebuffer, width, height);
}

EditorAPI void FreeFramebuffer (Framebuffer& framebuffer)
{
    glDeleteFramebuffers(1, &framebuffer.handle);
    glDeleteTextures(1, &framebuffer.texture);
}

EditorAPI void ResizeFramebuffer (Framebuffer& framebuffer, float width, float height)
{
    FreeFramebuffer(framebuffer); // Delete the old contents (if any).

    glGenFramebuffers(1, &framebuffer.handle);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.handle);

    glGenTextures(1, &framebuffer.texture);
    glBindTexture(GL_TEXTURE_2D, framebuffer.texture);

    int textureWidth = static_cast<int>(width);
    int textureHeight = static_cast<int>(height);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // @Improve: Right now there is no way to specify how a framebuffer should wrap or be filtered?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LogSingleSystemMessage("[framebuffer]", "Failed to complete framebuffer resize!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    framebuffer.width = width;
    framebuffer.height = height;
}
