/*******************************************************************************
 * Facilities for loading and creating OpenGL vert/frag shader programs.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

TEINAPI GLuint internal__compile_shader (const GLchar* source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint info_log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
    char* info_log = cstd_malloc(char, info_log_length);
    if (info_log)
    {
        Defer { cstd_free(info_log); };

        GLint compile_success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_success);
        if (!compile_success)
        {
            glGetShaderInfoLog(shader, info_log_length, NULL, info_log);
            LOG_ERROR(ERR_MIN, "Failed to compile shader:\n%s", info_log);
        }
    }

    return shader;
}

/* -------------------------------------------------------------------------- */

TEINAPI Shader load_shader_from_source (std::string source)
{
    std::string vert_src(source);
    std::string frag_src(source);

    string_replace(vert_src, " vert()", " main()");
    string_replace(frag_src, " frag()", " main()");

    string_replace(vert_src, "COMPILING_VERTEX_PROGRAM",   "1");
    string_replace(vert_src, "COMPILING_FRAGMENT_PROGRAM", "0");

    string_replace(frag_src, "COMPILING_VERTEX_PROGRAM",   "0");
    string_replace(frag_src, "COMPILING_FRAGMENT_PROGRAM", "1");

    const GLchar* vsrc = static_cast<const GLchar*>(vert_src.c_str());
    const GLchar* fsrc = static_cast<const GLchar*>(frag_src.c_str());

    GLuint vert = internal__compile_shader(vsrc, GL_VERTEX_SHADER);
    GLuint frag = internal__compile_shader(fsrc, GL_FRAGMENT_SHADER);

    Defer { glDeleteShader(vert); glDeleteShader(frag); };

    if (!vert || !frag)
    {
        return NULL;
    }

    Shader program = glCreateProgram();

    glAttachShader(program, vert);
    glAttachShader(program, frag);

    glLinkProgram(program);

    GLint info_log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
    char* info_log = cstd_malloc(char, info_log_length);
    if (info_log)
    {
        Defer { cstd_free(info_log); };

        GLint link_success;
        glGetProgramiv(program, GL_LINK_STATUS, &link_success);
        if (!link_success)
        {
            glGetProgramInfoLog(program, info_log_length, NULL, info_log);
            LOG_ERROR(ERR_MIN, "Failed to link shader:\n%s", info_log);

            glDeleteProgram(program);
            program = NULL;
        }
    }

    return program;
}

TEINAPI Shader load_shader_from_file (std::string file_name)
{
    // Build an absolute path to the file based on the executable location.
    file_name = make_path_absolute(file_name);

    std::string source = read_entire_file(file_name);
    if (source.empty())
    {
        LOG_ERROR(ERR_MIN, "Failed to load shader '%s'!", file_name.c_str());
        return NULL;
    }

    Shader shader = load_shader_from_source(source);
    if (!shader)
    {
        LOG_ERROR(ERR_MIN, "Failed to build shader '%s'!", file_name.c_str());
    }
    return shader;
}

TEINAPI Shader load_shader_from_data (const std::vector<u8>& file_data)
{
    std::string source(file_data.begin(), file_data.end());
    return load_shader_from_source(source);
}

TEINAPI void free_shader (Shader program)
{
    glDeleteProgram(program);
}

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

/*******************************************************************************
 *
 * Copyright (c) 2020 Joshua Robertson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
*******************************************************************************/
