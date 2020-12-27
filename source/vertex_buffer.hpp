/*******************************************************************************
 * Facilities for creating a vertex buffer for storing/drawing vertices.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

enum class Buffer_Mode: GLenum
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
    vec2 position;
    vec2 texcoord;
    vec4 outcolor;
};

struct Vertex_Buffer
{
    VAO vao;
    VBO vbo;

    std::vector<Vertex> verts;
};

/* -------------------------------------------------------------------------- */

FILDEF void create_vertex_buffer (Vertex_Buffer& buffer);
FILDEF void free_vertex_buffer   (Vertex_Buffer& buffer);

FILDEF void put_buffer_vertex  (Vertex_Buffer& buffer, Vertex vertex);
FILDEF void draw_vertex_buffer (Vertex_Buffer& buffer, Buffer_Mode mode);

FILDEF void clear_vertex_buffer (Vertex_Buffer& buffer);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
