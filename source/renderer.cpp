static SDL_GLContext gGLContext;
static Window* gRenderTarget;

static Quad gRendererViewport;
static Vec4 gRendererDrawColor;

static std::stack<Quad> gScissorStack;

static Shader gUntexturedShader;
static Shader gTexturedShader;
static Shader gTextShader;

static float gTextureDrawScaleX;
static float gTextureDrawScaleY;

static float gFontDrawScale;

static GLfloat gMaxGLTextureSize;

static BufferMode gImmediateBufferDrawMode;

static VertexBuffer gDrawBuffer;
static VertexBuffer gTileBuffer;
static VertexBuffer gTextBuffer;

// Batched tile rendering.
static Vec4 gTileDrawColor;
static Texture* gTileTexture;

// Batched text rendering.
static Vec4 gTextDrawColor;
static Font* gTextFont;

namespace Internal
{
    TEINAPI Quad ConvertViewport (Quad viewport)
    {
        // Converts a viewport in our top-left format to GL's bottom-left format.
        Quad converted = viewport;
        converted.y = (GetRenderTargetHeight() - (viewport.y + viewport.h));
        return converted;
    }

    TEINAPI void SetTexture0Uniform (Shader shader, GLenum unit)
    {
        GLint location = glGetUniformLocation(shader, "texture0");
        glUniform1i(location, unit);
    }

    TEINAPI void DumpOpenGLDebugInfo ()
    {
        const GLubyte* openGLVersion = glGetString(GL_VERSION);
        const GLubyte* openGLSLVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
        const GLubyte* openGLRenderer = glGetString(GL_RENDERER);
        const GLubyte* openGLVendor = glGetString(GL_VENDOR);

        GLint maxTextureSize;
        GLint maxTextureUnits;

        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);

        BeginDebugSection("OpenGL:");
        LogDebug("Version %s", openGLVersion);
        LogDebug("GLSL Version %s", openGLSLVersion);
        LogDebug("Renderer: %s", openGLRenderer);
        LogDebug("Vendor: %s", openGLVendor);
        LogDebug("Max Texture Size: %d", maxTextureSize);
        LogDebug("Max Texture Units: %d", maxTextureUnits);
        EndDebugSection();
    }
}

TEINAPI bool InitRenderer ()
{
    gGLContext = SDL_GL_CreateContext(GetWindowFromName("WINMAIN").window);
    if (!gGLContext)
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
    Internal::DumpOpenGLDebugInfo();

    gRendererDrawColor = Vec4(1,1,1,1);
    gTextureDrawScaleX = 1;
    gTextureDrawScaleY = 1;
    gFontDrawScale = 1;

    glGetFloatv(GL_MAX_TEXTURE_SIZE, &gMaxGLTextureSize);

    gUntexturedShader = load_shader_resource("shaders/untextured.shader");
    if (!gUntexturedShader)
    {
        LogError(ERR_MAX, "Failed to load the untextured shader!");
        return false;
    }

    gTexturedShader = load_shader_resource("shaders/textured.shader");
    if (!gTexturedShader)
    {
        LogError(ERR_MAX, "Failed to load the textured shader!");
        return false;
    }

    // We carry on even on failure because text will still be drawn it
    // will just be extremely ugly... but it's not worth failing over.
    gTextShader = load_shader_resource("shaders/text.shader");
    if (!gTextShader)
    {
        LogError(ERR_MED, "Failed to load the text shader!");
    }

    // By default we render to the main window.
    SetRenderTarget(&GetWindowFromName("WINMAIN"));

    SetViewport(0, 0, GetRenderTargetWidth(), GetRenderTargetHeight());

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    CreateVertexBuffer(gDrawBuffer);
    CreateVertexBuffer(gTileBuffer);
    CreateVertexBuffer(gTextBuffer);

    gImmediateBufferDrawMode = BufferMode::TRIANGLE_STRIP;

    return true;
}

TEINAPI void QuitRenderer ()
{
    FreeVertexBuffer(gDrawBuffer);
    FreeVertexBuffer(gTileBuffer);
    FreeVertexBuffer(gTextBuffer);

    free_shader(gUntexturedShader);
    free_shader(gTexturedShader);
    free_shader(gTextShader);

    SDL_GL_DeleteContext(gGLContext);
    gGLContext = NULL;
}

TEINAPI void RenderClear (Vec4 clear)
{
    glClearColor(clear.r, clear.g, clear.b, clear.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

TEINAPI void RenderPresent ()
{
    assert(gRenderTarget);
    if (gRenderTarget) SDL_GL_SwapWindow(gRenderTarget->window);
}

TEINAPI Vec2 ScreenToWorld (Vec2 screen)
{
    // GL expects bottom-left so we have to convert our viewport first.
    Quad v = Internal::ConvertViewport(gRendererViewport);

    // We also need to do flip the Y coordinate to use this system.
    screen.y = GetRenderTargetHeight() - screen.y;
    Vec3 coord(screen.x, screen.y, 0);

    Mat4 projection;
    Mat4 modelView;

    float matrix[16];

    glGetFloatv(GL_PROJECTION_MATRIX, matrix);
    projection = glm::make_mat4(matrix);
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    modelView = glm::make_mat4(matrix);

    Vec4 viewport(v.x, v.y, v.w, v.h);

    coord = glm::unProject(coord, modelView, projection, viewport);

    Vec2 world(coord.x, coord.y);

    return world;
}

TEINAPI Vec2 WorldToScreen (Vec2 world)
{
    // GL expects bottom-left so we have to convert our viewport first.
    Quad v = Internal::ConvertViewport(gRendererViewport);

    Vec3 coord(world.x, world.y, 0);

    Mat4 projection;
    Mat4 modelView;

    float matrix[16];

    glGetFloatv(GL_PROJECTION_MATRIX, matrix);
    projection = glm::make_mat4(matrix);
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    modelView = glm::make_mat4(matrix);

    Vec4 viewport(v.x, v.y, v.w, v.h);

    coord = glm::project(coord, modelView, projection, viewport);

    // We also need to do flip the Y coordinate to use this system.
    Vec2 screen(coord.x, coord.y);

    screen.x -= v.x;
    screen.y = GetRenderTargetHeight() - (screen.y + gRendererViewport.y);

    return screen;
}

TEINAPI float GetMaxTextureSize ()
{
    return gMaxGLTextureSize;
}

TEINAPI Window* GetRenderTarget ()
{
    return gRenderTarget;
}

TEINAPI void SetRenderTarget (Window* window)
{
    gRenderTarget = window;
    assert(gRenderTarget);

    if (SDL_GL_MakeCurrent(gRenderTarget->window, gGLContext) < 0)
    {
        gRenderTarget = NULL;
        LogError(ERR_MED, "Failed to set render target! (%s)", SDL_GetError());
    }
}

TEINAPI float GetRenderTargetWidth ()
{
    int w = 0;
    if (gRenderTarget) SDL_GL_GetDrawableSize(gRenderTarget->window, &w, NULL);
    return static_cast<float>(w);
}

TEINAPI float GetRenderTargetHeight ()
{
    int h = 0;
    if (gRenderTarget) SDL_GL_GetDrawableSize(gRenderTarget->window, NULL, &h);
    return static_cast<float>(h);
}

TEINAPI void SetOrthographic (float l, float r, float b, float t)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(l,r,b,t, 0,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

TEINAPI void SetViewport (float x, float y, float w, float h)
{
    // GL expects bottom-left so we have to flip the Y coordinate around.
    gRendererViewport = { x, y, w, h };
    Quad v = Internal::ConvertViewport(gRendererViewport);

    GLint   vx = static_cast<GLint>(v.x);
    GLint   vy = static_cast<GLint>(v.y);
    GLsizei vw = static_cast<GLsizei>(v.w);
    GLsizei vh = static_cast<GLsizei>(v.h);

    glViewport(vx,vy,vw,vh);
    SetOrthographic(0,w,h,0);
}
TEINAPI void SetViewport (Quad v)
{
    SetViewport(v.x,v.y,v.w,v.h);
}

TEINAPI Quad GetViewport ()
{
    return gRendererViewport;
}

TEINAPI void SetDrawColor (float r, float g, float b, float a)
{
    gRendererDrawColor = { r,g,b,a };
}
TEINAPI void SetDrawColor (Vec4 color)
{
    gRendererDrawColor = color;
}

TEINAPI void SetLineWidth (float width)
{
    glLineWidth(width);
}

TEINAPI void SetTextureDrawScale (float sx, float sy)
{
    gTextureDrawScaleX = sx;
    gTextureDrawScaleY = sy;
}

TEINAPI float GetTextureDrawScaleX ()
{
    return gTextureDrawScaleX;
}
TEINAPI float GetTextureDrawScaleY ()
{
    return gTextureDrawScaleY;
}

TEINAPI void SetFontDrawScale (float scale)
{
    gFontDrawScale = scale;
}
TEINAPI float GetFontDrawScale ()
{
    return gFontDrawScale;
}

TEINAPI void BeginScissor (float x, float y, float w, float h)
{
    // Our version of setting the scissor region takes the currently set
    // viewport into consideration rather than basing the region on the
    // size of the screen/window. As a result, we need to perform a few
    // operations on the values passed in to make sure this the case.

    // We push scissor regions onto a stack so we can stack scissor calls.
    // This is particularly useful for the GUI which uses many scissors.
    if (gScissorStack.size() == 0) glEnable(GL_SCISSOR_TEST);
    gScissorStack.push({ x,y,w,h });

    // GL expects bottom-left so we have to flip the Y coordinate around.
    GLint   sx = static_cast<GLint>(gRendererViewport.x + x);
    GLint   sy = static_cast<GLint>(GetRenderTargetHeight() - (gRendererViewport.y + (y + h)));
    GLsizei sw = static_cast<GLsizei>(w);
    GLsizei sh = static_cast<GLsizei>(h);

    glScissor(sx,sy,sw,sh);
}
TEINAPI void EndScissor ()
{
    // Pop the last scissor region off the stack.
    Quad s = gScissorStack.top();
    gScissorStack.pop();

    // GL expects bottom-left so we have to flip the Y coordinate around.
    GLint   x = static_cast<GLint>(gRendererViewport.x + s.x);
    GLint   y = static_cast<GLint>(GetRenderTargetHeight() - (gRendererViewport.y + (s.y + s.h)));
    GLsizei w = static_cast<GLsizei>(s.w);
    GLsizei h = static_cast<GLsizei>(s.h);

    glScissor(x,y,w,h);

    if (gScissorStack.size() == 0) glDisable(GL_SCISSOR_TEST);
}

TEINAPI void BeginStencil ()
{
    glEnable(GL_STENCIL_TEST);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
}
TEINAPI void EndStencil ()
{
    StencilModeDraw();
    glDisable(GL_STENCIL_TEST);
}
TEINAPI void StencilModeErase ()
{
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glStencilFunc(GL_ALWAYS, 1, ~0u);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}
TEINAPI void StencilModeDraw ()
{
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glStencilFunc(GL_NOTEQUAL, 1, ~0u);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

TEINAPI void DrawLine (float x1, float y1, float x2, float y2)
{
    glUseProgram(gUntexturedShader);

    PutBufferVertex(gDrawBuffer, { Vec2(x1,y1), Vec2(0,0), gRendererDrawColor });
    PutBufferVertex(gDrawBuffer, { Vec2(x2,y2), Vec2(0,0), gRendererDrawColor });

    DrawVertexBuffer(gDrawBuffer, BufferMode::LINES);
    ClearVertexBuffer(gDrawBuffer);
}

TEINAPI void DrawQuad (float x1, float y1, float x2, float y2)
{
    glUseProgram(gUntexturedShader);

    // We want to use .5f otherwise the lines don't draw where we want them.
    x1 += .5f;
    y1 += .5f;
    x2 -= .5f;
    y2 -= .5f;

    PutBufferVertex(gDrawBuffer, { Vec2(x1,y1), Vec2(0,0), gRendererDrawColor });
    PutBufferVertex(gDrawBuffer, { Vec2(x2,y1), Vec2(0,0), gRendererDrawColor });
    PutBufferVertex(gDrawBuffer, { Vec2(x2,y2), Vec2(0,0), gRendererDrawColor });
    PutBufferVertex(gDrawBuffer, { Vec2(x1,y2), Vec2(0,0), gRendererDrawColor });

    DrawVertexBuffer(gDrawBuffer, BufferMode::LINE_LOOP);
    ClearVertexBuffer(gDrawBuffer);
}

TEINAPI void FillQuad (float x1, float y1, float x2, float y2)
{
    glUseProgram(gUntexturedShader);

    PutBufferVertex(gDrawBuffer, { Vec2(x1,y2), Vec2(0,0), gRendererDrawColor });
    PutBufferVertex(gDrawBuffer, { Vec2(x1,y1), Vec2(0,0), gRendererDrawColor });
    PutBufferVertex(gDrawBuffer, { Vec2(x2,y2), Vec2(0,0), gRendererDrawColor });
    PutBufferVertex(gDrawBuffer, { Vec2(x2,y1), Vec2(0,0), gRendererDrawColor });

    DrawVertexBuffer(gDrawBuffer, BufferMode::TRIANGLE_STRIP);
    ClearVertexBuffer(gDrawBuffer);
}

TEINAPI void DrawTexture (const Texture& tex, float x, float y, const Quad* clip)
{
    glBindTexture(GL_TEXTURE_2D, tex.handle);
    glEnable(GL_TEXTURE_2D);

    Defer { glDisable(GL_TEXTURE_2D); };

    glUseProgram(gTexturedShader);
    Internal::SetTexture0Uniform(gTexturedShader, GL_TEXTURE0);

    float cx1,cy1,cx2,cy2;
    float w,h;

    if (clip)
    {
        cx1 =       (clip->x / tex.w);
        cy1 =       (clip->y / tex.h);
        cx2 = cx1 + (clip->w / tex.w);
        cy2 = cy1 + (clip->h / tex.h);

        w = clip->w * gTextureDrawScaleX;
        h = clip->h * gTextureDrawScaleY;
    }
    else
    {
        cx1 = 0;
        cy1 = 0;
        cx2 = 1;
        cy2 = 1;

        w = tex.w * gTextureDrawScaleX;
        h = tex.h * gTextureDrawScaleY;
    }

    float x1 = x  - (w / 2); // Center anchor.
    float y1 = y  - (h / 2); // Center anchor.
    float x2 = x1 + w;
    float y2 = y1 + h;

    PutBufferVertex(gDrawBuffer, { Vec2(x1,y2), Vec2(cx1,cy2), tex.color });
    PutBufferVertex(gDrawBuffer, { Vec2(x1,y1), Vec2(cx1,cy1), tex.color });
    PutBufferVertex(gDrawBuffer, { Vec2(x2,y2), Vec2(cx2,cy2), tex.color });
    PutBufferVertex(gDrawBuffer, { Vec2(x2,y1), Vec2(cx2,cy1), tex.color });

    DrawVertexBuffer(gDrawBuffer, BufferMode::TRIANGLE_STRIP);
    ClearVertexBuffer(gDrawBuffer);
}

TEINAPI void DrawText (const Font& fnt, float x, float y, std::string text)
{
    glBindTexture(GL_TEXTURE_2D, fnt.cache.at(fnt.currentPointSize).handle);
    glEnable(GL_TEXTURE_2D);

    Defer { glDisable(GL_TEXTURE_2D); };

    glUseProgram(gTextShader);
    Internal::SetTexture0Uniform(gTextShader, GL_TEXTURE0);

    std::vector<Vertex> verts;

    int index = 0;
    int prevIndex = 0;

    float cx = x;
    float cy = y;

    float scale = gFontDrawScale;

    const Texture& cache = fnt.cache.at(fnt.currentPointSize);
    auto& glyphs = fnt.glyphs.at(fnt.currentPointSize);

    for (const char* c=text.c_str(); *c; ++c)
    {
        if (*c < 0 || *c >= gTotalGlyphCount) continue;

        cx += (GetFontKerning(fnt, *c, index, prevIndex) * scale);

        switch (*c)
        {
            case ('\r'): cx = x;                                                       break;
            case ('\n'): cx = x, cy += (fnt.lineGap.at(fnt.currentPointSize) * scale); break;
            case ('\t'): cx += GetFontTabWidth(fnt) * scale;                           break;

            default:
            {
                const FontGlyph& glyph = glyphs.at(*c);
                const Quad& clip = glyph.bounds;

                float bearingX = glyph.bearing.x * scale;
                float bearingY = glyph.bearing.y * scale;

                float advance = glyph.advance * scale;

                float cx1 =       (clip.x / cache.w);
                float cy1 =       (clip.y / cache.h);
                float cx2 = cx1 + (clip.w / cache.w);
                float cy2 = cy1 + (clip.h / cache.h);

                float w = clip.w * scale;
                float h = clip.h * scale;

                float x1 = roundf(cx + bearingX);
                float y1 = roundf(cy - bearingY);
                float x2 = roundf(x1 + w);
                float y2 = roundf(y1 + h);

                PutBufferVertex(gDrawBuffer, { Vec2(x1,y2), Vec2(cx1,cy2), fnt.color }); // V0
                PutBufferVertex(gDrawBuffer, { Vec2(x1,y1), Vec2(cx1,cy1), fnt.color }); // V1
                PutBufferVertex(gDrawBuffer, { Vec2(x2,y2), Vec2(cx2,cy2), fnt.color }); // V2
                PutBufferVertex(gDrawBuffer, { Vec2(x2,y2), Vec2(cx2,cy2), fnt.color }); // V2
                PutBufferVertex(gDrawBuffer, { Vec2(x1,y1), Vec2(cx1,cy1), fnt.color }); // V1
                PutBufferVertex(gDrawBuffer, { Vec2(x2,y1), Vec2(cx2,cy1), fnt.color }); // V3

                cx += advance;
            } break;
        }
    }

    DrawVertexBuffer(gDrawBuffer, BufferMode::TRIANGLES);
    ClearVertexBuffer(gDrawBuffer);
}

TEINAPI void BeginDraw (BufferMode mode)
{
    gImmediateBufferDrawMode = mode;
}
TEINAPI void EndDraw ()
{
    glUseProgram(gUntexturedShader);
    DrawVertexBuffer(gDrawBuffer, gImmediateBufferDrawMode);
    ClearVertexBuffer(gDrawBuffer);
}

TEINAPI void PutVertex (float x, float y, Vec4 color)
{
    PutBufferVertex(gDrawBuffer, { Vec2(x,y), Vec2(0,0), color });
}

TEINAPI void PushMatrix (MatrixMode mode)
{
    glMatrixMode(static_cast<GLenum>(mode));
    glPushMatrix();
    glLoadIdentity();
}
TEINAPI void PopMatrix (MatrixMode mode)
{
    glMatrixMode(static_cast<GLenum>(mode));
    glPopMatrix();
}

TEINAPI void Translate (float x, float y)
{
    glTranslatef(x, y, 0);
}
TEINAPI void Rotate (float x, float y, float angle)
{
    glRotatef(angle, x, y, 0);
}
TEINAPI void Scale (float x, float y)
{
    glScalef(x, y, 1);
}

TEINAPI void SetTileBatchTexture (Texture& tex)
{
    gTileTexture = &tex;
}
TEINAPI void SetTextBatchFont (Font& fnt)
{
    gTextFont = &fnt;
}
TEINAPI void SetTileBatchColor (Vec4 color)
{
    gTileDrawColor = color;
}

TEINAPI void SetTextBatchColor (Vec4 color)
{
    gTextDrawColor = color;
}

TEINAPI void DrawBatchedTile (float x, float y, const Quad* clip)
{
    assert(gTileTexture);

    float cx1 =       (clip->x / gTileTexture->w);
    float cy1 =       (clip->y / gTileTexture->h);
    float cx2 = cx1 + (clip->w / gTileTexture->w);
    float cy2 = cy1 + (clip->h / gTileTexture->h);

    float w = clip->w * gTextureDrawScaleX;
    float h = clip->h * gTextureDrawScaleY;

    float x1 = x  - (w / 2); // Center anchor.
    float y1 = y  - (h / 2); // Center anchor.
    float x2 = x1 + w;
    float y2 = y1 + h;

    PutBufferVertex(gTileBuffer, { Vec2(x1,y2), Vec2(cx1,cy2), gTileDrawColor }); // V0
    PutBufferVertex(gTileBuffer, { Vec2(x1,y1), Vec2(cx1,cy1), gTileDrawColor }); // V1
    PutBufferVertex(gTileBuffer, { Vec2(x2,y2), Vec2(cx2,cy2), gTileDrawColor }); // V2
    PutBufferVertex(gTileBuffer, { Vec2(x2,y2), Vec2(cx2,cy2), gTileDrawColor }); // V2
    PutBufferVertex(gTileBuffer, { Vec2(x1,y1), Vec2(cx1,cy1), gTileDrawColor }); // V1
    PutBufferVertex(gTileBuffer, { Vec2(x2,y1), Vec2(cx2,cy1), gTileDrawColor }); // V3
}

TEINAPI void DrawBatchedText (float x, float y, std::string text)
{
    int index = 0;
    int prevIndex = 0;

    float cx = x;
    float cy = y;

    const Font& fnt = *gTextFont;
    float scale = gFontDrawScale;

    const auto& cache = fnt.cache.at(fnt.currentPointSize);
    const auto& glyphs = fnt.glyphs.at(fnt.currentPointSize);
    const auto& lineGap = fnt.lineGap.at(fnt.currentPointSize);

    for (const char* c=text.c_str(); *c; ++c)
    {
        if (*c < 0 || *c >= gTotalGlyphCount) continue;

        cx += (GetFontKerning(fnt, *c, index, prevIndex) * scale);

        switch (*c)
        {
            case ('\r'): cx = x;                               break;
            case ('\n'): cx = x, cy += (lineGap * scale);      break;
            case ('\t'): cx += (GetFontTabWidth(fnt) * scale); break;

            default:
            {
                const FontGlyph& glyph = glyphs.at(*c);
                const Quad& clip = glyph.bounds;

                float bearingX = glyph.bearing.x * scale;
                float bearingY = glyph.bearing.y * scale;

                float advance = glyph.advance * scale;

                float cx1 =       (clip.x / cache.w);
                float cy1 =       (clip.y / cache.h);
                float cx2 = cx1 + (clip.w / cache.w);
                float cy2 = cy1 + (clip.h / cache.h);

                float w = clip.w * scale;
                float h = clip.h * scale;

                float x1 = roundf(cx + bearingX);
                float y1 = roundf(cy - bearingY);
                float x2 = roundf(x1 + w);
                float y2 = roundf(y1 + h);

                PutBufferVertex(gTextBuffer, { Vec2(x1,y2), Vec2(cx1,cy2), gTextDrawColor }); // V0
                PutBufferVertex(gTextBuffer, { Vec2(x1,y1), Vec2(cx1,cy1), gTextDrawColor }); // V1
                PutBufferVertex(gTextBuffer, { Vec2(x2,y2), Vec2(cx2,cy2), gTextDrawColor }); // V2
                PutBufferVertex(gTextBuffer, { Vec2(x2,y2), Vec2(cx2,cy2), gTextDrawColor }); // V2
                PutBufferVertex(gTextBuffer, { Vec2(x1,y1), Vec2(cx1,cy1), gTextDrawColor }); // V1
                PutBufferVertex(gTextBuffer, { Vec2(x2,y1), Vec2(cx2,cy1), gTextDrawColor }); // V3

                cx += advance;
            } break;
        }
    }
}

TEINAPI void FlushBatchedTiles ()
{
    glBindTexture(GL_TEXTURE_2D, gTileTexture->handle);
    glEnable(GL_TEXTURE_2D);
    glUseProgram(gTexturedShader);
    DrawVertexBuffer(gTileBuffer, BufferMode::TRIANGLES);
    ClearVertexBuffer(gTileBuffer);
    glDisable(GL_TEXTURE_2D);
}

TEINAPI void FlushBatchedText ()
{
    glBindTexture(GL_TEXTURE_2D, gTextFont->cache.at(gTextFont->currentPointSize).handle);
    glEnable(GL_TEXTURE_2D);
    glUseProgram(gTextShader);
    DrawVertexBuffer(gTextBuffer, BufferMode::TRIANGLES);
    ClearVertexBuffer(gTextBuffer);
    glDisable(GL_TEXTURE_2D);
}
