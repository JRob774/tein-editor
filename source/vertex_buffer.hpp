enum class BufferMode: GLenum
{
    POINTS         = GL_POINTS,
    LINE_STRIP     = GL_LINE_STRIP,
    LINE_LOOP      = GL_LINE_LOOP,
    LINES          = GL_LINES,
    TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
    TRIANGLE_FAN   = GL_TRIANGLE_FAN,
    TRIANGLES      = GL_TRIANGLES
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
TEINAPI void FreeVertexBuffer   (VertexBuffer& buffer);
TEINAPI void PutBufferVertex    (VertexBuffer& buffer, Vertex vertex);
TEINAPI void DrawVertexBuffer   (VertexBuffer& buffer, BufferMode mode);
TEINAPI void ClearVertexBuffer  (VertexBuffer& buffer);
