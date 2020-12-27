/*******************************************************************************
 * Simplisitic 2D renderer implemented using OpenGL 3.0 as the backend.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

enum class Matrix_Mode: GLenum { PROJECTION = GL_PROJECTION, MODELVIEW = GL_MODELVIEW };

struct Texture; // Defined in <texture.hpp>
struct Font;    // Defined in <font.hpp>

/* -------------------------------------------------------------------------- */

TEINAPI bool init_renderer ();
TEINAPI void quit_renderer ();

TEINAPI void render_clear   (Vec4 clear);
TEINAPI void render_present ();

TEINAPI Vec2 screen_to_world (Vec2 screen);
TEINAPI Vec2 world_to_screen (Vec2 world);

TEINAPI float get_max_texture_size ();

TEINAPI Window* get_render_target ();
TEINAPI void    set_render_target (Window* window);

TEINAPI float get_render_target_w ();
TEINAPI float get_render_target_h ();

TEINAPI void set_orthographic (float l, float r, float b, float t);

TEINAPI void set_viewport (float x, float y, float w, float h);
TEINAPI void set_viewport (quad v);
TEINAPI quad get_viewport ();

TEINAPI void set_draw_color (float r, float g, float b, float a);
TEINAPI void set_draw_color (Vec4 color);

TEINAPI void set_line_width (float width);

TEINAPI void  set_texture_draw_scale   (float sx, float sy);
TEINAPI float get_texture_draw_scale_x ();
TEINAPI float get_texture_draw_scale_y ();

TEINAPI void  set_font_draw_scale (float scale);
TEINAPI float get_font_draw_scale ();

TEINAPI void begin_scissor (float x, float y, float w, float h);
TEINAPI void end_scissor   ();

TEINAPI void begin_stencil      ();
TEINAPI void end_stencil        ();
TEINAPI void stencil_mode_erase ();
TEINAPI void stencil_mode_draw  ();

TEINAPI void draw_line (float x1, float y1, float x2, float y2);
TEINAPI void draw_quad (float x1, float y1, float x2, float y2);
TEINAPI void fill_quad (float x1, float y1, float x2, float y2);

TEINAPI void draw_texture (const Texture& tex, float x, float y, const quad* clip);
TEINAPI void draw_text    (const Font&    fnt, float x, float y, std::string text);

TEINAPI void begin_draw (Buffer_Mode mode);
TEINAPI void end_draw   ();

TEINAPI void put_vertex (float x, float y, Vec4 color);

TEINAPI void push_matrix (Matrix_Mode mode);
TEINAPI void pop_matrix  (Matrix_Mode mode);

TEINAPI void translate (float x, float y);
TEINAPI void rotate    (float x, float y, float angle);
TEINAPI void scale     (float x, float y);

TEINAPI void set_tile_batch_texture (Texture& tex);
TEINAPI void set_text_batch_font    (Font&    fnt);
TEINAPI void set_tile_batch_color   (Vec4 color);
TEINAPI void set_text_batch_color   (Vec4 color);

TEINAPI void draw_batched_tile  (float x, float y, const quad* clip);
TEINAPI void draw_batched_text  (float x, float y, std::string text);

TEINAPI void flush_batched_tile ();
TEINAPI void flush_batched_text ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
