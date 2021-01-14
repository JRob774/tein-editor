#pragma once

#include "Utility.hpp"

#include <glad/glad.h>

namespace TEIN
{
    DECLARE_ENUM(e_DrawMode, DrawMode, GLenum)
    {
        Points = GL_POINTS,
        LineStrip = GL_LINE_STRIP,
        LineLoop = GL_LINE_LOOP,
        Lines = GL_LINES,
        TriangleStrip = GL_TRIANGLE_STRIP,
        TriangleFan = GL_TRIANGLE_FAN,
        Triangles = GL_TRIANGLES
    };

    struct Vertex
    {
        Vec2 position;
        Vec2 texCoord;
        Vec4 color;
    };

    class VertexBuffer
    {
    public:
        void Create ();
        void Free   ();
        void Draw   (DrawMode drawMode);
        void Clear  ();

        GLuint m_VAO, m_VBO;
        std::vector<Vertex> m_Verts;
    };

    class Shader
    {
    public:
        void Load (std::string fileName);
        void Free ();

        GLuint m_Program;

    private:
        GLuint Compile (std::string& source, GLenum type);
        void   Create  (std::stringstream& stream);
    };

    class Texture
    {
    public:
        void Load (std::string fileName);
        void Free ();

        int m_Width, m_Height;
        GLuint m_Handle;

    private:
        void Create (int width, int height, int bytesPerPixel, void* rawData);
    };

    class Framebuffer
    {
    public:
        void Create (int width, int height);
        void Free   ();
        void Resize (int width, int height);

        GLuint m_Handle = GL_NONE;
        GLuint m_Texture = GL_NONE;
        int m_Width;
        int m_Height;
    };

    namespace Renderer
    {
        void Clear (float r, float g, float b, float a = 1.0f);
        void BindFramebuffer (Framebuffer* framebuffer); // NULL to set to the main window.
    }
}
