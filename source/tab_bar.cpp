/*******************************************************************************
 * GUI widget that showcases the currently opened level/map tabs in the editor.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float DEFAULT_LEVEL_TAB_WIDTH = 128;
static constexpr float SHIFT_TAB_BUTTON_WIDTH  =  13;

static constexpr size_t NO_TAB_TO_CLOSE = static_cast<size_t>(-1);

static size_t starting_tab_offset = 0;
static size_t max_number_of_tabs  = 0;

static bool need_to_scroll_tab_bar;

static bool can_scroll_in_tab_bar = false;

/* -------------------------------------------------------------------------- */

TEINAPI bool internal__do_level_tab (float w, const Tab& tab, size_t index, bool current)
{
    bool should_close = false;

    float xpad = 6;

    float tw = w;
    float th = get_panel_h();

    Vec2 cursor1(0, 0);
    Vec2 cursor2(xpad, 0);

    float bw = 24;
    float pw = tw-(bw);
    float lw = pw-(xpad*2);

    std::string name((!tab.name.empty()) ? StripFilePath(tab.name) : "Untitled");
    // We insert at the start so that it is always visible even if the
    // level's name gets cut off by the width of the final level tab.
    if (tab.unsaved_changes) name.insert(0, "* ");

    UI_Flag flags = (current) ? UI_HIGHLIGHT : UI_INACTIVE;
    begin_panel(get_panel_cursor().x, 0, tw, th, flags, ui_color_medium);

    set_panel_cursor_dir(UI_DIR_DOWN);
    set_panel_cursor(&cursor1);

    // We display the level tab's full file name in the status bar on hover.
    std::string info((tab.name.empty()) ? "Untitled" : tab.name);
    if (begin_click_panel_gradient(NULL, pw,th+1.0f, flags, info))
    {
        set_current_tab(index);
    }

    set_panel_cursor_dir(UI_DIR_RIGHT);
    set_panel_cursor(&cursor2);

    do_label(UI_ALIGN_LEFT, UI_ALIGN_CENTER, lw,th, name);

    end_panel();

    cursor1.x += pw;
    cursor1.y  = 0.0f;

    if (do_button_img_gradient(NULL, bw,th+1, flags, &gClipCross, info))
    {
        should_close = true;
    }

    end_panel();

    advance_panel_cursor(tw+1);
    return should_close;
}

/* -------------------------------------------------------------------------- */

TEINAPI void handle_tab_bar_events ()
{
    switch (main_event.type)
    {
        case (SDL_MOUSEWHEEL):
        {
            if (can_scroll_in_tab_bar)
            {
                if (main_event.wheel.y > 0) increment_tab();
                if (main_event.wheel.y < 0) decrement_tab();
            }
        } break;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void do_tab_bar ()
{
    float x = get_toolbar_w() + 1;
    float y = 0;

    float bw = SHIFT_TAB_BUTTON_WIDTH;
    float bh = TAB_BAR_HEIGHT;

    set_ui_texture(&gResourceIcons);
    set_ui_font(&GetEditorRegularFont());

    float whole_tab_bar_w = GetViewport().w - get_toolbar_w() - get_control_panel_w();

    float pw = whole_tab_bar_w - (bw*2) - 4;
    float ph = TAB_BAR_HEIGHT;

    // To account for the control panel disappearing.
    if (!current_tab_is_level()) pw += 1;

    // Figure out how many tabs we can fit on the bar before we need to start scrolling.
    max_number_of_tabs = static_cast<int>(ceilf(pw / (DEFAULT_LEVEL_TAB_WIDTH + 1)));

    if (need_to_scroll_tab_bar)
    {
        need_to_scroll_tab_bar = false;
        maybe_scroll_tab_bar();
    }

    float tab_width = DEFAULT_LEVEL_TAB_WIDTH;
    float left_over = 0;

    if (editor.tabs.size() >= max_number_of_tabs)
    {
        tab_width = floorf((pw-((max_number_of_tabs-1)*1)) / max_number_of_tabs);
        left_over = (pw-((max_number_of_tabs-1)*1)) - (tab_width * max_number_of_tabs);
    }
    else
    {
        starting_tab_offset = 0;
    }

    // Prevents the tab bar from being offset too far to the right creating an ugly space when there shouldn't be.
    if (editor.tabs.size() >= max_number_of_tabs)
    {
        while (starting_tab_offset+max_number_of_tabs > editor.tabs.size())
        {
             --starting_tab_offset;
        }
    }

    // THE LEFT ARROW BUTTON
    if (are_there_any_tabs())
    {
        begin_panel(x, y, bw,bh, UI_NONE);
        Vec2 tmp(0,0);
        set_panel_cursor(&tmp);
        bool l_arrow_active = (starting_tab_offset != 0);
        UI_Flag flags = (l_arrow_active) ? UI_NONE : UI_LOCKED;
        if (do_button_img(NULL, bw+1,bh, flags, &gClipArrowLeft))
        {
            --starting_tab_offset;
        }
        end_panel();
    }

    // THE LIST OF TABS
    Vec2 cursor(0,0);

    Vec4 color = (are_there_any_tabs()) ? ui_color_med_dark : ui_color_ex_dark;
    begin_panel(x+bw+1, y, pw, ph, UI_NONE, color);

    set_panel_cursor_dir(UI_DIR_RIGHT);
    set_panel_cursor(&cursor);

    // Check to see if the mouse is in the panel, if it is then the mouse scroll wheel will scroll through tabs.
    can_scroll_in_tab_bar = (mouse_in_ui_bounds_xywh(0, 0, pw, ph) && IsKeyModStateActive(KMOD_NONE));

    size_t index_to_close = NO_TAB_TO_CLOSE;
    size_t last = std::min(editor.tabs.size(), starting_tab_offset+max_number_of_tabs);
    for (size_t i=starting_tab_offset; i<last; ++i)
    {
        float w = tab_width + ((i == last-1) ? left_over : 0);
        bool current = (i == editor.current_tab);
        if (internal__do_level_tab(w, editor.tabs.at(i), i, current))
        {
            index_to_close = i;
        }
    }

    end_panel();

    // THE RIGHT ARROW BUTTON
    if (are_there_any_tabs())
    {
        begin_panel(x+bw+2+pw, 0, bw,bh, UI_NONE);
        Vec2 tmp(0,0);
        set_panel_cursor(&tmp);
        bool r_arrow_active = (starting_tab_offset+max_number_of_tabs < editor.tabs.size());
        UI_Flag flags = (r_arrow_active) ? UI_NONE : UI_LOCKED;
        if (do_button_img(NULL, bw+1,bh, flags, &gClipArrowRight))
        {
            ++starting_tab_offset;
        }
        end_panel();
    }

    // If a level needs to be closed then we do it now.
    if (index_to_close != NO_TAB_TO_CLOSE)
    {
        close_tab(index_to_close);
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void maybe_scroll_tab_bar ()
{
    if (editor.current_tab < starting_tab_offset) starting_tab_offset = editor.current_tab;
    while (editor.current_tab >= std::min(editor.tabs.size(), starting_tab_offset+max_number_of_tabs))
    {
        ++starting_tab_offset;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void move_tab_left ()
{
    if (are_there_any_tabs())
    {
        Tab& tab = get_current_tab();
        if (tab.type != Tab_Type::MAP || !tab.map_node_info.active)
        {
            if (editor.current_tab > 0)
            {
                auto begin = editor.tabs.begin();
                std::iter_swap(begin+editor.current_tab-1, begin+editor.current_tab);
                --editor.current_tab;
                maybe_scroll_tab_bar();
            }
        }
    }
}

TEINAPI void move_tab_right ()
{
    if (are_there_any_tabs())
    {
        Tab& tab = get_current_tab();
        if (tab.type != Tab_Type::MAP || !tab.map_node_info.active)
        {
            if (editor.current_tab < editor.tabs.size()-1)
            {
                auto begin = editor.tabs.begin();
                std::iter_swap(begin+editor.current_tab+1, begin+editor.current_tab);
                ++editor.current_tab;
                maybe_scroll_tab_bar();
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void need_to_scroll_next_update ()
{
    need_to_scroll_tab_bar = true;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool mouse_is_over_tab_bar ()
{
    return can_scroll_in_tab_bar;
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
