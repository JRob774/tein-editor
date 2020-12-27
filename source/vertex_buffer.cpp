/*******************************************************************************
 * Facilities for creating a vertex buffer for storing/drawing vertices.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

TEINAPI void create_vertex_buffer (Vertex_Buffer& buffer)
{
    glGenVertexArrays(1, &buffer.vao);
    glBindVertexArray(buffer.vao);

    glGenBuffers(1, &buffer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);

    glVertexPointer  (2, GL_FLOAT, sizeof(Vertex), (void*)(0*sizeof(float)));
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)(2*sizeof(float)));
    glColorPointer   (4, GL_FLOAT, sizeof(Vertex), (void*)(4*sizeof(float)));

    glEnableClientState(GL_VERTEX_ARRAY       );
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY        );
}

TEINAPI void free_vertex_buffer (Vertex_Buffer& buffer)
{
    glDeleteVertexArrays(1, &buffer.vao);
    glDeleteBuffers(1, &buffer.vbo);

    buffer.verts.clear();
}

/* -------------------------------------------------------------------------- */

TEINAPI void put_buffer_vertex (Vertex_Buffer& buffer, Vertex v)
{
    buffer.verts.push_back(v);
}

TEINAPI void draw_vertex_buffer (Vertex_Buffer& buffer, Buffer_Mode mode)
{
    if (buffer.verts.empty()) return;

    glBindVertexArray(buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);

    GLsizeiptr size = buffer.verts.size() * sizeof(Vertex);
    glBufferData(GL_ARRAY_BUFFER, size, &buffer.verts[0], GL_DYNAMIC_DRAW);

    glDrawArrays(static_cast<GLenum>(mode), 0, static_cast<GLsizei>(buffer.verts.size()));
}

/* -------------------------------------------------------------------------- */

TEINAPI void clear_vertex_buffer (Vertex_Buffer& buffer)
{
    buffer.verts.clear();
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
