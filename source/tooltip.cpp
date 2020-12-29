/*******************************************************************************
 * System for tooltips that appear when hovering on certain GUI elements.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr size_t TOOLTIP_MAX_LINE_LENGTH =  96;
static constexpr float  TOOLTIP_APPEAR_TIME     = .5f; // Seconds

static std::string tooltip_name;
static std::string tooltip_desc;

static bool tooltip_set_this_update;
static bool tooltip_visible;

static SDL_TimerID tooltip_timer;

/* -------------------------------------------------------------------------- */

TEINAPI U32 internal__tooltip_callback (U32 interval, void* user_data)
{
    PushEditorEvent(EDITOR_EVENT_SHOW_TOOLTIP, NULL, NULL);
    return 0;
}

TEINAPI void internal__reset_tooltip ()
{
    tooltip_name = "";
    tooltip_desc = "";

    tooltip_visible = false;
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_current_tooltip (std::string name, std::string desc)
{
    // This is how we know we are still hovered over the tooltip item and don't need to hide it.
    if (tooltip_name == name && tooltip_desc == desc)
    {
        tooltip_set_this_update = true;
        return;
    }
    if (tooltip_timer)
    {
        SDL_RemoveTimer(tooltip_timer);
    }

    internal__reset_tooltip();

    tooltip_name = name;
    tooltip_desc = desc;

    tooltip_timer = SDL_AddTimer(static_cast<U32>(TOOLTIP_APPEAR_TIME*1000), internal__tooltip_callback, NULL);
    if (!tooltip_timer)
    {
        LogError(ERR_MIN, "Failed to setup tooltip timer! (%s)", SDL_GetError());
    }

    tooltip_set_this_update = true;
}

TEINAPI void do_tooltip ()
{
    if (!gEditorSettings.showTooltips) return;

    if (!tooltip_set_this_update)
    {
        internal__reset_tooltip();
    }
    if (tooltip_visible)
    {
        // Word wrap the description if it is too large for the tooltip box.
        std::string desc = tooltip_desc;
        size_t max_width = std::max<size_t>(TOOLTIP_MAX_LINE_LENGTH, tooltip_name.length());
        int linecount = 1;
        if (max_width < desc.length())
        {
            size_t offset = 0;
            for (size_t pos=0; pos!=std::string::npos; pos=desc.find(' ', pos))
            {
                if (max_width <= (pos - offset))
                {
                    desc.at(pos) = '\n';
                    offset = pos;
                    linecount++;
                }
                ++pos;
            }
        }

        Font& fnt = get_editor_regular_font();
        set_ui_font(&fnt);

        constexpr float XPAD = 4;
        constexpr float YPAD = 8;

        float xpad = XPAD;
        float ypad = YPAD;

        if (desc.empty()) ypad /= 2; // Looks nicer!

        float nw = get_text_width_scaled (fnt, tooltip_name);
        float nh = get_text_height_scaled(fnt, tooltip_name);
        float dw = get_text_width_scaled (fnt, desc        );
        float dh = get_text_height_scaled(fnt, desc        );

        Vec2 mouse = GetMousePos();

        float tx = mouse.x;
        float ty = mouse.y + 16; // Some constant just so it doesn't get covered by the mouse.
        float tw = std::max<float>(nw,dw) + (xpad*2);
        float th = (nh+dh)                + (ypad*2);

        // Make sure that the tooltip is always on-screen and not out-of-bounds.
        if (tx+tw >= GetRenderTargetWidth()) tx = mouse.x - tw;
        if (ty+th >= GetRenderTargetHeight()) ty = mouse.y - th;

        begin_panel(tx, ty, tw, th, UI_NONE, Vec4(0,0,0,.8f));

        Vec2 cursor(xpad, ypad);

        set_panel_cursor(&cursor);
        set_panel_cursor_dir(UI_DIR_DOWN);

        // The set panel flags are just a hack to get the text drawing nicely in the tooltip box..
        set_panel_flags(UI_TOOLTIP);
        do_label(UI_ALIGN_LEFT,UI_ALIGN_CENTER, tw, nh, tooltip_name);
        set_panel_flags(UI_TOOLTIP|UI_DARKEN);
        do_label(UI_ALIGN_LEFT,UI_ALIGN_TOP, tw, dh+ypad, desc);

        end_panel();
    }
}

TEINAPI void handle_tooltip_events ()
{
    tooltip_set_this_update = false;

    if (main_event.type == SDL_USEREVENT)
    {
        if (main_event.user.code == EDITOR_EVENT_SHOW_TOOLTIP)
        {
            tooltip_visible = true;
        }
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
