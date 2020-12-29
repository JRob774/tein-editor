/*******************************************************************************
 * Dialog box that opens up when the user wants to create a new level/map.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float NEW_V_FRAME_H = 26;

static constexpr float NEW_XPAD = 8;
static constexpr float NEW_YPAD = 8;

static constexpr float NEW_TEXT_BOX_H = 20;

static constexpr const char* NEW_WIDTH_LABEL = "Level Width:  ";
static constexpr const char* NEW_HEIGHT_LABEL = "Level Height:  ";

static int current_new_width = static_cast<int>(DEFAULT_LEVEL_WIDTH);
static int current_new_height = static_cast<int>(DEFAULT_LEVEL_HEIGHT);

static Tab_Type current_tab_type = Tab_Type::LEVEL;

/* -------------------------------------------------------------------------- */

TEINAPI void internal__okay_new ()
{
    if (current_new_width < MINIMUM_LEVEL_WIDTH || current_new_height < MINIMUM_LEVEL_HEIGHT)
    {
        ShowAlert("Warning", FormatString("Minimum level size is %dx%d!", MINIMUM_LEVEL_WIDTH, MINIMUM_LEVEL_HEIGHT), ALERT_TYPE_WARNING, ALERT_BUTTON_OK, "WINNEW");
        return;
    }

    switch (current_tab_type)
    {
        case (Tab_Type::LEVEL): create_new_level_tab_and_focus(get_new_w(), get_new_h()); break;
        case (Tab_Type::MAP  ): create_new_map_tab_and_focus  ();                         break;
    }

    HideWindow("WINNEW");
}

/* -------------------------------------------------------------------------- */

TEINAPI void open_new ()
{
    current_new_width = static_cast<int>(DEFAULT_LEVEL_WIDTH);
    current_new_height = static_cast<int>(DEFAULT_LEVEL_HEIGHT);

    // Default to level because people make more levels than they do maps.
    current_tab_type = Tab_Type::LEVEL;

    ShowWindow("WINNEW");
}

TEINAPI void do_new ()
{
    Quad p1, p2;

    p1.x = gWindowBorder;
    p1.y = gWindowBorder;
    p1.w = GetViewport().w - (gWindowBorder * 2);
    p1.h = GetViewport().h - (gWindowBorder * 2);

    SetUiFont(&GetEditorRegularFont());

    BeginPanel(p1, UI_NONE, gUiColorExDark);

    Vec2 cursor;

    float nvfh = NEW_V_FRAME_H;

    float vw = GetViewport().w;
    float vh = GetViewport().h;

    float bw = roundf(vw / 2);
    float bh = nvfh - gWindowBorder;

    // Top tabs for switching type of file to create.
    cursor = Vec2(0,0);
    BeginPanel(0, 0, vw, nvfh, UI_NONE, gUiColorMedium);

    SetPanelCursorDir(UI_DIR_RIGHT);
    SetPanelCursor(&cursor);

    UiFlag level_flags = (current_tab_type == Tab_Type::LEVEL) ? UI_HIGHLIGHT : UI_INACTIVE;
    UiFlag map_flags   = (current_tab_type == Tab_Type::MAP  ) ? UI_HIGHLIGHT : UI_INACTIVE;

    if (DoTextButton(NULL, bw,bh, level_flags, "Level"    )) current_tab_type = Tab_Type::LEVEL;
    if (DoTextButton(NULL, bw,bh, map_flags,   "World Map")) current_tab_type = Tab_Type::MAP;

    // Just in case of weird rounding manually add the right separator.
    cursor.x = vw;
    DoSeparator(bh);

    // Add a separator to the left for symmetry.
    cursor.x = 1;
    DoSeparator(bh);

    EndPanel();

    // Bottom buttons for okaying or cancelling the resize.
    cursor = Vec2(0, gWindowBorder);
    BeginPanel(0, vh-nvfh, vw, nvfh, UI_NONE, gUiColorMedium);

    SetPanelCursorDir(UI_DIR_RIGHT);
    SetPanelCursor(&cursor);

    // Just to make sure that we always reach the end of the panel space.
    float bw2 = vw - bw;

    if (DoTextButton(NULL, bw ,bh, UI_NONE, "Create")) internal__okay_new();
    if (DoTextButton(NULL, bw2,bh, UI_NONE, "Cancel")) cancel_new();

    // Add a separator to the left for symmetry.
    cursor.x = 1;
    DoSeparator(bh);

    EndPanel();

    p2.x =                    1;
    p2.y = nvfh             + 1;
    p2.w = vw               - 2;
    p2.h = vh - p2.y - nvfh - 1;

    UiFlag panel_flags = (current_tab_type == Tab_Type::LEVEL) ? UI_NONE : UI_LOCKED;
    BeginPanel(p2, panel_flags, gUiColorMedium);

    cursor = Vec2(NEW_XPAD, NEW_YPAD);

    SetPanelCursorDir(UI_DIR_DOWN);
    SetPanelCursor(&cursor);

    float label_w_w = GetTextWidthScaled(GetEditorRegularFont(), NEW_WIDTH_LABEL);
    float label_h_w = GetTextWidthScaled(GetEditorRegularFont(), NEW_HEIGHT_LABEL);

    float text_box_w = (vw-(NEW_XPAD*2));
    float label_w = std::max(label_w_w, label_h_w);

    std::string w_str(std::to_string(current_new_width));
    std::string h_str(std::to_string(current_new_height));

    DoTextBoxLabeled(text_box_w, NEW_TEXT_BOX_H, UI_NUMERIC, w_str, label_w, NEW_WIDTH_LABEL, "0");
    AdvancePanelCursor(NEW_YPAD);
    DoTextBoxLabeled(text_box_w, NEW_TEXT_BOX_H, UI_NUMERIC, h_str, label_w, NEW_HEIGHT_LABEL, "0");

    if (atoi(w_str.c_str()) > MAXIMUM_LEVEL_WIDTH)
    {
        w_str = std::to_string(MAXIMUM_LEVEL_WIDTH);
    }
    if (atoi(h_str.c_str()) > MAXIMUM_LEVEL_HEIGHT)
    {
        h_str = std::to_string(MAXIMUM_LEVEL_HEIGHT);
    }

    int old_new_width = current_new_width;
    int new_new_width = atoi(w_str.c_str());
    if (new_new_width != old_new_width)
    {
        current_new_width = new_new_width;
    }
    int old_new_height = current_new_height;
    int new_new_height = atoi(h_str.c_str());
    if (new_new_height != old_new_height)
    {
        current_new_height = new_new_height;
    }

    EndPanel();

    EndPanel();
}

TEINAPI void cancel_new ()
{
    HideWindow("WINNEW");
}

/* -------------------------------------------------------------------------- */

TEINAPI void handle_new_events ()
{
    if (IsWindowFocused("WINNEW"))
    {
        if (!TextBoxIsActive())
        {
            if (main_event.type == SDL_KEYDOWN)
            {
                switch (main_event.key.keysym.sym)
                {
                    case (SDLK_RETURN): internal__okay_new(); break;
                    case (SDLK_ESCAPE): cancel_new();         break;
                }
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI int get_new_w ()
{
    return current_new_width;
}

TEINAPI int get_new_h ()
{
    return current_new_height;
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
