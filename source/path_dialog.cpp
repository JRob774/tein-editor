/*******************************************************************************
 * Dialog box that opens up for the user to locate the TEIN executable file.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float PATH_DIALOG_BOTTOM_BORDER = 26;
static std::string temp_game_path;

/* -------------------------------------------------------------------------- */

TEINAPI void open_path ()
{
    temp_game_path = gEditorSettings.gamePath;
    ShowWindow("WINPATH");
}

TEINAPI void do_path ()
{
    Quad p1, p2;

    p1.x = gWindowBorder;
    p1.y = gWindowBorder;
    p1.w = GetViewport().w - (gWindowBorder * 2);
    p1.h = GetViewport().h - (gWindowBorder * 2);

    set_ui_font(&get_editor_regular_font());

    begin_panel(p1, UI_NONE, ui_color_ex_dark);

    float bb = PATH_DIALOG_BOTTOM_BORDER;

    float vw = GetViewport().w;
    float vh = GetViewport().h;

    float bw = roundf(vw / 2);
    float bh = bb - gWindowBorder;

    // Bottom buttons for okaying or cancelling the path dialog.
    Vec2 btn_cursor(0, gWindowBorder);
    begin_panel(0, vh-bb, vw, bb, UI_NONE, ui_color_medium);

    set_panel_cursor_dir(UI_DIR_RIGHT);
    set_panel_cursor(&btn_cursor);

    // Just to make sure that we always reach the end of the panel space.
    float bw2 = vw - bw;

    if (do_button_txt(NULL, bw ,bh, UI_NONE, "Okay"  )) okay_path  ();
    if (do_button_txt(NULL, bw2,bh, UI_NONE, "Cancel")) cancel_path();

    // Add a separator to the left for symmetry.
    btn_cursor.x = 1;
    do_separator(bh);

    end_panel();

    p2.x =                  1;
    p2.y =                  1;
    p2.w = vw             - 2;
    p2.h = vh - p2.y - bb - 1;

    begin_panel(p2, UI_NONE, ui_color_medium);

    constexpr float XPAD = 8;
    constexpr float YPAD = 4;

    Vec2 cursor(XPAD, YPAD);

    set_panel_cursor_dir(UI_DIR_DOWN);
    set_panel_cursor(&cursor);

    constexpr float BUTTON_W = 80;
    constexpr float LABEL_H = 24;

    cursor.x += 2;
    do_label(UI_ALIGN_LEFT,UI_ALIGN_CENTER, LABEL_H, "Please locate 'The End is Nigh' executable:");
    cursor.x -= 2;

    set_panel_cursor_dir(UI_DIR_RIGHT);

    float tw = GetViewport().w - BUTTON_W - (XPAD*2);
    float th = 24;

    cursor.y += 2;
    do_text_box(tw,th, UI_FILEPATH, temp_game_path, "", UI_ALIGN_LEFT);
    cursor.y += 1;

    float btnh = th-2;

    float x1 = cursor.x            - 1;
    float y1 = cursor.y            - 1;
    float x2 = cursor.x + BUTTON_W + 1;
    float y2 = cursor.y + btnh     + 1;

    // Create a nice border so the button's bounds are actually visible!
    SetDrawColor(ui_color_ex_dark);
    FillQuad(x1, y1, x2, y2);

    if (do_button_txt(NULL, BUTTON_W,btnh, UI_SINGLE, "Search"))
    {
        auto result = OpenDialog(DialogType::EXE, false);
        if (!result.empty())
        {
            temp_game_path = result.at(0);
        }
    }

    end_panel();
    end_panel();
}

TEINAPI void okay_path ()
{
    temp_game_path = FixPathSlashes(temp_game_path);
    gEditorSettings.gamePath = temp_game_path;

    save_preferences();
    HideWindow("WINPATH");

    if (!RunExecutable(gEditorSettings.gamePath))
    {
        LogError(ERR_MED, "Failed to launch The End is Nigh executable!");
    }
}

TEINAPI void cancel_path ()
{
    temp_game_path.clear();
    HideWindow("WINPATH");
}

/* -------------------------------------------------------------------------- */

TEINAPI void handle_path_events ()
{
    if (!IsWindowFocused("WINPATH")) return;

    if (!text_box_is_active())
    {
        if (main_event.type == SDL_KEYDOWN)
        {
            switch (main_event.key.keysym.sym)
            {
                case (SDLK_RETURN): okay_path  (); break;
                case (SDLK_ESCAPE): cancel_path(); break;
            }
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
