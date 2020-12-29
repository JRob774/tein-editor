/*******************************************************************************
 * Dialog box that opens up when the user wants to create a new level/map.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float RESIZE_BOTTOM_BORDER = 26;

static constexpr float RESIZE_XPAD = 8;
static constexpr float RESIZE_YPAD = 8;

static constexpr float RESIZE_TEXT_BOX_H = 20;

static constexpr const char* RESIZE_WIDTH_LABEL = "Level Width:  ";
static constexpr const char* RESIZE_HEIGHT_LABEL = "Level Height:  ";

static int current_resize_width = static_cast<int>(DEFAULT_LEVEL_WIDTH);
static int current_resize_height = static_cast<int>(DEFAULT_LEVEL_HEIGHT);

static Resize_Dir resize_dialog_dir = Resize_Dir::CENTER;

static const Quad* nw_clip     = &CLIP_RESIZE_NW;
static const Quad* n_clip      = &CLIP_RESIZE_N;
static const Quad* ne_clip     = &CLIP_RESIZE_NE;
static const Quad* w_clip      = &CLIP_RESIZE_W;
static const Quad* center_clip = &CLIP_BULLET;
static const Quad* e_clip      = &CLIP_RESIZE_E;
static const Quad* sw_clip     = &CLIP_RESIZE_SW;
static const Quad* s_clip      = &CLIP_RESIZE_S;
static const Quad* se_clip     = &CLIP_RESIZE_SE;

/* -------------------------------------------------------------------------- */

TEINAPI void internal__calculate_dir_clips ()
{
    switch (resize_dialog_dir)
    {
        case (Resize_Dir::NW):
        {
            nw_clip     = &CLIP_BULLET;
            n_clip      = &CLIP_RESIZE_E;
            ne_clip     = &CLIP_NONE;
            w_clip      = &CLIP_RESIZE_S;
            center_clip = &CLIP_RESIZE_SE;
            e_clip      = &CLIP_NONE;
            sw_clip     = &CLIP_NONE;
            s_clip      = &CLIP_NONE;
            se_clip     = &CLIP_NONE;
        } break;
        case (Resize_Dir::N):
        {
            nw_clip     = &CLIP_RESIZE_W;
            n_clip      = &CLIP_BULLET;
            ne_clip     = &CLIP_RESIZE_E;
            w_clip      = &CLIP_RESIZE_SW;
            center_clip = &CLIP_RESIZE_S;
            e_clip      = &CLIP_RESIZE_SE;
            sw_clip     = &CLIP_NONE;
            s_clip      = &CLIP_NONE;
            se_clip     = &CLIP_NONE;
        } break;
        case (Resize_Dir::NE):
        {
            nw_clip     = &CLIP_NONE;
            n_clip      = &CLIP_RESIZE_W;
            ne_clip     = &CLIP_BULLET;
            w_clip      = &CLIP_NONE;
            center_clip = &CLIP_RESIZE_SW;
            e_clip      = &CLIP_RESIZE_S;
            sw_clip     = &CLIP_NONE;
            s_clip      = &CLIP_NONE;
            se_clip     = &CLIP_NONE;
        } break;
        case (Resize_Dir::W):
        {
            nw_clip     = &CLIP_RESIZE_N;
            n_clip      = &CLIP_RESIZE_NE;
            ne_clip     = &CLIP_NONE;
            w_clip      = &CLIP_BULLET;
            center_clip = &CLIP_RESIZE_E;
            e_clip      = &CLIP_NONE;
            sw_clip     = &CLIP_RESIZE_S;
            s_clip      = &CLIP_RESIZE_SE;
            se_clip     = &CLIP_NONE;
        } break;
        case (Resize_Dir::CENTER):
        {
            nw_clip     = &CLIP_RESIZE_NW;
            n_clip      = &CLIP_RESIZE_N;
            ne_clip     = &CLIP_RESIZE_NE;
            w_clip      = &CLIP_RESIZE_W;
            center_clip = &CLIP_BULLET;
            e_clip      = &CLIP_RESIZE_E;
            sw_clip     = &CLIP_RESIZE_SW;
            s_clip      = &CLIP_RESIZE_S;
            se_clip     = &CLIP_RESIZE_SE;
        } break;
        case (Resize_Dir::E):
        {
            nw_clip     = &CLIP_NONE;
            n_clip      = &CLIP_RESIZE_NW;
            ne_clip     = &CLIP_RESIZE_N;
            w_clip      = &CLIP_NONE;
            center_clip = &CLIP_RESIZE_W;
            e_clip      = &CLIP_BULLET;
            sw_clip     = &CLIP_NONE;
            s_clip      = &CLIP_RESIZE_SW;
            se_clip     = &CLIP_RESIZE_S;
        } break;
        case (Resize_Dir::SW):
        {
            nw_clip     = &CLIP_NONE;
            n_clip      = &CLIP_NONE;
            ne_clip     = &CLIP_NONE;
            w_clip      = &CLIP_RESIZE_N;
            center_clip = &CLIP_RESIZE_NE;
            e_clip      = &CLIP_NONE;
            sw_clip     = &CLIP_BULLET;
            s_clip      = &CLIP_RESIZE_E;
            se_clip     = &CLIP_NONE;
        } break;
        case (Resize_Dir::S):
        {
            nw_clip     = &CLIP_NONE;
            n_clip      = &CLIP_NONE;
            ne_clip     = &CLIP_NONE;
            w_clip      = &CLIP_RESIZE_NW;
            center_clip = &CLIP_RESIZE_N;
            e_clip      = &CLIP_RESIZE_NE;
            sw_clip     = &CLIP_RESIZE_W;
            s_clip      = &CLIP_BULLET;
            se_clip     = &CLIP_RESIZE_E;
        } break;
        case (Resize_Dir::SE):
        {
            nw_clip     = &CLIP_NONE;
            n_clip      = &CLIP_NONE;
            ne_clip     = &CLIP_NONE;
            w_clip      = &CLIP_NONE;
            center_clip = &CLIP_RESIZE_NW;
            e_clip      = &CLIP_RESIZE_N;
            sw_clip     = &CLIP_NONE;
            s_clip      = &CLIP_RESIZE_W;
            se_clip     = &CLIP_BULLET;
        } break;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void internal__do_resize_alignment (Vec2& cursor)
{
    // Do the long horizontal separator first.
    float w = GetViewport().w - (RESIZE_XPAD*2);

    advance_panel_cursor(RESIZE_YPAD*1.5f);
    do_separator(w);
    advance_panel_cursor(RESIZE_YPAD*1.5f);

    set_panel_cursor_dir(UI_DIR_RIGHT);

    float bw = 25;
    float bh = 25;

    float x = (GetViewport().w/2) - ((bw*3)/2);

    cursor.x  = x;
    cursor.y -= 2; // Just to get the spacing above and below even.

    float qx1 = cursor.x            - 1;
    float qy1 = cursor.y            - 1;
    float qx2 = cursor.x + (bw * 3) + 1;
    float qy2 = cursor.y + (bh * 3) + 1;

    SetDrawColor(ui_color_ex_dark);
    FillQuad(qx1, qy1, qx2, qy2);

    Resize_Dir old_dir = resize_dialog_dir;

    if (do_button_img(NULL, bw,bh, UI_NONE, nw_clip    )) resize_dialog_dir = Resize_Dir::NW;
    if (do_button_img(NULL, bw,bh, UI_NONE, n_clip     )) resize_dialog_dir = Resize_Dir::N;
    if (do_button_img(NULL, bw,bh, UI_NONE, ne_clip    )) resize_dialog_dir = Resize_Dir::NE;

    cursor.x  = x;
    cursor.y += bh;

    if (do_button_img(NULL, bw,bh, UI_NONE, w_clip     )) resize_dialog_dir = Resize_Dir::W;
    if (do_button_img(NULL, bw,bh, UI_NONE, center_clip)) resize_dialog_dir = Resize_Dir::CENTER;
    if (do_button_img(NULL, bw,bh, UI_NONE, e_clip     )) resize_dialog_dir = Resize_Dir::E;

    cursor.x  = x;
    cursor.y += bh;

    if (do_button_img(NULL, bw,bh, UI_NONE, sw_clip    )) resize_dialog_dir = Resize_Dir::SW;
    if (do_button_img(NULL, bw,bh, UI_NONE, s_clip     )) resize_dialog_dir = Resize_Dir::S;
    if (do_button_img(NULL, bw,bh, UI_NONE, se_clip    )) resize_dialog_dir = Resize_Dir::SE;

    if (old_dir != resize_dialog_dir)
    {
        internal__calculate_dir_clips();
    }
}

TEINAPI void internal__okay_resize ()
{
    if (current_resize_width < MINIMUM_LEVEL_WIDTH || current_resize_height < MINIMUM_LEVEL_HEIGHT)
    {
        ShowAlert("Warning", FormatString("Minimum level size is %dx%d!", MINIMUM_LEVEL_WIDTH, MINIMUM_LEVEL_HEIGHT), ALERT_TYPE_WARNING, ALERT_BUTTON_OK, "WINRESIZE");
        return;
    }

    le_resize_okay();
    HideWindow("WINRESIZE");
}

/* -------------------------------------------------------------------------- */

TEINAPI void open_resize (int lw, int lh)
{
    SetWindowPos("WINRESIZE", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED);

    if (lw > 0) current_resize_width = lw;
    if (lh > 0) current_resize_height = lh;

    resize_dialog_dir = Resize_Dir::CENTER;
    internal__calculate_dir_clips();

    ShowWindow("WINRESIZE");
}

TEINAPI void do_resize ()
{
    Quad p1, p2;

    p1.x = gWindowBorder;
    p1.y = gWindowBorder;
    p1.w = GetViewport().w - (gWindowBorder * 2);
    p1.h = GetViewport().h - (gWindowBorder * 2);

    set_ui_font(&get_editor_regular_font());

    begin_panel(p1, UI_NONE, ui_color_ex_dark);

    float bb = RESIZE_BOTTOM_BORDER;

    float vw = GetViewport().w;
    float vh = GetViewport().h;

    float bw = roundf(vw / 2);
    float bh = bb - gWindowBorder;

    // Bottom buttons for okaying or cancelling the resize.
    Vec2 btn_cursor(0, gWindowBorder);
    begin_panel(0, vh-bb, vw, bb, UI_NONE, ui_color_medium);

    set_panel_cursor_dir(UI_DIR_RIGHT);
    set_panel_cursor(&btn_cursor);

    // Just to make sure that we always reach the end of the panel space.
    float bw2 = vw - bw;

    if (do_button_txt(NULL, bw ,bh, UI_NONE, "Resize")) internal__okay_resize();
    if (do_button_txt(NULL, bw2,bh, UI_NONE, "Cancel")) cancel_resize();

    // Add a separator to the left for symmetry.
    btn_cursor.x = 1;
    do_separator(bh);

    end_panel();

    p2.x =                  1;
    p2.y =                  1;
    p2.w = vw             - 2;
    p2.h = vh - p2.y - bb - 1;

    begin_panel(p2, UI_NONE, ui_color_medium);

    Vec2 cursor(RESIZE_XPAD, RESIZE_YPAD);

    set_panel_cursor_dir(UI_DIR_DOWN);
    set_panel_cursor(&cursor);

    float label_w_w = GetTextWidthScaled(get_editor_regular_font(), RESIZE_WIDTH_LABEL);
    float label_h_w = GetTextWidthScaled(get_editor_regular_font(), RESIZE_HEIGHT_LABEL);

    float text_box_w = (vw-(RESIZE_XPAD*2));
    float label_w = std::max(label_w_w, label_h_w);

    std::string w_str(std::to_string(current_resize_width));
    std::string h_str(std::to_string(current_resize_height));

    do_text_box_labeled(text_box_w, RESIZE_TEXT_BOX_H, UI_NUMERIC, w_str, label_w, RESIZE_WIDTH_LABEL, "0");
    advance_panel_cursor(RESIZE_YPAD);
    do_text_box_labeled(text_box_w, RESIZE_TEXT_BOX_H, UI_NUMERIC, h_str, label_w, RESIZE_HEIGHT_LABEL, "0");

    if (atoi(w_str.c_str()) > MAXIMUM_LEVEL_WIDTH)
    {
        w_str = std::to_string(MAXIMUM_LEVEL_WIDTH);
    }
    if (atoi(h_str.c_str()) > MAXIMUM_LEVEL_HEIGHT)
    {
        h_str = std::to_string(MAXIMUM_LEVEL_HEIGHT);
    }

    int old_resize_width = current_resize_width;
    int new_resize_width = atoi(w_str.c_str());
    if (new_resize_width != old_resize_width)
    {
        current_resize_width = new_resize_width;
    }
    int old_resize_height = current_resize_height;
    int new_resize_height = atoi(h_str.c_str());
    if (new_resize_height != old_resize_height)
    {
        current_resize_height = new_resize_height;
    }

    internal__do_resize_alignment(cursor);

    end_panel();
    end_panel();
}

TEINAPI void cancel_resize ()
{
    HideWindow("WINRESIZE");
}

/* -------------------------------------------------------------------------- */

TEINAPI void handle_resize_events ()
{
    if (!IsWindowFocused("WINRESIZE")) return;

    if (!text_box_is_active())
    {
        if (main_event.type == SDL_KEYDOWN)
        {
            switch (main_event.key.keysym.sym)
            {
                case (SDLK_RETURN): internal__okay_resize(); break;
                case (SDLK_ESCAPE): cancel_resize();         break;
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI int get_resize_w ()
{
    return current_resize_width;
}

TEINAPI int get_resize_h ()
{
    return current_resize_height;
}

/* -------------------------------------------------------------------------- */

TEINAPI Resize_Dir get_resize_dir ()
{
    return resize_dialog_dir;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool resize_dir_is_north (Resize_Dir dir)
{
    return (dir == Resize_Dir::NW || dir == Resize_Dir::N || dir == Resize_Dir::NE);
}

TEINAPI bool resize_dir_is_east (Resize_Dir dir)
{
    return (dir == Resize_Dir::NE || dir == Resize_Dir::E || dir == Resize_Dir::SE);
}

TEINAPI bool resize_dir_is_south (Resize_Dir dir)
{
    return (dir == Resize_Dir::SW || dir == Resize_Dir::S || dir == Resize_Dir::SE);
}

TEINAPI bool resize_dir_is_west (Resize_Dir dir)
{
    return (dir == Resize_Dir::NW || dir == Resize_Dir::W || dir == Resize_Dir::SW);
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
