namespace Internal
{
    TEINAPI GLuint CompileShader (const GLchar* source, GLenum type)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);

        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        char* infoLog = Malloc(char, infoLogLength);
        if (infoLog)
        {
            Defer { Free(infoLog); };

            GLint compileSuccess;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);
            if (!compileSuccess)
            {
                glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
                LogError(ERR_MIN, "Failed to compile shader:\n%s", infoLog);
            }
        }

        return shader;
    }
}

TEINAPI Shader LoadShaderFromSource (std::string source)
{
    std::string vertSrc(source);
    std::string fragSrc(source);

    StringReplace(vertSrc, " vert()", " main()");
    StringReplace(fragSrc, " frag()", " main()");

    StringReplace(vertSrc, "COMPILING_VERTEX_PROGRAM",   "1");
    StringReplace(vertSrc, "COMPILING_FRAGMENT_PROGRAM", "0");

    StringReplace(fragSrc, "COMPILING_VERTEX_PROGRAM",   "0");
    StringReplace(fragSrc, "COMPILING_FRAGMENT_PROGRAM", "1");

    const GLchar* vSrc = static_cast<const GLchar*>(vertSrc.c_str());
    const GLchar* fSrc = static_cast<const GLchar*>(fragSrc.c_str());

    GLuint vert = Internal::CompileShader(vSrc, GL_VERTEX_SHADER);
    GLuint frag = Internal::CompileShader(fSrc, GL_FRAGMENT_SHADER);

    Defer { glDeleteShader(vert); glDeleteShader(frag); };

    if (!vert || !frag)
    {
        return NULL;
    }

    Shader program = glCreateProgram();

    glAttachShader(program, vert);
    glAttachShader(program, frag);

    glLinkProgram(program);

    GLint infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    char* infoLog = Malloc(char, infoLogLength);
    if (infoLog)
    {
        Defer { Free(infoLog); };

        GLint linkSuccess;
        glGetProgramiv(program, GL_LINK_STATUS, &linkSuccess);
        if (!linkSuccess)
        {
            glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
            LogError(ERR_MIN, "Failed to link shader:\n%s", infoLog);

            glDeleteProgram(program);
            program = NULL;
        }
    }

    return program;
}

TEINAPI Shader LoadShaderFromData (const std::vector<U8>& fileData)
{
    std::string source(fileData.begin(), fileData.end());
    return LoadShaderFromSource(source);
}

TEINAPI Shader LoadShaderFromFile (std::string fileName)
{
    // Build an absolute path to the file based on the executable location.
    fileName = MakePathAbsolute(fileName);

    std::string source = ReadEntireFile(fileName);
    if (source.empty())
    {
        LogError(ERR_MIN, "Failed to load shader '%s'!", fileName.c_str());
        return NULL;
    }

    Shader shader = LoadShaderFromSource(source);
    if (!shader)
    {
        LogError(ERR_MIN, "Failed to build shader '%s'!", fileName.c_str());
    }
    return shader;
}

TEINAPI void FreeShader (Shader program)
{
    glDeleteProgram(program);
}
