TEINAPI void CreateVertexBuffer (VertexBuffer& buffer)
{
    glGenVertexArrays(1, &buffer.vao);
    glBindVertexArray(buffer.vao);

    glGenBuffers(1, &buffer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex,position)));
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex,texCoord)));
    glColorPointer(4, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex,outColor)));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
}

TEINAPI void FreeVertexBuffer (VertexBuffer& buffer)
{
    glDeleteVertexArrays(1, &buffer.vao);
    glDeleteBuffers(1, &buffer.vbo);
    buffer.verts.clear();
}

TEINAPI void PutBufferVertex (VertexBuffer& buffer, Vertex v)
{
    buffer.verts.push_back(v);
}

TEINAPI void DrawVertexBuffer (VertexBuffer& buffer, BufferMode mode)
{
    if (buffer.verts.empty()) return;

    glBindVertexArray(buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    GLsizeiptr size = buffer.verts.size() * sizeof(Vertex);
    glBufferData(GL_ARRAY_BUFFER, size, &buffer.verts[0], GL_DYNAMIC_DRAW);
    glDrawArrays(static_cast<GLenum>(mode), 0, static_cast<GLsizei>(buffer.verts.size()));
}

TEINAPI void ClearVertexBuffer (VertexBuffer& buffer)
{
    buffer.verts.clear();
}
