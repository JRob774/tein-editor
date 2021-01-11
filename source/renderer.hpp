//
// Vertex Buffer
//

enum class DrawMode: GLenum
{
    Points        = GL_POINTS,
    LineStrip     = GL_LINE_STRIP,
    LineLoop      = GL_LINE_LOOP,
    Lines         = GL_LINES,
    TriangleStrip = GL_TRIANGLE_STRIP,
    TriangleFan   = GL_TRIANGLE_FAN,
    Triangles     = GL_TRIANGLES
};

struct Vertex
{
    Vec2 position;
    Vec2 texCoord;
    Vec4 color;
};

struct VertexBuffer
{
    GLuint vao, vbo;
    std::vector<Vertex> verts;
};

EditorAPI void CreateVertexBuffer (VertexBuffer& buffer);
EditorAPI void FreeVertexBuffer   (VertexBuffer& buffer);
EditorAPI void DrawVertexBuffer   (VertexBuffer& buffer, DrawMode drawMode);
EditorAPI void ClearVertexBuffer  (VertexBuffer& buffer);

//
// Shader
//

struct Shader
{
    GLuint program;
};

EditorAPI void LoadShader (Shader& shader, std::string fileName);
EditorAPI void FreeShader (Shader& shader);

//
// Texture
//

struct Texture
{
    GLuint handle;
    int width;
    int height;
};

EditorAPI void LoadTexture (Texture& texture, std::string fileName);
EditorAPI void FreeTexture (Texture& texture);

//
// Framebuffer
//

struct Framebuffer
{
    GLuint handle = GL_NONE; // Framebuffer
    GLuint texture = GL_NONE; // Color Attachment
    int width;
    int height;
};

EditorAPI void CreateFramebuffer (Framebuffer& framebuffer, int width, int height);
EditorAPI void FreeFramebuffer   (Framebuffer& framebuffer);
EditorAPI void ResizeFramebuffer (Framebuffer& framebuffer, int width, int height);
