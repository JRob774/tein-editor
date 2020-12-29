/*******************************************************************************
 * Editor GUI widget that contains all of the other editor sub-panels.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static Quad control_panel_bounds;

/* -------------------------------------------------------------------------- */

TEINAPI void do_control_panel ()
{
    control_panel_bounds.w = CONTROL_PANEL_WIDTH + CONTROL_PANEL_SCROLLBAR_WIDTH;
    control_panel_bounds.h = GetViewport().h;
    control_panel_bounds.x = GetViewport().w - control_panel_bounds.w;
    control_panel_bounds.y = 0;

    // Determine if we need scrollbars for either sub-panels.
    bool scrollbar = tile_panel_needs_scrollbar() || layer_panel_needs_scrollbar();
    if (!scrollbar)
    {
        control_panel_bounds.w = CONTROL_PANEL_WIDTH;
        control_panel_bounds.x = GetViewport().w - control_panel_bounds.w;
    }

    if (current_tab_is_level())
    {
        begin_panel(control_panel_bounds, UI_NONE);
        do_tile_panel (scrollbar);
        do_layer_panel(scrollbar);
        end_panel();
    }
}

TEINAPI float get_control_panel_w ()
{
    return ((current_tab_is_level()) ? control_panel_bounds.w : 0);
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
