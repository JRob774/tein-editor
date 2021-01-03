enum class BufferMode: GLenum
{
    Points = GL_POINTS,
    LineStrip = GL_LINE_STRIP,
    LineLoop = GL_LINE_LOOP,
    Lines = GL_LINES,
    TriangleStrip = GL_TRIANGLE_STRIP,
    TriangleFan = GL_TRIANGLE_FAN,
    Triangles = GL_TRIANGLES
};

typedef GLuint VBO;
typedef GLuint VAO;

struct Vertex
{
    Vec2 position;
    Vec2 texCoord;
    Vec4 outColor;
};

struct VertexBuffer
{
    VAO vao;
    VBO vbo;

    std::vector<Vertex> verts;
};

TEINAPI void CreateVertexBuffer (VertexBuffer& buffer);
TEINAPI void FreeVertexBuffer (VertexBuffer& buffer);
TEINAPI void PutBufferVertex (VertexBuffer& buffer, Vertex vertex);
TEINAPI void DrawVertexBuffer (VertexBuffer& buffer, BufferMode mode);
TEINAPI void ClearVertexBuffer (VertexBuffer& buffer);
