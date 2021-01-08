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
