/*******************************************************************************
 * Editor GUI widget that shows information on the current hovered element.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr const char* STATUS_BAR_DEFAULT_MSG = "Hover over items and widgets for more information.";
static constexpr float       STATUS_BAR_LABEL_WIDTH = .125f; // NDC!

static std::string status_bar_pushed_message;

/* -------------------------------------------------------------------------- */

TEINAPI void push_status_bar_message (const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    status_bar_pushed_message = FormatStringV(fmt, args);
    va_end(args);
}

TEINAPI void do_status_bar ()
{
    // If there is a message that has been pushed we display that, if
    // not then we just display a default message for the status bar.
    std::string message = STATUS_BAR_DEFAULT_MSG;
    if (!status_bar_pushed_message.empty())
    {
        message = status_bar_pushed_message;
    }

    Vec2 cursor(STATUS_BAR_INNER_PAD, 0);

    float x = get_toolbar_w() + 1;
    float y = get_viewport().h - STATUS_BAR_HEIGHT;
    float w = get_viewport().w - get_toolbar_w() - (get_control_panel_w()) - 2;
    float h = STATUS_BAR_HEIGHT;

    // To account for the control panel and toolbar disappearing.
    if (!current_tab_is_level())         w += 1;
    if (!are_there_any_tabs  ()) x -= 1, w += 1;

    float status_bar_width = w - (STATUS_BAR_INNER_PAD * 2);
    float advance = STATUS_BAR_INNER_PAD;

    float l2_w = roundf(status_bar_width * STATUS_BAR_LABEL_WIDTH); // Mouse.
    float l3_w = roundf(status_bar_width * STATUS_BAR_LABEL_WIDTH); // Select.

    // Get the mouse position.
    int mx = 0, my = 0;
    if (IsWindowFocused("WINMAIN"))
    {
        if (current_tab_is_level())
        {
            if (PointInBoundsXYWH(GetMousePos(), level_editor.viewport))
            {
                mx = static_cast<int>(level_editor.mouse_tile.x);
                my = static_cast<int>(level_editor.mouse_tile.y);
            }
        }
        else if (current_tab_is_map())
        {
            if (PointInBoundsXYWH(GetMousePos(), map_editor.viewport))
            {
                mx = static_cast<int>(map_editor.mouse_tile.x);
                my = static_cast<int>(map_editor.mouse_tile.y);
            }
        }
    }

    // Get the select bounds.
    int sx = 0, sy = 0, sw = 0, sh = 0;
    if (current_tab_is_level() && are_any_select_boxes_visible())
    {
        int l,t,r,b;
        get_total_select_boundary(&l,&t,&r,&b);
        sx = l, sy = b, sw = (r-l)+1, sh = (t-b)+1;
    }
    else if (current_tab_is_map() && map_select_box_present())
    {
        int l,t,r,b;
        get_map_select_bounds(&l,&t,&r,&b);
        sx = l, sy = b, sw = (r-l)+1, sh = (t-b)+1;
    }

    std::string mouse_str = FormatString("Position (%d,%d)", mx,my);
    std::string select_str = FormatString("Selection (%d,%d,%d,%d)", sx,sy,sw,sh);

    // We ensure that the mouse and select labels are always big enough to
    // show their entire content and they take priority over the tool-tip.
    float l2_tw = get_text_width_scaled(get_editor_regular_font(), mouse_str);
    if (l2_w < l2_tw) l2_w = l2_tw;
    float l3_tw = get_text_width_scaled(get_editor_regular_font(), select_str);
    if (l3_w < l3_tw) l3_w = l3_tw;

    // Now we can calculate how much space is left for the tool-tip label.
    float l1_w = (status_bar_width - (l2_w + l3_w)) - (advance * 2);

    set_ui_font(&get_editor_regular_font());

    // Status bar is a horizontal list of editor status information.
    begin_panel(x, y, w, h, UI_NONE, ui_color_medium);

    set_panel_cursor(&cursor);
    set_panel_cursor_dir(UI_DIR_RIGHT);

    do_label(UI_ALIGN_LEFT,  UI_ALIGN_CENTER, l1_w, h, message);
    advance_panel_cursor(STATUS_BAR_INNER_PAD);
    do_label(UI_ALIGN_RIGHT, UI_ALIGN_CENTER, l2_w, h, mouse_str);
    advance_panel_cursor(STATUS_BAR_INNER_PAD);
    do_label(UI_ALIGN_RIGHT, UI_ALIGN_CENTER, l3_w, h, select_str);

    end_panel();

    // Reset the pushed status bar message for the next update.
    status_bar_pushed_message.clear();
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
