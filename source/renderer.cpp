/*******************************************************************************
 * Simplisitic 2D renderer implemented using OpenGL 3.0 as the backend.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static SDL_GLContext gl_context;
static Window*    render_target;

static Quad renderer_viewport;
static Vec4 renderer_draw_color;

static std::stack<Quad> scissor_stack;

static Shader untextured_shader;
static Shader   textured_shader;
static Shader       text_shader;

static float texture_draw_scale_x;
static float texture_draw_scale_y;

static float font_draw_scale;

static GLfloat max_gl_texture_size;

static Buffer_Mode immediate_buffer_draw_mode;

static Vertex_Buffer draw_buffer;
static Vertex_Buffer tile_buffer;
static Vertex_Buffer text_buffer;

// Batched tile rendering.
static Vec4     tile_draw_color;
static Texture* tile_texture;
// Batched text rendering.
static Vec4     text_draw_color;
static Font*    text_font;

/* -------------------------------------------------------------------------- */

TEINAPI Quad internal__convert_viewport (Quad viewport)
{
    // Converts a viewport in our top-left format to GL's bottom-left format.
    Quad converted;

    converted.x = viewport.x;
    converted.y = (get_render_target_h() - (viewport.y + viewport.h));
    converted.w = viewport.w;
    converted.h = viewport.h;

    return converted;
}

TEINAPI void internal__set_texture0_uniform (Shader shader, GLenum unit)
{
    GLint location = glGetUniformLocation(shader, "texture0");
    glUniform1i(location, unit);
}

TEINAPI void internal__dump_opengl_debug_info ()
{
    const GLubyte* gl_version   = glGetString(GL_VERSION                 );
    const GLubyte* gl_slversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    const GLubyte* gl_renderer  = glGetString(GL_RENDERER                );
    const GLubyte* gl_vendor    = glGetString(GL_VENDOR                  );

    GLint max_texture_size;
    GLint max_texture_units;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE,        &max_texture_size );
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);

    BeginDebugSection("OpenGL:");
    LogDebug("Version %s"            , gl_version       );
    LogDebug("GLSL Version %s"       , gl_slversion     );
    LogDebug("Renderer: %s"          , gl_renderer      );
    LogDebug("Vendor: %s"            , gl_vendor        );
    LogDebug("Max Texture Size: %d"  , max_texture_size );
    LogDebug("Max Texture Units: %d" , max_texture_units);
    EndDebugSection();
}

/* -------------------------------------------------------------------------- */

TEINAPI bool init_renderer ()
{
    gl_context = SDL_GL_CreateContext(GetWindowFromName("WINMAIN").window);
    if (!gl_context)
    {
        LogError(ERR_MIN, "Failed to create GL context! (%s)", SDL_GetError());
        return false;
    }

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        LogError(ERR_MIN, "Failed to load OpenGL procedures!");
        return false;
    }

    LogDebug("Initialized OpenGL Renderer");
    internal__dump_opengl_debug_info();

    renderer_draw_color  = Vec4(1,1,1,1);
    texture_draw_scale_x = 1;
    texture_draw_scale_y = 1;
    font_draw_scale      = 1;

    glGetFloatv(GL_MAX_TEXTURE_SIZE, &max_gl_texture_size);

    untextured_shader = load_shader_resource("shaders/untextured.shader");
    if (!untextured_shader)
    {
        LogError(ERR_MAX, "Failed to load the untextured shader!");
        return false;
    }

    textured_shader = load_shader_resource("shaders/textured.shader");
    if (!textured_shader)
    {
        LogError(ERR_MAX, "Failed to load the textured shader!");
        return false;
    }

    // We carry on even on failure because text will still be drawn it
    // will just be extremely ugly... but it's not worth failing over.
    text_shader = load_shader_resource("shaders/text.shader");
    if (!text_shader)
    {
        LogError(ERR_MED, "Failed to load the text shader!");
    }

    // By default we render to the main window.
    set_render_target(&GetWindowFromName("WINMAIN"));

    set_viewport(0, 0, get_render_target_w(), get_render_target_h());

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    create_vertex_buffer(draw_buffer);
    create_vertex_buffer(tile_buffer);
    create_vertex_buffer(text_buffer);

    immediate_buffer_draw_mode = Buffer_Mode::TRIANGLE_STRIP;

    return true;
}

TEINAPI void quit_renderer ()
{
    free_vertex_buffer(draw_buffer);
    free_vertex_buffer(tile_buffer);
    free_vertex_buffer(text_buffer);

    free_shader(untextured_shader);
    free_shader(  textured_shader);
    free_shader(      text_shader);

    SDL_GL_DeleteContext(gl_context);
    gl_context = NULL;
}

/* -------------------------------------------------------------------------- */

TEINAPI void render_clear (Vec4 clear)
{
    glClearColor(clear.r, clear.g, clear.b, clear.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

TEINAPI void render_present ()
{
    assert(render_target);
    if (render_target)
    {
        SDL_GL_SwapWindow(render_target->window);
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI Vec2 screen_to_world (Vec2 screen)
{
    // GL expects bottom-left so we have to convert our viewport first.
    Quad v = internal__convert_viewport(renderer_viewport);

    // We also need to do flip the Y coordinate to use this system.
    screen.y = get_render_target_h() - screen.y;
    Vec3 coord(screen.x, screen.y, 0);

    Mat4 projection;
    Mat4 modelview;

    float matrix[16];

    glGetFloatv(GL_PROJECTION_MATRIX, matrix);
    projection = glm::make_mat4(matrix);
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    modelview = glm::make_mat4(matrix);

    Vec4 viewport(v.x, v.y, v.w, v.h);

    coord = glm::unProject(coord, modelview, projection, viewport);

    Vec2 world(coord.x, coord.y);

    return world;
}

TEINAPI Vec2 world_to_screen (Vec2 world)
{
    // GL expects bottom-left so we have to convert our viewport first.
    Quad v = internal__convert_viewport(renderer_viewport);

    Vec3 coord(world.x, world.y, 0);

    Mat4 projection;
    Mat4 modelview;

    float matrix[16];

    glGetFloatv(GL_PROJECTION_MATRIX, matrix);
    projection = glm::make_mat4(matrix);
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    modelview = glm::make_mat4(matrix);

    Vec4 viewport(v.x, v.y, v.w, v.h);

    coord = glm::project(coord, modelview, projection, viewport);

    // We also need to do flip the Y coordinate to use this system.
    Vec2 screen(coord.x, coord.y);

    screen.x -= v.x;
    screen.y  = get_render_target_h() - (screen.y + renderer_viewport.y);

    return screen;
}

/* -------------------------------------------------------------------------- */

TEINAPI float get_max_texture_size ()
{
    return max_gl_texture_size;
}

/* -------------------------------------------------------------------------- */

TEINAPI Window* get_render_target ()
{
    return render_target;
}

TEINAPI void set_render_target (Window* window)
{
    render_target = window;
    assert(render_target);

    if (SDL_GL_MakeCurrent(render_target->window, gl_context) < 0)
    {
        render_target = NULL;
        LogError(ERR_MED, "Failed to set render target! (%s)", SDL_GetError());
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI float get_render_target_w ()
{
    int w = 0;
    if (render_target) SDL_GL_GetDrawableSize(render_target->window, &w, NULL);
    return static_cast<float>(w);
}

TEINAPI float get_render_target_h ()
{
    int h = 0;
    if (render_target) SDL_GL_GetDrawableSize(render_target->window, NULL, &h);
    return static_cast<float>(h);
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_orthographic (float l, float r, float b, float t)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(l, r, b, t, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_viewport (float x, float y, float w, float h)
{
    // GL expects bottom-left so we have to flip the Y coordinate around.
    renderer_viewport = { x, y, w, h };
    Quad v = internal__convert_viewport(renderer_viewport);

    GLint   vx = static_cast<GLint>(v.x);
    GLint   vy = static_cast<GLint>(v.y);
    GLsizei vw = static_cast<GLsizei>(v.w);
    GLsizei vh = static_cast<GLsizei>(v.h);

    glViewport(vx, vy, vw, vh);

    set_orthographic(0, w, h, 0);
}

TEINAPI void set_viewport (Quad v)
{
    set_viewport(v.x, v.y, v.w, v.h);
}

TEINAPI Quad get_viewport ()
{
    return renderer_viewport;
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_draw_color (float r, float g, float b, float a)
{
    renderer_draw_color = { r, g, b, a };
}

TEINAPI void set_draw_color (Vec4 color)
{
    renderer_draw_color = color;
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_line_width (float width)
{
    glLineWidth(width);
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_texture_draw_scale (float sx, float sy)
{
    texture_draw_scale_x = sx;
    texture_draw_scale_y = sy;
}

TEINAPI float get_texture_draw_scale_x ()
{
    return texture_draw_scale_x;
}

TEINAPI float get_texture_draw_scale_y ()
{
    return texture_draw_scale_y;
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_font_draw_scale (float scale)
{
    font_draw_scale = scale;
}

TEINAPI float get_font_draw_scale ()
{
    return font_draw_scale;
}

/* -------------------------------------------------------------------------- */

TEINAPI void begin_scissor (float x, float y, float w, float h)
{
    // Our version of setting the scissor region takes the currently set
    // viewport into consideration rather than basing the region on the
    // size of the screen/window. As a result, we need to perform a few
    // operations on the values passed in to make sure this the case.

    // We push scissor regions onto a stack so we can stack scissor calls.
    // This is particularly useful for the GUI which uses many scissors.
    if (scissor_stack.size() == 0) glEnable(GL_SCISSOR_TEST);
    scissor_stack.push({ x, y, w, h });

    // GL expects bottom-left so we have to flip the Y coordinate around.
    GLint   sx = static_cast<GLint>(renderer_viewport.x + x);
    GLint   sy = static_cast<GLint>(get_render_target_h() - (renderer_viewport.y + (y + h)));
    GLsizei sw = static_cast<GLsizei>(w);
    GLsizei sh = static_cast<GLsizei>(h);

    glScissor(sx, sy, sw, sh);
}

TEINAPI void end_scissor ()
{
    // Pop the last scissor region off the stack.
    Quad s = scissor_stack.top();
    scissor_stack.pop();

    // GL expects bottom-left so we have to flip the Y coordinate around.
    GLint   x = static_cast<GLint>(renderer_viewport.x + s.x);
    GLint   y = static_cast<GLint>(get_render_target_h() - (renderer_viewport.y + (s.y + s.h)));
    GLsizei w = static_cast<GLsizei>(s.w);
    GLsizei h = static_cast<GLsizei>(s.h);

    glScissor(x, y, w, h);

    if (scissor_stack.size() == 0) glDisable(GL_SCISSOR_TEST);
}

/* -------------------------------------------------------------------------- */

TEINAPI void begin_stencil ()
{
    glEnable(GL_STENCIL_TEST);

    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
}

TEINAPI void end_stencil ()
{
    stencil_mode_draw();
    glDisable(GL_STENCIL_TEST);
}

TEINAPI void stencil_mode_erase ()
{
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glStencilFunc(GL_ALWAYS, 1, ~0u);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

TEINAPI void stencil_mode_draw ()
{
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glStencilFunc(GL_NOTEQUAL, 1, ~0u);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

/* -------------------------------------------------------------------------- */

TEINAPI void draw_line (float x1, float y1, float x2, float y2)
{
    glUseProgram(untextured_shader);

    put_buffer_vertex(draw_buffer, { Vec2(x1,y1), Vec2(0,0), renderer_draw_color });
    put_buffer_vertex(draw_buffer, { Vec2(x2,y2), Vec2(0,0), renderer_draw_color });

    draw_vertex_buffer(draw_buffer, Buffer_Mode::LINES);
    clear_vertex_buffer(draw_buffer);
}

TEINAPI void draw_quad (float x1, float y1, float x2, float y2)
{
    glUseProgram(untextured_shader);

    // We want to use .5f otherwise the lines don't draw where we want them.
    x1 += .5f;
    y1 += .5f;
    x2 -= .5f;
    y2 -= .5f;

    put_buffer_vertex(draw_buffer, { Vec2(x1,y1), Vec2(0,0), renderer_draw_color });
    put_buffer_vertex(draw_buffer, { Vec2(x2,y1), Vec2(0,0), renderer_draw_color });
    put_buffer_vertex(draw_buffer, { Vec2(x2,y2), Vec2(0,0), renderer_draw_color });
    put_buffer_vertex(draw_buffer, { Vec2(x1,y2), Vec2(0,0), renderer_draw_color });

    draw_vertex_buffer(draw_buffer, Buffer_Mode::LINE_LOOP);
    clear_vertex_buffer(draw_buffer);
}

TEINAPI void fill_quad (float x1, float y1, float x2, float y2)
{
    glUseProgram(untextured_shader);

    put_buffer_vertex(draw_buffer, { Vec2(x1,y2), Vec2(0,0), renderer_draw_color });
    put_buffer_vertex(draw_buffer, { Vec2(x1,y1), Vec2(0,0), renderer_draw_color });
    put_buffer_vertex(draw_buffer, { Vec2(x2,y2), Vec2(0,0), renderer_draw_color });
    put_buffer_vertex(draw_buffer, { Vec2(x2,y1), Vec2(0,0), renderer_draw_color });

    draw_vertex_buffer(draw_buffer, Buffer_Mode::TRIANGLE_STRIP);
    clear_vertex_buffer(draw_buffer);
}

/* -------------------------------------------------------------------------- */

TEINAPI void draw_texture (const Texture& tex, float x, float y, const Quad* clip)
{
    glBindTexture(GL_TEXTURE_2D, tex.handle);
    glEnable(GL_TEXTURE_2D);

    Defer { glDisable(GL_TEXTURE_2D); };

    glUseProgram(textured_shader);
    internal__set_texture0_uniform(textured_shader, GL_TEXTURE0);

    float cx1, cy1, cx2, cy2;
    float w, h;

    if (clip)
    {
        cx1 =       (clip->x / tex.w);
        cy1 =       (clip->y / tex.h);
        cx2 = cx1 + (clip->w / tex.w);
        cy2 = cy1 + (clip->h / tex.h);

        w = clip->w * texture_draw_scale_x;
        h = clip->h * texture_draw_scale_y;
    }
    else
    {
        cx1 = 0;
        cy1 = 0;
        cx2 = 1;
        cy2 = 1;

        w = tex.w * texture_draw_scale_x;
        h = tex.h * texture_draw_scale_y;
    }

    float x1 = x  - (w / 2); // Center anchor.
    float y1 = y  - (h / 2); // Center anchor.
    float x2 = x1 + w;
    float y2 = y1 + h;

    put_buffer_vertex(draw_buffer, { Vec2(x1,y2), Vec2(cx1,cy2), tex.color });
    put_buffer_vertex(draw_buffer, { Vec2(x1,y1), Vec2(cx1,cy1), tex.color });
    put_buffer_vertex(draw_buffer, { Vec2(x2,y2), Vec2(cx2,cy2), tex.color });
    put_buffer_vertex(draw_buffer, { Vec2(x2,y1), Vec2(cx2,cy1), tex.color });

    draw_vertex_buffer(draw_buffer, Buffer_Mode::TRIANGLE_STRIP);
    clear_vertex_buffer(draw_buffer);
}

TEINAPI void draw_text (const Font& fnt, float x, float y, std::string text)
{
    glBindTexture(GL_TEXTURE_2D, fnt.cache.at(fnt.current_pt_size).handle);
    glEnable(GL_TEXTURE_2D);

    Defer { glDisable(GL_TEXTURE_2D); };

    glUseProgram(text_shader);
    internal__set_texture0_uniform(text_shader, GL_TEXTURE0);

    std::vector<Vertex> verts;

    int index      = 0;
    int prev_index = 0;

    float cx = x;
    float cy = y;

    float scale = font_draw_scale;

    const Texture& cache = fnt.cache.at(fnt.current_pt_size);
    auto& glyphs = fnt.glyphs.at(fnt.current_pt_size);

    for (const char* c=text.c_str(); *c; ++c)
    {
        if (*c < 0 || *c >= TOTAL_GLYPH_COUNT) continue;

        cx += (get_font_kerning(fnt, *c, index, prev_index) * scale);

        switch (*c)
        {
            case ('\r'): cx = x;                                                       break;
            case ('\n'): cx = x, cy += (fnt.line_gap.at(fnt.current_pt_size) * scale); break;
            case ('\t'): cx += get_font_tab_width(fnt) * scale;                        break;

            default:
            {
                const Font_Glyph& glyph = glyphs.at(*c);
                const Quad& clip = glyph.bounds;

                float bearing_x = glyph.bearing.x * scale;
                float bearing_y = glyph.bearing.y * scale;

                float advance = glyph.advance * scale;

                float cx1 =       (clip.x / cache.w);
                float cy1 =       (clip.y / cache.h);
                float cx2 = cx1 + (clip.w / cache.w);
                float cy2 = cy1 + (clip.h / cache.h);

                float w = clip.w * scale;
                float h = clip.h * scale;

                float x1 = roundf(cx + bearing_x);
                float y1 = roundf(cy - bearing_y);
                float x2 = roundf(x1 + w);
                float y2 = roundf(y1 + h);

                put_buffer_vertex(draw_buffer, { Vec2(x1,y2), Vec2(cx1,cy2), fnt.color }); // V0
                put_buffer_vertex(draw_buffer, { Vec2(x1,y1), Vec2(cx1,cy1), fnt.color }); // V1
                put_buffer_vertex(draw_buffer, { Vec2(x2,y2), Vec2(cx2,cy2), fnt.color }); // V2
                put_buffer_vertex(draw_buffer, { Vec2(x2,y2), Vec2(cx2,cy2), fnt.color }); // V2
                put_buffer_vertex(draw_buffer, { Vec2(x1,y1), Vec2(cx1,cy1), fnt.color }); // V1
                put_buffer_vertex(draw_buffer, { Vec2(x2,y1), Vec2(cx2,cy1), fnt.color }); // V3

                cx += advance;
            } break;
        }
    }

    draw_vertex_buffer(draw_buffer, Buffer_Mode::TRIANGLES);
    clear_vertex_buffer(draw_buffer);
}

/* -------------------------------------------------------------------------- */

TEINAPI void begin_draw (Buffer_Mode mode)
{
    immediate_buffer_draw_mode = mode;
}

TEINAPI void end_draw ()
{
    glUseProgram(untextured_shader);

    draw_vertex_buffer(draw_buffer, immediate_buffer_draw_mode);
    clear_vertex_buffer(draw_buffer);
}

/* -------------------------------------------------------------------------- */

TEINAPI void put_vertex (float x, float y, Vec4 color)
{
    put_buffer_vertex(draw_buffer, { Vec2(x,y), Vec2(0,0), color });
}

/* -------------------------------------------------------------------------- */

TEINAPI void push_matrix (Matrix_Mode mode)
{
    glMatrixMode(static_cast<GLenum>(mode));
    glPushMatrix();
    glLoadIdentity();
}

TEINAPI void pop_matrix (Matrix_Mode mode)
{
    glMatrixMode(static_cast<GLenum>(mode));
    glPopMatrix();
}

/* -------------------------------------------------------------------------- */

TEINAPI void translate (float x, float y)
{
    glTranslatef(x, y, 0);
}

TEINAPI void rotate (float x, float y, float angle)
{
    glRotatef(angle, x, y, 0);
}

TEINAPI void scale (float x, float y)
{
    glScalef(x, y, 1);
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_tile_batch_texture (Texture& tex)
{
    tile_texture = &tex;
}

TEINAPI void set_text_batch_font (Font& fnt)
{
    text_font = &fnt;
}

TEINAPI void set_tile_batch_color (Vec4 color)
{
    tile_draw_color = color;
}

TEINAPI void set_text_batch_color (Vec4 color)
{
    text_draw_color = color;
}

/* -------------------------------------------------------------------------- */

TEINAPI void draw_batched_tile (float x, float y, const Quad* clip)
{
    assert(tile_texture);

    float cx1 =       (clip->x / tile_texture->w);
    float cy1 =       (clip->y / tile_texture->h);
    float cx2 = cx1 + (clip->w / tile_texture->w);
    float cy2 = cy1 + (clip->h / tile_texture->h);

    float w = clip->w * texture_draw_scale_x;
    float h = clip->h * texture_draw_scale_y;

    float x1 = x  - (w / 2); // Center anchor.
    float y1 = y  - (h / 2); // Center anchor.
    float x2 = x1 + w;
    float y2 = y1 + h;

    put_buffer_vertex(tile_buffer, { Vec2(x1,y2), Vec2(cx1,cy2), tile_draw_color }); // V0
    put_buffer_vertex(tile_buffer, { Vec2(x1,y1), Vec2(cx1,cy1), tile_draw_color }); // V1
    put_buffer_vertex(tile_buffer, { Vec2(x2,y2), Vec2(cx2,cy2), tile_draw_color }); // V2
    put_buffer_vertex(tile_buffer, { Vec2(x2,y2), Vec2(cx2,cy2), tile_draw_color }); // V2
    put_buffer_vertex(tile_buffer, { Vec2(x1,y1), Vec2(cx1,cy1), tile_draw_color }); // V1
    put_buffer_vertex(tile_buffer, { Vec2(x2,y1), Vec2(cx2,cy1), tile_draw_color }); // V3
}

TEINAPI void draw_batched_text (float x, float y, std::string text)
{
    int index      = 0;
    int prev_index = 0;

    float cx = x;
    float cy = y;

    const Font& fnt = *text_font;
    float scale = font_draw_scale;

    const auto& cache    = fnt.cache   .at(fnt.current_pt_size);
    const auto& glyphs   = fnt.glyphs  .at(fnt.current_pt_size);
    const auto& line_gap = fnt.line_gap.at(fnt.current_pt_size);

    for (const char* c=text.c_str(); *c; ++c)
    {
        if (*c < 0 || *c >= TOTAL_GLYPH_COUNT) continue;

        cx += (get_font_kerning(fnt, *c, index, prev_index) * scale);

        switch (*c)
        {
            case ('\r'): cx = x;                                  break;
            case ('\n'): cx = x, cy += (line_gap * scale);        break;
            case ('\t'): cx += (get_font_tab_width(fnt) * scale); break;

            default:
            {
                const Font_Glyph& glyph = glyphs.at(*c);
                const Quad& clip = glyph.bounds;

                float bearing_x = glyph.bearing.x * scale;
                float bearing_y = glyph.bearing.y * scale;

                float advance = glyph.advance * scale;

                float cx1 =       (clip.x / cache.w);
                float cy1 =       (clip.y / cache.h);
                float cx2 = cx1 + (clip.w / cache.w);
                float cy2 = cy1 + (clip.h / cache.h);

                float w = clip.w * scale;
                float h = clip.h * scale;

                float x1 = roundf(cx + bearing_x);
                float y1 = roundf(cy - bearing_y);
                float x2 = roundf(x1 + w);
                float y2 = roundf(y1 + h);

                put_buffer_vertex(text_buffer, { Vec2(x1,y2), Vec2(cx1,cy2), text_draw_color }); // V0
                put_buffer_vertex(text_buffer, { Vec2(x1,y1), Vec2(cx1,cy1), text_draw_color }); // V1
                put_buffer_vertex(text_buffer, { Vec2(x2,y2), Vec2(cx2,cy2), text_draw_color }); // V2
                put_buffer_vertex(text_buffer, { Vec2(x2,y2), Vec2(cx2,cy2), text_draw_color }); // V2
                put_buffer_vertex(text_buffer, { Vec2(x1,y1), Vec2(cx1,cy1), text_draw_color }); // V1
                put_buffer_vertex(text_buffer, { Vec2(x2,y1), Vec2(cx2,cy1), text_draw_color }); // V3

                cx += advance;
            } break;
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void flush_batched_tile ()
{
    glBindTexture(GL_TEXTURE_2D, tile_texture->handle);
    glEnable(GL_TEXTURE_2D);

    glUseProgram(textured_shader);

    draw_vertex_buffer(tile_buffer, Buffer_Mode::TRIANGLES);
    clear_vertex_buffer(tile_buffer);

    glDisable(GL_TEXTURE_2D);
}

TEINAPI void flush_batched_text ()
{
    glBindTexture(GL_TEXTURE_2D, text_font->cache.at(text_font->current_pt_size).handle);
    glEnable(GL_TEXTURE_2D);

    glUseProgram(text_shader);

    draw_vertex_buffer(text_buffer, Buffer_Mode::TRIANGLES);
    clear_vertex_buffer(text_buffer);

    glDisable(GL_TEXTURE_2D);
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
