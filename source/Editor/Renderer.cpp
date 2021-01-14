#include "Renderer.hpp"

#include "Logger.hpp"

#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb/stb_image.h>

namespace TEIN
{
    void VertexBuffer::Create ()
    {
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

        glVertexPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex,position)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex,texCoord)));
        glColorPointer(4, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex,color)));

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
    }
    void VertexBuffer::Free ()
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        m_Verts.clear();
    }
    void VertexBuffer::Draw (DrawMode drawMode)
    {
        if (m_Verts.empty()) return; // There's nothing to draw.
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        GLsizeiptr size = m_Verts.size() * sizeof(Vertex);
        glBufferData(GL_ARRAY_BUFFER, size, &m_Verts[0], GL_DYNAMIC_DRAW);
        glDrawArrays(drawMode, 0, static_cast<GLsizei>(m_Verts.size()));
    }
    void VertexBuffer::Clear ()
    {
        m_Verts.clear();
    }

    GLuint Shader::Compile (std::string& source, GLenum type)
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
            Logger::SingleSystemMessage("shader", "Failed to compile shader:\n%s", infoLog.c_str());
        }

        return shader;
    }
    void Shader::Create (std::stringstream& stream)
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

        GLuint vert = Compile(vertSource, GL_VERTEX_SHADER);
        GLuint frag = Compile(fragSource, GL_FRAGMENT_SHADER);

        Defer { glDeleteShader(vert); };
        Defer { glDeleteShader(frag); };

        m_Program = glCreateProgram();
        glAttachShader(m_Program, vert);
        glAttachShader(m_Program, frag);
        glLinkProgram(m_Program);

        GLint success;
        glGetProgramiv(m_Program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint infoLogLength;
            glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &infoLogLength);
            std::string infoLog;
            infoLog.resize(infoLogLength);
            glGetProgramInfoLog(m_Program, infoLogLength, NULL, &infoLog[0]);
            Logger::SingleSystemMessage("shader", "Failed to link shader:\n%s", infoLog.c_str());
        }
    }
    void Shader::Load (std::string fileName)
    {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            Logger::SingleSystemMessage("shader", "Failed to load shader from file '%s'!", fileName.c_str());
        } else {
            std::stringstream stream;
            stream << file.rdbuf();
            Create(stream);
        }
    }
    void Shader::Free ()
    {
        glDeleteProgram(m_Program);
    }

    void Texture::Create (int width, int height, int bytesPerPixel, void* rawData)
    {
        glActiveTexture(GL_TEXTURE0);

        glGenTextures(1, &m_Handle);
        glBindTexture(GL_TEXTURE_2D, m_Handle);

        // @Improve: Right now there is no way to specify how a texture should wrap or be filtered?
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width,height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawData);

        m_Width = width;
        m_Height = height;
    }
    void Texture::Load (std::string fileName)
    {
        constexpr int BytesPerPixel = 4;
        int width, height, bytesPerPixel;
        U8* rawData = stbi_load(fileName.c_str(), &width,&height,&bytesPerPixel,BytesPerPixel); // We force all textures to 4-channel RGBA.
        if (!rawData) {
            Logger::SingleSystemMessage("texture", "Failed to load texture from file '%s'!", fileName.c_str());
        } else {
            Defer { stbi_image_free(rawData); };
            Create(width, height, BytesPerPixel, rawData);
        }
    }
    void Texture::Free ()
    {
        glDeleteTextures(1, &m_Handle);
    }

    void Framebuffer::Create (int width, int height)
    {
        Resize(width, height);
    }
    void Framebuffer::Free ()
    {
        glDeleteFramebuffers(1, &m_Handle);
        glDeleteTextures(1, &m_Texture);
    }
    void Framebuffer::Resize (int width, int height)
    {
        Free(); // Delete the old contents (if any).

        glGenFramebuffers(1, &m_Handle);
        glBindFramebuffer(GL_FRAMEBUFFER, m_Handle);

        glGenTextures(1, &m_Texture);
        glBindTexture(GL_TEXTURE_2D, m_Texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        // @Improve: Right now there is no way to specify how a framebuffer should wrap or be filtered?
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            Logger::SingleSystemMessage("framebuffer", "Failed to complete framebuffer resize!");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_Width = width;
        m_Height = height;
    }

    namespace Renderer
    {
        void Clear (float r, float g, float b, float a)
        {
            glClearColor(r,g,b,a);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void BindFramebuffer (Framebuffer* framebuffer)
        {
            if (framebuffer) {
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->m_Handle);
            } else {
                glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
            }
        }
    }
}
