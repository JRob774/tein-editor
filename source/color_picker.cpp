/*******************************************************************************
 * Editor GUI widget that allows for picking a color using various sliders.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

enum class Channel_Type { INVALID, R, G, B, A };

static constexpr float COLOR_PICKER_CHANNEL_W =  40;
static constexpr float COLOR_PICKER_CHANNEL_H = 180;

static constexpr float COLOR_PICKER_XPAD = 8;
static constexpr float COLOR_PICKER_YPAD = 8;

static constexpr float COLOR_PICKER_TEXT_BOX_H = 20;
static constexpr float COLOR_PICKER_ALPHA_H    = 20;

static constexpr float COLOR_PICKER_BOTTOM_BORDER = 26.;

static constexpr float COLOR_PICKER_SWATCH_LG    = 32;
static constexpr float COLOR_PICKER_SWATCH_SM    = 16;
static constexpr float COLOR_PICKER_SWATCH_XPAD  =  7;
static constexpr float COLOR_PICKER_SWATCH_YPAD  =  8;
static constexpr float COLOR_PICKER_SWATCH_GAP   =  6;
static constexpr int   COLOR_PICKER_SWATCH_COUNT =  9;

static Channel_Type color_picker_active_channel;
static bool color_picker_mouse_pressed;

static std::deque<Vec4> color_picker_swatches;

// When the color picker menu is opened we cache the current color value.
// This allows the color picker menu to modify the current color immediately
// for instant feedback. If the user then wants to cancel the changes made,
// we can just restore the cached version of the color to turn it back.

static Vec4* current_color_picker_color;
static Vec4  cached_color_picker_color;

/* -------------------------------------------------------------------------- */

TEINAPI void internal__do_color_channel (Vec2& cursor, Vec4 min, Vec4 max, float& channel, Channel_Type type)
{
    float xpad = COLOR_PICKER_XPAD;
    float ypad = COLOR_PICKER_YPAD;

    float cw = COLOR_PICKER_CHANNEL_W;
    float ch = COLOR_PICKER_CHANNEL_H;

    float x1 = cursor.x;
    float y1 = cursor.y;
    float x2 = cursor.x + cw;
    float y2 = cursor.y + ch;

    // The dark and light border surrounding the channel quad.
    set_draw_color(ui_color_light);
    fill_quad(x1-2, y1-2, x2+2, y2+2);
    set_draw_color(ui_color_ex_dark);
    fill_quad(x1-1, y1-1, x2+1, y2+1);

    begin_draw(Buffer_Mode::TRIANGLE_STRIP);
    put_vertex(cursor.x   , cursor.y+ch, min); // BL
    put_vertex(cursor.x   , cursor.y   , max); // TL
    put_vertex(cursor.x+cw, cursor.y+ch, min); // BR
    put_vertex(cursor.x+cw, cursor.y   , max); // TR
    end_draw();

    float percent = std::clamp(roundf(channel*100), 0.0f, 100.0f);
    std::string channel_str(std::to_string(static_cast<int>(percent)));
    cursor.y += (ch + ypad);
    do_text_box(cw, COLOR_PICKER_TEXT_BOX_H, UI_NUMERIC, channel_str, "0");
    if (atoi(channel_str.c_str()) > 100) channel_str = "100";
    float new_channel = static_cast<float>(atoi(channel_str.c_str())) / 100;
    if (channel != new_channel) channel = new_channel;
    // Reset cursor after text box.
    cursor.x -=  cw;
    cursor.y -= (ch + ypad);

    // Draw the indicator for what value is currently selected.
    float pos = (cursor.y + ch) - roundf(ch * channel);

    float ix1 = cursor.x   -2;
    float iy1 = pos        -2;
    float ix2 = cursor.x+cw+2;
    float iy2 = pos        +2;

    Vec4 cur = min;
    switch (type)
    {
        case (Channel_Type::R): cur.r = channel; break;
        case (Channel_Type::G): cur.g = channel; break;
        case (Channel_Type::B): cur.b = channel; break;
    }

    set_draw_color(ui_color_light);
    fill_quad(ix1-2, iy1-2, ix2+2, iy2+2);
    set_draw_color(ui_color_ex_dark);
    fill_quad(ix1-1, iy1-1, ix2+1, iy2+1);
    set_draw_color(cur);
    fill_quad(ix1-0, iy1-0, ix2+0, iy2+0);

    // Handle the indicator both being selected, deselected, and moved.
    if (color_picker_mouse_pressed)
    {
        if (mouse_in_ui_bounds_xywh(ix1-1, iy1-1, cw+2, 7))
        {
            color_picker_active_channel = type;
        }
        else if (mouse_in_ui_bounds_xywh(cursor.x, cursor.y, cw, ch))
        {
            color_picker_active_channel = type;
            // Determine where the click was and jump to that position.
            float my = ch - (get_mouse_pos().y - get_viewport().y - cursor.y);
            channel = my / ch;
        }
    }
    if (color_picker_active_channel == type)
    {
        channel -= (ui_get_relative_mouse().y / ch);
        channel = std::clamp(channel, 0.0f, 1.0f);
    }

    // Move the cursor into position for next channel.
    cursor.x += cw + (xpad*3);
}

TEINAPI void internal__do_color_preview (Vec2& cursor, Vec4 c, float size)
{
    const Texture& tex = (static_cast<int>(size) % 14 == 0) ? resource_checker_14 : resource_checker_16;

    float x = cursor.x;
    float y = cursor.y;

    // The dark and light border surrounding the color.
    set_draw_color(ui_color_light);
    fill_quad(x-2, y-2, x+size+2, y+size+2);
    set_draw_color(ui_color_ex_dark);
    fill_quad(x-1, y-1, x+size+1, y+size+1);

    float tx = x + (size / 2);
    float ty = y + (size / 2);

    quad clip = { 0, 0, size, size };
    draw_texture(tex, tx, ty, &clip);

    Vec4 max(c.r, c.g, c.b,   1);
    Vec4 min(c.r, c.g, c.b, c.a);

    begin_draw(Buffer_Mode::TRIANGLE_STRIP);
    put_vertex(x     , y+size, min); // BL
    put_vertex(x     , y     , max); // TL
    put_vertex(x+size, y+size, min); // BR
    put_vertex(x+size, y     , max); // TR
    end_draw();
}

TEINAPI void internal__do_swatch_panel (Vec2& cursor)
{
    assert(current_color_picker_color);

    cursor.y += COLOR_PICKER_SWATCH_LG + COLOR_PICKER_YPAD;

    set_draw_color(ui_color_med_dark);
    draw_quad(cursor.x, cursor.y, cursor.x+COLOR_PICKER_SWATCH_LG, get_panel_h()-COLOR_PICKER_YPAD);

    float x = cursor.x                                  +1;
    float y = cursor.y                                  +1;
    float w = COLOR_PICKER_SWATCH_LG                    -2;
    float h = (get_panel_h()-COLOR_PICKER_YPAD)-cursor.y-2;

    Vec2 cursor2(COLOR_PICKER_SWATCH_XPAD, COLOR_PICKER_SWATCH_YPAD);
    begin_panel(x, y, w, h, UI_NONE);

    for (auto& c: color_picker_swatches)
    {
        internal__do_color_preview(cursor2, c, COLOR_PICKER_SWATCH_SM);
        if (color_picker_mouse_pressed)
        {
            if (mouse_in_ui_bounds_xywh(cursor2.x, cursor2.y, COLOR_PICKER_SWATCH_SM, COLOR_PICKER_SWATCH_SM))
            {
                *current_color_picker_color = c;
            }
        }

        cursor2.y += COLOR_PICKER_SWATCH_SM;
        cursor2.y += COLOR_PICKER_SWATCH_GAP;
    }

    end_panel();
}

TEINAPI void internal__do_alpha_channel (Vec2& cursor, Vec4& c)
{
    float xpad = COLOR_PICKER_XPAD;
    float ypad = COLOR_PICKER_YPAD;

    float cw = COLOR_PICKER_CHANNEL_W;
    float ch = COLOR_PICKER_CHANNEL_H;

    cursor.x =  xpad;
    cursor.y = (ypad*3.25f) + ch + COLOR_PICKER_TEXT_BOX_H;

    set_panel_cursor_dir(UI_DIR_DOWN);
    do_separator((cw*3)+(xpad*6));
    cursor.y -= 1;
    set_panel_cursor_dir(UI_DIR_RIGHT);

    cursor.y += (ypad*1.25f);

    float percent = std::clamp(roundf(c.a*100), 0.0f, 100.0f);
    std::string alpha_str(std::to_string(static_cast<int>(percent)));
    do_text_box(cw, COLOR_PICKER_TEXT_BOX_H, UI_NUMERIC, alpha_str, "0");
    if (atoi(alpha_str.c_str()) > 100) alpha_str = "100";
    float new_alpha = static_cast<float>(atoi(alpha_str.c_str())) / 100;
    if (c.a != new_alpha) c.a = new_alpha;

    cursor.x += xpad;

    float x1 = cursor.x;
    float y1 = cursor.y;
    float x2 = cursor.x + ((cw*2)+(xpad*5));
    float y2 = cursor.y + COLOR_PICKER_ALPHA_H;

    set_draw_color(ui_color_light);
    fill_quad(x1-2, y1-2, x2+2, y2+2);
    set_draw_color(ui_color_ex_dark);
    fill_quad(x1-1, y1-1, x2+1, y2+1);

    float tw = x2-x1;
    float th = COLOR_PICKER_ALPHA_H;
    float tx = cursor.x + (tw / 2);
    float ty = cursor.y + (th / 2);

    quad clip1 = { 0, 0, tw, th };
    draw_texture(resource_checker_20, tx, ty, &clip1);

    Vec4 min(c.r, c.g, c.b, 0);
    Vec4 max(c.r, c.g, c.b, 1);

    begin_draw(Buffer_Mode::TRIANGLE_STRIP);
    put_vertex(x1, y2, min); // BL
    put_vertex(x1, y1, min); // TL
    put_vertex(x2, y2, max); // BR
    put_vertex(x2, y1, max); // TR
    end_draw();

    // Draw the indicator for what value is currently selected.
    float pos = (cursor.x + tw) - roundf(tw * (1-c.a));

    float ix1 = pos        -2;
    float iy1 = cursor.y   -2;
    float ix2 = pos        +2;
    float iy2 = cursor.y+th+2;

    set_draw_color(ui_color_light);
    fill_quad(ix1-2, iy1-2, ix2+2, iy2+2);
    set_draw_color(ui_color_ex_dark);
    fill_quad(ix1-1, iy1-1, ix2+1, iy2+1);

    float itw = ix2-ix1;
    float ith = iy2-iy1;
    float itx = ix1 + (itw / 2);
    float ity = iy1 + (ith / 2);

    quad clip2 = { ix1-cursor.x, -2, itw, ith };
    draw_texture(resource_checker_20, itx, ity, &clip2);

    set_draw_color(c);
    fill_quad(ix1, iy1, ix2, iy2);

    // Handle the indicator both being selected, deselected, and moved.
    if (color_picker_mouse_pressed)
    {
        if (mouse_in_ui_bounds_xywh(ix1-1, iy1-1, 7, th+2))
        {
            color_picker_active_channel = Channel_Type::A;
        }
        else if (mouse_in_ui_bounds_xywh(cursor.x, cursor.y, tw, th))
        {
            color_picker_active_channel = Channel_Type::A;
            // Determine where the click was and jump to that position.
            float mx = tw - (get_mouse_pos().x - get_viewport().x - cursor.x);
            c.a = 1 - (mx / tw);
        }
    }
    if (color_picker_active_channel == Channel_Type::A)
    {
        c.a += (ui_get_relative_mouse().x / tw);
        c.a = std::clamp(c.a, 0.0f, 1.0f);
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void internal__okay_color ()
{
    // Special checks for background and tile grid color because of the defaulting.
    if (current_color_picker_color == &editor_settings.background_color)
    {
        editor_settings.background_color_defaulted = false;
    }
    if (current_color_picker_color == &editor_settings.tile_grid_color)
    {
        editor_settings.tile_grid_color_defaulted = false;
    }

    hide_window("WINCOLOR");
}

TEINAPI void internal__save_color_swatch ()
{
    assert(current_color_picker_color);

    color_picker_swatches.pop_front();
    color_picker_swatches.push_back(*current_color_picker_color);
}

TEINAPI void internal__cancel_color ()
{
    assert(current_color_picker_color);
    *current_color_picker_color = cached_color_picker_color;

    hide_window("WINCOLOR");
}

/* -------------------------------------------------------------------------- */

TEINAPI void init_color_picker ()
{
    current_color_picker_color = NULL;
    // Add some empty swatches to the color picker on start-up.
    for (int i=0; i<COLOR_PICKER_SWATCH_COUNT; ++i)
    {
        color_picker_swatches.push_back(Vec4(1,1,1,0));
    }
}

TEINAPI void open_color_picker (Vec4* color)
{
    raise_window("WINCOLOR");

    color_picker_active_channel = Channel_Type::INVALID;
    color_picker_mouse_pressed = false;

    assert(color);

    current_color_picker_color = color;
    cached_color_picker_color = *color;

    if (is_window_hidden("WINCOLOR"))
    {
        show_window("WINCOLOR");
    }
}

TEINAPI void do_color_picker ()
{
    if (is_window_hidden("WINCOLOR")) return;

    quad p1, p2;

    p1.x = WINDOW_BORDER;
    p1.y = WINDOW_BORDER;
    p1.w = get_viewport().w - (WINDOW_BORDER * 2);
    p1.h = get_viewport().h - (WINDOW_BORDER * 2);

    set_ui_font(&get_editor_regular_font());

    begin_panel(p1, UI_NONE, ui_color_ex_dark);

    float bb = COLOR_PICKER_BOTTOM_BORDER;

    float vw = get_viewport().w;
    float vh = get_viewport().h;

    float bw = roundf(vw / 3);
    float bh = bb - WINDOW_BORDER;

    // Bottom buttons for okaying or cancelling the color picker.
    Vec2 btn_cursor(0, WINDOW_BORDER);
    begin_panel(0, vh-bb, vw, bb, UI_NONE, ui_color_medium);

    set_panel_cursor_dir(UI_DIR_RIGHT);
    set_panel_cursor(&btn_cursor);

    // Just to make sure that we always reach the end of the panel space.
    float bw2 = vw - (bw*2);

    if (do_button_txt(NULL, bw ,bh, UI_NONE, "Okay"  )) internal__okay_color();
    if (do_button_txt(NULL, bw ,bh, UI_NONE, "Save"  )) internal__save_color_swatch();
    if (do_button_txt(NULL, bw2,bh, UI_NONE, "Cancel")) internal__cancel_color();

    // Add a separator to the left for symmetry.
    btn_cursor.x = 1;
    do_separator(bh);

    end_panel();

    p2.x =                  1;
    p2.y =                  1;
    p2.w = vw             - 2;
    p2.h = vh - p2.y - bb - 1;

    begin_panel(p2, UI_NONE, ui_color_medium);

    assert(current_color_picker_color);

    Vec4& c = *current_color_picker_color; // So much shorter to type out...
    Vec2 cursor(COLOR_PICKER_XPAD, COLOR_PICKER_YPAD);

    set_panel_cursor_dir(UI_DIR_RIGHT);
    set_panel_cursor(&cursor);

    // Draw the three R G B channel selectors/sliders.
    Vec4 r_min(  0, c.g, c.b, 1);
    Vec4 r_max(  1, c.g, c.b, 1);
    Vec4 g_min(c.r,   0, c.b, 1);
    Vec4 g_max(c.r,   1, c.b, 1);
    Vec4 b_min(c.r, c.g,   0, 1);
    Vec4 b_max(c.r, c.g,   1, 1);

    internal__do_color_channel(cursor, r_min, r_max, c.r, Channel_Type::R);
    internal__do_color_channel(cursor, g_min, g_max, c.g, Channel_Type::G);
    internal__do_color_channel(cursor, b_min, b_max, c.b, Channel_Type::B);

    // Draw the current color to showcase to the user.
    internal__do_color_preview(cursor, c, COLOR_PICKER_SWATCH_LG);

    // Draw box surrounding the saved swatches.
    internal__do_swatch_panel(cursor);

    // Draw the alpha/opacity text box section.
    internal__do_alpha_channel(cursor, c);

    end_panel();
    end_panel();
}

TEINAPI void cancel_color_picker ()
{
    internal__cancel_color();
}

/* -------------------------------------------------------------------------- */

TEINAPI void handle_color_picker_events ()
{
    color_picker_mouse_pressed = false;

    if (!is_window_focused("WINCOLOR")) return;

    // Determine if the mouse was pressed this update/cycle.
    switch (main_event.type)
    {
        case (SDL_MOUSEBUTTONDOWN):
        {
            if (main_event.button.button == SDL_BUTTON_LEFT)
            {
                color_picker_mouse_pressed = true;
            }
        } break;
        case (SDL_MOUSEBUTTONUP):
        {
            if (main_event.button.button == SDL_BUTTON_LEFT)
            {
                color_picker_active_channel = Channel_Type::INVALID;
            }
        } break;
        case (SDL_KEYDOWN):
        {
            if (!text_box_is_active())
            {
                switch (main_event.key.keysym.sym)
                {
                    case (SDLK_RETURN): internal__okay_color();   break;
                    case (SDLK_ESCAPE): internal__cancel_color(); break;
                }
            }
        } break;
    }
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
