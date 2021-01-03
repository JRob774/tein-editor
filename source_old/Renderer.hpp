enum class MatrixMode: GLenum { Projection = GL_PROJECTION, ModelView = GL_MODELVIEW };

struct Texture;
struct Font;

TEINAPI bool InitRenderer ();
TEINAPI void QuitRenderer ();

TEINAPI void RenderClear   (Vec4 clear);
TEINAPI void RenderPresent ();

TEINAPI Vec2 ScreenToWorld (Vec2 screen);
TEINAPI Vec2 WorldToScreen (Vec2 world);

TEINAPI float GetMaxTextureSize ();

TEINAPI Window* GetRenderTarget ();
TEINAPI void    SetRenderTarget (Window* window);

TEINAPI float GetRenderTargetWidth  ();
TEINAPI float GetRenderTargetHeight ();

TEINAPI void SetOrthographic (float l, float r, float b, float t);

TEINAPI void SetViewport (float x, float y, float w, float h);
TEINAPI void SetViewport (Quad v);
TEINAPI Quad GetViewport ();

TEINAPI void SetDrawColor (float r, float g, float b, float a = 1.0f);
TEINAPI void SetDrawColor (Vec4 color);

TEINAPI void SetLineWidth (float width);

TEINAPI void  SetTextureDrawScale  (float sx, float sy);
TEINAPI float GetTextureDrawScaleX ();
TEINAPI float GetTextureDrawScaleY ();

TEINAPI void  SetFontDrawScale (float scale);
TEINAPI float GetFontDrawScale ();

TEINAPI void BeginScissor (float x, float y, float w, float h);
TEINAPI void EndScissor   ();

TEINAPI void BeginStencil     ();
TEINAPI void EndStencil       ();
TEINAPI void StencilModeErase ();
TEINAPI void StencilModeDraw  ();

TEINAPI void DrawLine (float x1, float y1, float x2, float y2);
TEINAPI void DrawQuad (float x1, float y1, float x2, float y2);
TEINAPI void FillQuad (float x1, float y1, float x2, float y2);

TEINAPI void DrawTexture (const Texture& tex, float x, float y, const Quad* clip);
TEINAPI void DrawText    (const Font&    fnt, float x, float y, std::string text);

TEINAPI void BeginDraw (BufferMode mode);
TEINAPI void EndDraw   ();

TEINAPI void PutVertex (float x, float y, Vec4 color);

TEINAPI void PushMatrix (MatrixMode mode);
TEINAPI void PopMatrix  (MatrixMode mode);

TEINAPI void Translate (float x, float y);
TEINAPI void Rotate    (float x, float y, float angle);
TEINAPI void Scale     (float x, float y);

TEINAPI void SetTileBatchTexture (Texture& tex);
TEINAPI void SetTextBatchFont    (Font&    fnt);
TEINAPI void SetTileBatchColor   (Vec4 color);
TEINAPI void SetTextBatchColor   (Vec4 color);

TEINAPI void DrawBatchedTile (float x, float y, const Quad* clip);
TEINAPI void DrawBatchedText (float x, float y, std::string text);

TEINAPI void FlushBatchedTiles ();
TEINAPI void FlushBatchedText  ();
