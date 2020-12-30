/*******************************************************************************
 * The systems and functionality for the map editing portion of the editor.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

TEINAPI bool internal__mouse_inside_map_editor_viewport ()
{
    Vec2 m = map_editor.mouse;
    Quad v = map_editor.viewport;

    // We do this check for the disabling of cursor drawing during a resize.
    // As once the resize is done normally this function would end up being
    // true and would then draw the cursor at the wrong place after resize
    // so this check prevents that from happening and looks visually better.
    if (!SDL_GetMouseFocus()) return false;

    return ((m.x>=v.x) && (m.y>=v.y) && (m.x<=(v.x+v.w)) && (m.y<=(v.y+v.h)));
}

TEINAPI std::string internal__get_tileset (std::string lvl)
{
    if (lvl == "..") return lvl;
    if (lvl.empty()) return "";
    if (lvl.at(0) == '$') lvl.erase(0, 1);
    size_t end = lvl.find_first_of("-");
    if (end == std::string::npos) return "";
    return lvl.substr(0, end);
}

TEINAPI Vec4 internal__get_node_text_color (Vec4 bg)
{
    if (map_editor.text_color_map.count(bg)) return map_editor.text_color_map.at(bg);

    Vec4 bg2 = bg;

    // Useful way of determining whether we want white or black text based on the brightness of the node's bg color.
    // The second W3C guideline-based method was used from this answer here: https://stackoverflow.com/a/3943023
    if (bg.r <= 0.03928f) bg.r /= 12.92f; else bg.r = powf(((bg.r + .055f) / 1.055f), 2.4f);
    if (bg.g <= 0.03928f) bg.g /= 12.92f; else bg.g = powf(((bg.g + .055f) / 1.055f), 2.4f);
    if (bg.b <= 0.03928f) bg.b /= 12.92f; else bg.b = powf(((bg.b + .055f) / 1.055f), 2.4f);

    float l = .2126f * bg.r + .7152f * bg.g + .0722f * bg.b;

    map_editor.text_color_map.insert(std::pair<Vec4,Vec4>(bg2, ((l > .179f) ? Vec4(0,0,0,1) : Vec4(1,1,1,1))));
    return map_editor.text_color_map.at(bg2);
}

TEINAPI Vec4 internal__get_node_shadow_color (Vec4 bg)
{
    return ((internal__get_node_text_color(bg) == Vec4(1,1,1,1)) ? Vec4(0,0,0,1) : Vec4(1,1,1,1));
}

TEINAPI Vec2 internal__mouse_to_node_position ()
{
    Vec2 m = map_editor.mouse_world;

    m.x = floorf((m.x - map_editor.bounds.x) / MAP_NODE_W);
    m.y = floorf((m.y - map_editor.bounds.y) / MAP_NODE_H);

    return m;
}

TEINAPI IVec2 internal__mouse_to_node_position_int ()
{
    Vec2 m = map_editor.mouse_world;

    m.x = floorf((m.x - map_editor.bounds.x) / MAP_NODE_W);
    m.y = floorf((m.y - map_editor.bounds.y) / MAP_NODE_H);

    return IVec2(static_cast<int>(m.x), static_cast<int>(m.y));
}

TEINAPI U32 internal__map_cursor_blink_callback (U32 interval, void* user_data)
{
    PushEditorEvent(EDITOR_EVENT_BLINK_CURSOR, NULL, NULL);
    return interval;
}

TEINAPI void internal__init_map_editor_cursor ()
{
    map_editor.cursor_visible = true;
    map_editor.cursor_blink_timer = SDL_AddTimer(gUiCursorBlinkInterval, internal__map_cursor_blink_callback, NULL);
    if (!map_editor.cursor_blink_timer)
    {
        LogError(ERR_MIN, "Failed to setup cursor blink timer! (%s)", SDL_GetError());
    }
}

TEINAPI void internal__quit_map_editor_cursor ()
{
    if (map_editor.cursor_blink_timer)
    {
        SDL_RemoveTimer(map_editor.cursor_blink_timer);
        map_editor.cursor_blink_timer = NULL;
    }
}

TEINAPI void internal__create_new_active_node ()
{
    if (!current_tab_is_map()) return;

    Tab& tab = get_current_tab();
    tab.map.push_back(MapNode { tab.map_node_info.active_pos.x, tab.map_node_info.active_pos.y, "" });
    tab.map_node_info.active = &tab.map.back();
    tab.map_node_info.select = 0;
    tab.map_node_info.cursor = 0;
    tab.map_node_info.cached_lvl_text = "";

    SDL_StartTextInput();
}

TEINAPI bool internal__remove_active_node_with_no_content ()
{
    if (!current_tab_is_map()) return false;

    Tab& tab = get_current_tab();
    if (tab.map_node_info.active && tab.map_node_info.active->lvl.empty())
    {
        for (size_t i=0; i<tab.map.size(); ++i)
        {
            auto& node = tab.map.at(i);
            if (tab.map_node_info.active_pos.x == node.x && tab.map_node_info.active_pos.y == node.y)
            {
                tab.map.erase(tab.map.begin()+i);
                return true;
            }
        }
    }
    return false;
}

TEINAPI void internal__deselect_active_node ()
{
    if (!current_tab_is_map()) return;

    Tab& tab = get_current_tab();

    internal__quit_map_editor_cursor();
    if (!internal__remove_active_node_with_no_content()) // Didn't remove.
    {
        if (tab.map_node_info.active)
        {
            if (tab.map_node_info.cached_lvl_text != tab.map_node_info.active->lvl)
            {
                new_map_history_state(tab.map);
                tab.unsaved_changes = true;
            }
        }
    }
    else
    {
        if (!tab.map_node_info.cached_lvl_text.empty())
        {
            new_map_history_state(tab.map);
            tab.unsaved_changes = true;
        }
    }

    tab.map_node_info.selecting = false;
    tab.map_node_info.active = NULL;

    SDL_StopTextInput();
}

TEINAPI bool internal__is_text_select_active ()
{
    if (!current_tab_is_map()) return false;
    const Tab& tab = get_current_tab();
    return (tab.map_node_info.cursor != tab.map_node_info.select);
}

TEINAPI bool internal__map_clipboard_empty ()
{
    return (map_editor.clipboard.empty());
}

TEINAPI void internal__map_copy ()
{
    if (!current_tab_is_map    ()) return;
    if (!map_select_box_present()) return;

    Tab& tab = get_current_tab();

    int sx1 = std::min(tab.map_select.a.x, tab.map_select.b.x);
    int sy1 = std::min(tab.map_select.a.y, tab.map_select.b.y);
    int sx2 = std::max(tab.map_select.a.x, tab.map_select.b.x);
    int sy2 = std::max(tab.map_select.a.y, tab.map_select.b.y);

    map_editor.clipboard.clear();

    int min_node_x = INT_MAX;
    int min_node_y = INT_MAX;

    // Determine the position of the top-left most node in the selection so we can offset all the clipboard data relative to that.
    for (auto& node: tab.map)
    {
        if (node.x >= sx1 && node.x <= sx2 && node.y >= sy1 && node.y <= sy2) // Inside select bounds.
        {
            min_node_x = std::min(min_node_x, node.x);
            min_node_y = std::min(min_node_y, node.y);
        }
    }

    for (auto& node: tab.map)
    {
        if (node.x >= sx1 && node.x <= sx2 && node.y >= sy1 && node.y <= sy2) // Inside select bounds.
        {
            map_editor.clipboard.push_back({ node.x-min_node_x, node.y-min_node_y, node.lvl });
        }
    }
}

TEINAPI void internal__draw_map_clipboard ()
{
    Tab& tab = get_current_tab();

    float px = (1 / tab.camera.zoom);

    float x = map_editor.bounds.x;
    float y = map_editor.bounds.y;

    IVec2 m = internal__mouse_to_node_position_int();

    Font& fnt = (IsEditorFontOpenSans()) ? gResourceFontRegularLibMono : gResourceFontMonoDyslexic;
    SetTextBatchFont(fnt);

    for (auto node: map_editor.clipboard)
    {
        float nx = static_cast<float>(node.x + m.x) * MAP_NODE_W;
        float ny = static_cast<float>(node.y + m.y) * MAP_NODE_H;

        float x1 = nx;
        float y1 = ny;
        float x2 = nx+MAP_NODE_W;
        float y2 = ny+MAP_NODE_H;

        std::string tileset(internal__get_tileset(node.lvl));
        SetDrawColor(0,0,0,1);
        FillQuad(x1,y1,x2,y2);
        x2 -= px;
        y2 -= px;

        Vec4 bg = get_tileset_main_color(tileset);
        SetDrawColor(bg);
        FillQuad(x1,y1,x2,y2);

        // Don't bother drawing text when it's this zoomed out (can't even see it).
        if (tab.camera.zoom >= MAP_EDITOR_TEXT_CUT_OFF)
        {
            float tw = GetTextWidthScaled (fnt, node.lvl);
            float th = GetTextHeightScaled(fnt, node.lvl);
            float tx = x1+MAP_EDITOR_TEXT_PAD;
            float ty = y1+roundf(((MAP_NODE_H/2)+(th/4)));

            if (tw > (MAP_NODE_W-(MAP_EDITOR_TEXT_PAD*2)))
            {
                Vec2 sa(WorldToScreen(Vec2(x+x1+MAP_EDITOR_TEXT_PAD, y+y1)));
                Vec2 sb(WorldToScreen(Vec2(x+x2-MAP_EDITOR_TEXT_PAD, y+y2)));

                float scx = floorf(sa.x);
                float scy = floorf(sa.y);
                float scw = ceilf (sb.x - scx);
                float sch = ceilf (sb.y - scy);

                FlushBatchedText();
                BeginScissor(scx,scy,scw,sch);
            }

            SetTextBatchColor(internal__get_node_shadow_color(bg));
            DrawBatchedText(tx+1, ty+1, node.lvl);
            SetTextBatchColor(internal__get_node_text_color(bg));
            DrawBatchedText(tx, ty, node.lvl);

            if (tw > (MAP_NODE_W-(MAP_EDITOR_TEXT_PAD*2)))
            {
                FlushBatchedText();
                EndScissor();
            }
        }

        x2 += px;
        y2 += px;

        SetDrawColor(gEditorSettings.cursorColor);
        FillQuad(x1,y1,x2,y2);
    }

    FlushBatchedText();
}

/* -------------------------------------------------------------------------- */

TEINAPI void init_map_editor ()
{
    map_editor.mouse_world = Vec2(0,0);
    map_editor.mouse       = Vec2(0,0);
    map_editor.mouse_tile  = Vec2(0,0);

    map_editor.cursor_blink_timer = NULL;
    map_editor.cursor_visible     = true;

    map_editor.bounds   = { 0,0,0,0 };
    map_editor.viewport = { 0,0,0,0 };
}

TEINAPI void do_map_editor ()
{
    SetCursorType(Cursor::ARROW);

    Quad p1;

    p1.x = GetToolbarWidth() + 1;
    p1.y = TAB_BAR_HEIGHT  + 1;
    p1.w = GetViewport().w - GetToolbarWidth() - (GetControlPanelWidth()) - 2;
    p1.h = GetViewport().h - STATUS_BAR_HEIGHT - TAB_BAR_HEIGHT - 2;

    // To account for the control panel disappearing.
    p1.w += 1;

    BeginPanel(p1.x, p1.y, p1.w, p1.h, UI_NONE, gEditorSettings.backgroundColor);

    // We cache the mouse position so that systems such as paste which can
    // potentially happen outside of this section of code (where the needed
    // transforms will be applied) can use the mouse position reliably as
    // prior to doing this there were bugs with the cursor's position being
    // slightly off during those operations + it's probably a bit faster.
    push_editor_camera_transform();
    map_editor.mouse_world = ScreenToWorld(GetMousePos());
    map_editor.mouse = GetMousePos();
    map_editor.mouse_tile = internal__mouse_to_node_position();
    pop_editor_camera_transform();

    // We cache this just in case anyone else wants to use it (status bar).
    map_editor.viewport = GetViewport();

    const Tab& tab = get_current_tab();

    push_editor_camera_transform();

    map_editor.bounds.x = 0;
    map_editor.bounds.y = 0;
    map_editor.bounds.w = GetMapWidth(tab.map) * MAP_NODE_W;
    map_editor.bounds.h = GetMapHeight(tab.map) * MAP_NODE_H;

    float x = map_editor.bounds.x;
    float y = map_editor.bounds.y;
    float w = map_editor.bounds.w;
    float h = map_editor.bounds.h;

    // Because we mess with the orthographic projection matrix a pixel is no
    // longer 1.0f so we need to adjust by the current zoom to get a pixel.
    //
    // We do this because otherwise the outer border gets scaled incorrectly
    // and looks quite ugly. This method ensures it always remains 1px thick.
    float px = (1 / tab.camera.zoom);

    bool active_node_pos_been_drawn = false;
    bool mouse_over_node = false;

    // Determine if we are going to draw the clipboard or not.
    bool draw_clipboard = false;
    if (!tab.map_node_info.active)
    {
        if (!IsAWindowResizing() && internal__mouse_inside_map_editor_viewport())
        {
            if (!internal__map_clipboard_empty() && IsKeyModStateActive(GetKeyBinding(gKbPaste).mod))
            {
                draw_clipboard = true;
            }
        }
    }

    // DRAW NODES
    Font& fnt = (IsEditorFontOpenSans()) ? gResourceFontRegularLibMono : gResourceFontMonoDyslexic;
    SetTextBatchFont(fnt);
    for (auto node: tab.map)
    {
        float nx = static_cast<float>(node.x) * MAP_NODE_W;
        float ny = static_cast<float>(node.y) * MAP_NODE_H;

        float x1 = nx;
        float y1 = ny;
        float x2 = nx+MAP_NODE_W;
        float y2 = ny+MAP_NODE_H;

        std::string tileset(internal__get_tileset(node.lvl));
        SetDrawColor(0,0,0,1);
        FillQuad(x1,y1,x2,y2);
        x2 -= px;
        y2 -= px;

        Vec4 bg = get_tileset_main_color(tileset);

        // Highlight the moused over node and the active node.
        IVec2 m = internal__mouse_to_node_position_int();
        if (!draw_clipboard)
        {
            if ((m.x == node.x && m.y == node.y) || ((tab.map_node_info.active) &&
                (tab.map_node_info.active_pos.x == node.x && tab.map_node_info.active_pos.y == node.y)))
            {
                if ((m.x == node.x && m.y == node.y))
                {
                    push_status_bar_message(node.lvl.c_str());
                    mouse_over_node = true;

                    if (IsWindowFocused("WINMAIN"))
                    {
                        bg.r += ((1-bg.r)*.4f);
                        bg.g += ((1-bg.g)*.4f);
                        bg.b += ((1-bg.b)*.4f);
                    }
                }
                if (((tab.map_node_info.active) &&
                    (tab.map_node_info.active_pos.x == node.x && tab.map_node_info.active_pos.y == node.y)))
                {
                    active_node_pos_been_drawn = true;
                    bg = gUiColorExLight;
                }
            }
        }

        SetDrawColor(bg);
        FillQuad(x1,y1,x2,y2);

        // Don't bother drawing text when it's this zoomed out (can't even see it).
        if (tab.camera.zoom >= MAP_EDITOR_TEXT_CUT_OFF)
        {
            float tw = GetTextWidthScaled (fnt, node.lvl);
            float th = GetTextHeightScaled(fnt, node.lvl);
            float tx = x1+MAP_EDITOR_TEXT_PAD;
            float ty = y1+roundf(((MAP_NODE_H/2)+(th/4)));

            if ((tw > (MAP_NODE_W-(MAP_EDITOR_TEXT_PAD*2))) || ((tab.map_node_info.active) &&
                (tab.map_node_info.active_pos.x == node.x && tab.map_node_info.active_pos.y == node.y)))
            {
                Vec2 sa(WorldToScreen(Vec2(x+x1+MAP_EDITOR_TEXT_PAD, y+y1)));
                Vec2 sb(WorldToScreen(Vec2(x+x2-MAP_EDITOR_TEXT_PAD, y+y2)));

                float scx = floorf(sa.x);
                float scy = floorf(sa.y);
                float scw = ceilf (sb.x - scx);
                float sch = ceilf (sb.y - scy);

                FlushBatchedText();
                BeginScissor(scx,scy,scw,sch);
            }

            float x_off = 0;
            if (tab.map_node_info.active &&
                (tab.map_node_info.active_pos.x == node.x && tab.map_node_info.active_pos.y == node.y))
            {
                std::string sub(node.lvl.substr(0, tab.map_node_info.cursor));
                float cursor_x = tx+GetTextWidthScaled(fnt, sub);
                if (cursor_x > tx+(MAP_NODE_W-(MAP_EDITOR_TEXT_PAD*2)))
                {
                    float diff = abs((MAP_NODE_W-(MAP_EDITOR_TEXT_PAD*2)) - GetTextWidthScaled(fnt, sub));
                    x_off = -diff;
                }
            }

            SetTextBatchColor(internal__get_node_shadow_color(bg));
            DrawBatchedText(tx+x_off+1, ty+1, node.lvl);
            SetTextBatchColor(internal__get_node_text_color(bg));
            DrawBatchedText(tx+x_off, ty, node.lvl);

            if ((tw > (MAP_NODE_W-(MAP_EDITOR_TEXT_PAD*2))) || ((tab.map_node_info.active) &&
                (tab.map_node_info.active_pos.x == node.x && tab.map_node_info.active_pos.y == node.y)))
            {
                FlushBatchedText();
                EndScissor();
            }
        }
        else
        {
            if (static_cast<int>(m.x) == node.x && static_cast<int>(m.y) == node.y)
            {
                SetCurrentTooltip(node.lvl);
            }
        }
    }
    FlushBatchedText();

    // DRAW HIGHLIGHT
    if (!draw_clipboard)
    {
        if (!mouse_over_node && internal__mouse_inside_map_editor_viewport())
        {
            if (IsWindowFocused("WINMAIN"))
            {
                Vec2 m = internal__mouse_to_node_position();
                m.x *= MAP_NODE_W;
                m.y *= MAP_NODE_H;
                SetDrawColor(1,1,1,.5f);
                FillQuad(m.x,m.y,m.x+MAP_NODE_W,m.y+MAP_NODE_H);
            }
        }
        if (tab.map_node_info.active && !active_node_pos_been_drawn)
        {
            IVec2 m = internal__mouse_to_node_position_int();
            float nx = tab.map_node_info.active_pos.x * MAP_NODE_W;
            float ny = tab.map_node_info.active_pos.y * MAP_NODE_H;
            SetDrawColor(((IsUiLight()) ? Vec4(1,1,1,1) : gUiColorExLight));
            FillQuad(nx,ny,nx+MAP_NODE_W,ny+MAP_NODE_H);
        }
    }

    // DRAW TEXT CURSOR/SELECT
    if (tab.map_node_info.active)
    {
        if (tab.camera.zoom >= MAP_EDITOR_TEXT_CUT_OFF)
        {
            std::string text = (tab.map_node_info.active) ? tab.map_node_info.active->lvl : "";

            float nx = static_cast<float>(tab.map_node_info.active_pos.x) * MAP_NODE_W;
            float ny = static_cast<float>(tab.map_node_info.active_pos.y) * MAP_NODE_H;

            float x1 = nx;
            float y1 = ny;
            float x2 = nx+MAP_NODE_W;
            float y2 = ny+MAP_NODE_H;

            float tw = GetTextWidthScaled (fnt, text);
            float th = GetTextHeightScaled(fnt, text);

            // So the cursor still draws when there is no text present.
            if (th <= 0) th = fnt.lineGap.at(fnt.currentPointSize) * GetFontDrawScale();

            float tx = x1+MAP_EDITOR_TEXT_PAD;
            float ty = y1+roundf(((MAP_NODE_H/2)+(th/4)));

            float x_off = 0;
            std::string sub(text.substr(0, tab.map_node_info.cursor));
            float cursor_x = tx+GetTextWidthScaled(fnt, sub);
            if (cursor_x > tx+(MAP_NODE_W-(MAP_EDITOR_TEXT_PAD*2)))
            {
                float diff = abs((MAP_NODE_W-(MAP_EDITOR_TEXT_PAD*2)) - GetTextWidthScaled(fnt, sub));
                x_off = -diff;
            }

            float xo = GetTextWidthScaled(fnt, sub);
            float yo = ((y2-y1)-th)/2; // Center the cursor vertically.
            // Just looks nicer...
            if ((tab.map_node_info.cursor != 0 && text.length()) || (!text.length()))
            {
                xo += 1;
            }

            if (internal__is_text_select_active())
            {
                if (!text.empty())
                {
                    size_t begin = std::min(tab.map_node_info.cursor, tab.map_node_info.select);
                    size_t end   = std::max(tab.map_node_info.cursor, tab.map_node_info.select);

                    Vec2 sa(WorldToScreen(Vec2(x+x1+MAP_EDITOR_TEXT_PAD, y+y1)));
                    Vec2 sb(WorldToScreen(Vec2(x+x2-MAP_EDITOR_TEXT_PAD, y+y2)));

                    float scx = floorf(sa.x);
                    float scy = floorf(sa.y);
                    float scw = ceilf (sb.x - scx);
                    float sch = ceilf (sb.y - scy);

                    BeginScissor(scx,scy,scw,sch);

                    float x_off2 = 0;
                    std::string sub2(text.substr(0, tab.map_node_info.select));
                    float cursor_x2 = tx+GetTextWidthScaled(fnt, sub2);
                    if (cursor_x2 > tx+(MAP_NODE_W-(MAP_EDITOR_TEXT_PAD*2)))
                    {
                        float diff = abs((MAP_NODE_W-(MAP_EDITOR_TEXT_PAD*2)) - GetTextWidthScaled(fnt, sub2));
                        x_off2 = -diff;
                    }

                    float xo2 = GetTextWidthScaled(fnt, sub2);
                    // Just looks nicer...
                    if ((tab.map_node_info.select != 0 && text.length()) || (!text.length()))
                    {
                        xo2 += 1;
                    }

                    SetDrawColor(gEditorSettings.selectColor);
                    FillQuad(tx+xo+x_off, y1+yo, tx+xo2+x_off2, y1+yo+th);

                    EndScissor();
                }
            }
            else
            {
                if (map_editor.cursor_visible)
                {
                    SetDrawColor(internal__get_node_shadow_color(gUiColorExLight));
                    DrawLine(tx+xo+x_off+1, y1+yo+1, tx+xo+x_off+1, y1+yo+th+1);
                    SetDrawColor(internal__get_node_text_color(gUiColorExLight));
                    DrawLine(tx+xo+x_off, y1+yo, tx+xo+x_off, y1+yo+th);
                }
            }
        }
    }

    // DRAW CLIPBOARD
    if (!tab.map_node_info.active)
    {
        if (!IsAWindowResizing() && internal__mouse_inside_map_editor_viewport())
        {
            if (!internal__map_clipboard_empty() && IsKeyModStateActive(GetKeyBinding(gKbPaste).mod))
            {
                internal__draw_map_clipboard();
            }
        }
    }

    // DRAW SELECT
    if (map_select_box_present())
    {
        float sx1 = std::min(tab.map_select.a.x, tab.map_select.b.x) * MAP_NODE_W;
        float sy1 = std::min(tab.map_select.a.y, tab.map_select.b.y) * MAP_NODE_H;
        float sx2 = std::max(tab.map_select.a.x, tab.map_select.b.x) * MAP_NODE_W;
        float sy2 = std::max(tab.map_select.a.y, tab.map_select.b.y) * MAP_NODE_H;

        sx2 += MAP_NODE_W;
        sy2 += MAP_NODE_H;

        SetDrawColor(gEditorSettings.selectColor);
        FillQuad(sx1,sy1,sx2,sy2);
    }

    pop_editor_camera_transform();

    EndPanel();
}

/* -------------------------------------------------------------------------- */

TEINAPI void handle_map_editor_events ()
{
    if (!current_tab_is_map() || !IsWindowFocused("WINMAIN")) return;

    Tab& tab = get_current_tab();

    std::string old_text = (tab.map_node_info.active) ? tab.map_node_info.active->lvl : "";
    size_t old_cursor = tab.map_node_info.cursor;

    bool movement_action   = false;
    bool just_selected     = false;
    bool do_not_set_select = false;

    switch (main_event.type)
    {
        case (SDL_MOUSEBUTTONDOWN):
        {
            if (internal__mouse_inside_map_editor_viewport())
            {
                if (main_event.button.button == SDL_BUTTON_LEFT)
                {
                    internal__deselect_active_node();
                    tab.map_node_info.pressed_node_pos = internal__mouse_to_node_position_int();
                    tab.map_select.a = tab.map_node_info.pressed_node_pos;
                    tab.map_select.b = tab.map_node_info.pressed_node_pos;
                    map_editor.left_pressed = true;
                }
                map_editor.pressed = true;
            }
        } break;
        case (SDL_MOUSEBUTTONUP):
        {
            if (internal__mouse_inside_map_editor_viewport())
            {
                if (main_event.button.button == SDL_BUTTON_LEFT)
                {
                    if (map_editor.pressed && (tab.map_node_info.pressed_node_pos == internal__mouse_to_node_position_int()))
                    {
                        tab.map_node_info.active_pos = internal__mouse_to_node_position_int();
                        // Find the node and if we can't it means it's empty and we will just create one.
                        for (auto& node: tab.map)
                        {
                            if (tab.map_node_info.active_pos.x == node.x && tab.map_node_info.active_pos.y == node.y)
                            {
                                tab.map_node_info.active = &node;
                                tab.map_node_info.select = 0;
                                tab.map_node_info.cursor = tab.map_node_info.active->lvl.length();
                                tab.map_node_info.cached_lvl_text = tab.map_node_info.active->lvl;
                                SDL_StartTextInput();
                                break;
                            }
                        }
                        if (!tab.map_node_info.active)
                        {
                            internal__create_new_active_node();
                        }
                        internal__init_map_editor_cursor();
                        just_selected = true;
                    }
                }
                else if (main_event.button.button == SDL_BUTTON_RIGHT)
                {
                    if (map_editor.pressed)
                    {
                        internal__deselect_active_node();
                        tab.map_select.a = IVec2(0,0);
                        tab.map_select.b = IVec2(0,0);
                        map_editor.left_pressed = false;
                    }
                }
            }
            map_editor.pressed = false;
            if (main_event.button.button == SDL_BUTTON_LEFT)
            {
                map_editor.left_pressed = false;
            }
        } break;
        case (SDL_MOUSEMOTION):
        {
            if (map_editor.left_pressed)
            {
                if (internal__mouse_inside_map_editor_viewport())
                {
                    tab.map_select.b = internal__mouse_to_node_position_int();
                }
            }
        } break;
        case (SDL_TEXTINPUT):
        {
            if (tab.map_node_info.active)
            {
                if (internal__is_text_select_active())
                {
                    size_t begin = std::min(tab.map_node_info.cursor, tab.map_node_info.select);
                    size_t end   = std::max(tab.map_node_info.cursor, tab.map_node_info.select);
                    tab.map_node_info.active->lvl.erase(begin, end-begin);
                    tab.map_node_info.cursor = begin;
                    for (size_t i=0; i<strlen(main_event.text.text); ++i)
                    {
                        auto pos = tab.map_node_info.active->lvl.begin()+(tab.map_node_info.cursor++);
                        tab.map_node_info.active->lvl.insert(pos, main_event.text.text[i]);
                    }
                    tab.map_node_info.select = tab.map_node_info.cursor;
                }
                else
                {
                    for (size_t i=0; i<strlen(main_event.text.text); ++i)
                    {
                        auto pos = tab.map_node_info.active->lvl.begin()+(tab.map_node_info.cursor++);
                        tab.map_node_info.active->lvl.insert(pos, main_event.text.text[i]);
                    }
                }
            }
        } break;
        case (SDL_KEYDOWN):
        {
            if (tab.map_node_info.active)
            {
                bool shift = (SDL_GetModState()&KMOD_SHIFT);
                bool ctrl = (SDL_GetModState()&KMOD_CTRL);

                switch (main_event.key.keysym.sym)
                {
                    case (SDLK_LEFT):
                    {
                        if (internal__is_text_select_active())
                        {
                            tab.map_node_info.cursor = std::min(tab.map_node_info.cursor, tab.map_node_info.select);
                        }
                        if (tab.map_node_info.cursor > 0)
                        {
                            --tab.map_node_info.cursor;
                        }
                        movement_action = true;
                    } break;
                    case (SDLK_RIGHT):
                    {
                        if (internal__is_text_select_active())
                        {
                            tab.map_node_info.cursor = std::max(tab.map_node_info.cursor, tab.map_node_info.select);
                        }
                        if (tab.map_node_info.cursor < tab.map_node_info.active->lvl.length())
                        {
                            ++tab.map_node_info.cursor;
                        }
                        movement_action = true;
                    } break;
                    case (SDLK_HOME):
                    {
                        tab.map_node_info.cursor = 0;
                        movement_action = true;
                    } break;
                    case (SDLK_END):
                    {
                        tab.map_node_info.cursor = tab.map_node_info.active->lvl.length();
                        movement_action = true;
                    } break;
                    case (SDLK_BACKSPACE):
                    {
                        if (internal__is_text_select_active())
                        {
                            size_t begin = std::min(tab.map_node_info.cursor, tab.map_node_info.select);
                            size_t end   = std::max(tab.map_node_info.cursor, tab.map_node_info.select);
                            tab.map_node_info.active->lvl.erase(begin, end-begin);
                            tab.map_node_info.cursor = begin;
                            tab.map_node_info.select = begin;
                        }
                        else
                        {
                            if (tab.map_node_info.cursor != 0)
                            {
                                tab.map_node_info.active->lvl.erase(--tab.map_node_info.cursor, 1);
                            }
                        }
                    } break;
                    case (SDLK_DELETE):
                    {
                        if (internal__is_text_select_active())
                        {
                            size_t begin = std::min(tab.map_node_info.cursor, tab.map_node_info.select);
                            size_t end   = std::max(tab.map_node_info.cursor, tab.map_node_info.select);
                            tab.map_node_info.active->lvl.erase(begin, end-begin);
                            tab.map_node_info.cursor = begin;
                            tab.map_node_info.select = begin;
                        }
                        else
                        {
                            if (tab.map_node_info.cursor < tab.map_node_info.active->lvl.length())
                            {
                                tab.map_node_info.active->lvl.erase(tab.map_node_info.cursor, 1);
                            }
                        }
                    } break;
                    case (SDLK_RETURN):
                    {
                        internal__deselect_active_node();
                    } break;
                    case (SDLK_ESCAPE):
                    {
                        tab.map_node_info.active->lvl = tab.map_node_info.cached_lvl_text;
                        internal__deselect_active_node();
                    } break;
                    case (SDLK_c):
                    {
                        if (ctrl)
                        {
                            if (internal__is_text_select_active())
                            {
                                size_t begin = std::min(tab.map_node_info.cursor, tab.map_node_info.select);
                                size_t end   = std::max(tab.map_node_info.cursor, tab.map_node_info.select);
                                std::string text(tab.map_node_info.active->lvl.substr(begin, end-begin));
                                if (SDL_SetClipboardText(text.c_str()) < 0)
                                {
                                    LogError(ERR_MED, "Failed to set clipboard text! (%s)", SDL_GetError());
                                }
                            }
                        }
                    } break;
                    case (SDLK_x):
                    {
                        if (ctrl)
                        {
                            if (internal__is_text_select_active())
                            {
                                size_t begin = std::min(tab.map_node_info.cursor, tab.map_node_info.select);
                                size_t end   = std::max(tab.map_node_info.cursor, tab.map_node_info.select);
                                std::string text(tab.map_node_info.active->lvl.substr(begin, end-begin));
                                if (SDL_SetClipboardText(text.c_str()) < 0)
                                {
                                    LogError(ERR_MED, "Failed to set clipboard text! (%s)", SDL_GetError());
                                }
                                else
                                {
                                    tab.map_node_info.active->lvl.erase(begin, end-begin);
                                    tab.map_node_info.cursor    = begin;
                                    tab.map_node_info.select    = begin;
                                    tab.map_node_info.selecting = false;
                                }
                            }
                        }
                    } break;
                    case (SDLK_v):
                    {
                        if (ctrl)
                        {
                            if (SDL_HasClipboardText())
                            {
                                char* text = SDL_GetClipboardText();
                                if (text)
                                {
                                    Defer { SDL_free(text); }; // Docs say we need to free.
                                    if (internal__is_text_select_active())
                                    {
                                        size_t begin = std::min(tab.map_node_info.cursor, tab.map_node_info.select);
                                        size_t end   = std::max(tab.map_node_info.cursor, tab.map_node_info.select);
                                        tab.map_node_info.active->lvl.erase(begin, end-begin);
                                        tab.map_node_info.cursor = begin;
                                        for (size_t i=0; i<strlen(text); ++i)
                                        {
                                            auto pos = tab.map_node_info.active->lvl.begin()+(tab.map_node_info.cursor++);
                                            if (text[i] != '\n' && text[i] != '\r') {
                                                tab.map_node_info.active->lvl.insert(pos, text[i]);
                                            }
                                        }
                                        tab.map_node_info.select = tab.map_node_info.cursor;
                                        tab.map_node_info.selecting = false;
                                    }
                                    else
                                    {
                                        for (size_t i=0; i<strlen(text); ++i)
                                        {
                                            auto pos = tab.map_node_info.active->lvl.begin()+(tab.map_node_info.cursor++);
                                            tab.map_node_info.active->lvl.insert(pos, text[i]);
                                        }
                                    }
                                }
                            }
                        }
                    } break;
                    case (SDLK_a):
                    {
                        if (ctrl)
                        {
                            tab.map_node_info.select = 0;
                            tab.map_node_info.cursor = tab.map_node_info.active->lvl.length();
                            do_not_set_select = true;
                        }
                    } break;
                    case (SDLK_LSHIFT):
                    case (SDLK_RSHIFT):
                    {
                        tab.map_node_info.selecting = true;
                    } break;
                }
            }
        } break;
        case (SDL_KEYUP):
        {
            if (tab.map_node_info.active)
            {
                if (main_event.key.keysym.sym == SDLK_LSHIFT || main_event.key.keysym.sym == SDLK_RSHIFT)
                {
                    tab.map_node_info.selecting = false;
                }
            }
        } break;
        case (SDL_USEREVENT):
        {
            if (main_event.user.code == EDITOR_EVENT_BLINK_CURSOR)
            {
                map_editor.cursor_visible = !map_editor.cursor_visible;
            }
        } break;
    }

    // Reset the cursor blink interval.
    if (tab.map_node_info.active) {
        if (old_text != tab.map_node_info.active->lvl || old_cursor != tab.map_node_info.cursor) {
            // If the cursor was blinking before then reset the timer.
            if (map_editor.cursor_blink_timer) {
                SDL_RemoveTimer(map_editor.cursor_blink_timer);
                map_editor.cursor_blink_timer = NULL;
            }
            // Start the blinking of the cursor.
            map_editor.cursor_visible = true;
            map_editor.cursor_blink_timer = SDL_AddTimer(gUiCursorBlinkInterval, internal__map_cursor_blink_callback, NULL);
            if (!map_editor.cursor_blink_timer) {
                LogError(ERR_MIN, "Failed to setup cursor blink timer! (%s)", SDL_GetError());
            }
        }
    }

    if (!do_not_set_select) {
        if (!just_selected) {
            if ((old_cursor != tab.map_node_info.cursor) || movement_action) {
                if (!tab.map_node_info.selecting) {
                    tab.map_node_info.select = tab.map_node_info.cursor;
                }
            }
        }
    }

    // Important to stop select issues when holding shift for CAPS.
    if (tab.map_node_info.active) {
        if (!just_selected) {
            if (old_text != tab.map_node_info.active->lvl) {
                tab.map_node_info.select = tab.map_node_info.cursor;
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void load_map_tab (std::string file_name)
{
    // If there is just one tab and it is completely empty with no changes
    // then we close this tab before opening the new world map(s) in editor.
    if (editor.tabs.size() == 1)
    {
        if (is_current_tab_empty() && !get_current_tab().unsaved_changes && get_current_tab().name.empty())
        {
            close_current_tab();
        }
    }

    size_t tab_index = get_tab_index_with_this_file_name(file_name);
    if (tab_index != INVALID_TAB) // This file is already open so just focus on it.
    {
        set_current_tab(tab_index);
    }
    else
    {
        create_new_map_tab_and_focus();
        Tab& tab = get_current_tab();
        tab.name = file_name;
        set_main_window_subtitle_for_tab(tab.name);

        if (!LoadMap(tab, tab.name))
        {
            close_current_tab();
        }

        tab.map_history.state.at(0) = tab.map;
    }

    need_to_scroll_next_update();
}

TEINAPI bool save_map_tab (Tab& tab)
{
    // If the current file already has a name (has been saved before) then we
    // just do a normal Save to that file. Otherwise, we perform a Save As.
    if (tab.name.empty())
    {
        std::string file_name = SaveDialog(DialogType::CSV);
        if (file_name.empty()) return false;
        tab.name = file_name;
    }

    SaveMap(tab, tab.name);
    backup_map_tab(tab, tab.name);

    tab.unsaved_changes = false;
    set_main_window_subtitle_for_tab(tab.name);

    return true;
}

TEINAPI void save_map_tab_as ()
{
    std::string file_name = SaveDialog(DialogType::CSV);
    if (file_name.empty()) return;

    Tab& tab = get_current_tab();

    tab.name = file_name;
    SaveMap(tab, tab.name);
    backup_map_tab(tab, tab.name);

    tab.unsaved_changes = false;
    set_main_window_subtitle_for_tab(tab.name);
}

/* -------------------------------------------------------------------------- */

TEINAPI void map_drop_file (Tab* tab, std::string file_name)
{
    file_name = FixPathSlashes(file_name);

    // If there is just one tab and it is completely empty with no changes
    // then we close this tab before opening the new world map(s) in editor.
    if (editor.tabs.size() == 1)
    {
        if (is_current_tab_empty() && !get_current_tab().unsaved_changes && get_current_tab().name.empty())
        {
            close_current_tab();
        }
    }

    size_t tab_index = get_tab_index_with_this_file_name(file_name);
    if (tab_index != INVALID_TAB) // This file is already open so just focus on it.
    {
        set_current_tab(tab_index);
    }
    else
    {
        create_new_map_tab_and_focus();
        tab = &get_current_tab();
        tab->name = file_name;
        set_main_window_subtitle_for_tab(tab->name);

        if (!LoadMap(*tab, tab->name))
        {
            close_current_tab();
        }

        tab->map_history.state.at(0) = tab->map;
    }

    need_to_scroll_next_update();
}

/* -------------------------------------------------------------------------- */

TEINAPI void backup_map_tab (const Tab& tab, const std::string& file_name)
{
    // Determine how many backups the user wants saved for a given map.
    int backup_count = gEditorSettings.backupCount;
    if (backup_count <= 0) return; // No backups are wanted!

    std::string map_name((file_name.empty()) ? "untitled" : StripFilePathAndExt(file_name));

    // Create a folder for this particular map's backups if it does not exist.
    // We make separate sub-folders in the backup directory for each map as
    // there was an issue in older versions with the editor freezing when backing
    // up levels to a backups folder with loads of saves. This was because the
    // editor was searching the folder for old backups (leading to a freeze).
    std::string backup_path(MakePathAbsolute("backups/" + map_name + "/"));
    if (!DoesPathExist(backup_path))
    {
        if (!CreatePath(backup_path))
        {
            LogError(ERR_MIN, "Failed to create backup for map \"%s\"!", map_name.c_str());
            return;
        }
    }

    // Determine how many backups are already saved of this map.
    std::vector<std::string> backups;
    ListPathContent(backup_path, backups);

    int map_count = 0;
    for (auto& file: backups)
    {
        if (IsFile(file))
        {
            // We strip extension twice because there are two extension parts to backups the .bak and the .csv.
            std::string compare_name(StripFileExt(StripFilePathAndExt(file)));
            if (InsensitiveCompare(map_name, compare_name)) ++map_count;
        }
    }

    // If there is still room to create a new backup then that is what
    // we do. Otherwise, we overwrite the oldest backup of the map.
    std::string backup_name = backup_path + map_name + ".bak";
    if (gEditorSettings.unlimitedBackups || (map_count < backup_count))
    {
        backup_name += std::to_string(map_count) + ".csv";
        SaveMap(tab, backup_name);
    }
    else
    {
        U64 oldest = UINT64_MAX;
        int oldest_index = 0;

        for (int i=0; i<map_count; ++i)
        {
            std::string name(backup_name + std::to_string(i) + ".csv");
            U64 current = LastFileWriteTime(name);
            if (current < oldest)
            {
                oldest = current;
                oldest_index = i;
            }
        }

        backup_name += std::to_string(oldest_index) + ".csv";
        SaveMap(tab, backup_name);
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI bool is_current_map_empty ()
{
    if (are_there_any_map_tabs())
    {
        const Tab& tab = get_current_tab();
        if (tab.type == Tab_Type::MAP)
        {
            return (tab.map.empty());
        }
    }
    return false;
}

/* -------------------------------------------------------------------------- */

TEINAPI float get_min_map_bounds_x ()
{
    float x = 0;
    if (current_tab_is_map())
    {
        const Tab& tab = get_current_tab();
        if (!tab.map.empty())
        {
            if (!(tab.map.size() == 1 && tab.map.back().lvl.empty()))
            {
                x = FLT_MAX;
                for (auto& node: tab.map)
                {
                    if (!node.lvl.empty())
                    {
                        if (node.x < x)
                        {
                            x = static_cast<float>(node.x);
                        }
                    }
                }
            }
        }
    }
    return (x * MAP_NODE_W);
}

TEINAPI float get_min_map_bounds_y ()
{
    float y = 0;
    if (current_tab_is_map())
    {
        const Tab& tab = get_current_tab();
        if (!tab.map.empty())
        {
            if (!(tab.map.size() == 1 && tab.map.back().lvl.empty()))
            {
                y = FLT_MAX;
                for (auto& node: tab.map)
                {
                    if (!node.lvl.empty())
                    {
                        if (node.y < y)
                        {
                            y = static_cast<float>(node.y);
                        }
                    }
                }
            }
        }
    }
    return (y * MAP_NODE_H);
}

TEINAPI float get_max_map_bounds_x ()
{
    float x = 0;
    if (current_tab_is_map())
    {
        x = FLT_MIN;
        if (!get_current_tab().map.empty())
        {
            for (auto& node: get_current_tab().map)
            {
                if (!node.lvl.empty())
                {
                    if (node.x > x)
                    {
                        x = static_cast<float>(node.x);
                    }
                }
            }
        }
    }
    return (x * MAP_NODE_W);
}

TEINAPI float get_max_map_bounds_y ()
{
    float y = 0;
    if (current_tab_is_map())
    {
        if (!get_current_tab().map.empty())
        {
            y = FLT_MIN;
            for (auto& node: get_current_tab().map)
            {
                if (!node.lvl.empty())
                {
                    if (node.y > y)
                    {
                        y = static_cast<float>(node.y);
                    }
                }
            }
        }
    }
    return (y * MAP_NODE_H);
}

/* -------------------------------------------------------------------------- */

TEINAPI void me_cut ()
{
    if (!current_tab_is_map() || !map_select_box_present()) return;

    Tab& tab = get_current_tab();

    if (!tab.map_node_info.active)
    {
        internal__map_copy();
        me_clear_select(); // Does deselect and history for us.
    }
}

TEINAPI void me_copy ()
{
    if (!current_tab_is_map() || !map_select_box_present()) return;

    Tab& tab = get_current_tab();

    if (!tab.map_node_info.active)
    {
        internal__map_copy();
        me_deselect();
    }
}

TEINAPI void me_paste ()
{
    if (!current_tab_is_map() || internal__map_clipboard_empty()) return;

    Tab& tab = get_current_tab();

    if (!tab.map_node_info.active)
    {
        int x1 = internal__mouse_to_node_position_int().x;
        int y1 = internal__mouse_to_node_position_int().y;

        // Clear old content from any nodes that are going to be overwritten/overlapped.
        for (auto& new_node: map_editor.clipboard)
        {
            tab.map.erase(std::remove_if(tab.map.begin(), tab.map.end(),
            [=](const MapNode& old_node)
            {
                return (old_node.x == new_node.x+x1 && old_node.y == new_node.y+y1);
            }),
            tab.map.end());
        }

        for (auto& node: map_editor.clipboard)
        {
            tab.map.push_back({ node.x+x1, node.y+y1, node.lvl });
        }

        new_map_history_state(tab.map);
        tab.unsaved_changes = true;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void me_deselect ()
{
    if (!current_tab_is_map() || !map_select_box_present()) return;

    Tab& tab = get_current_tab();

    tab.map_select.a = IVec2(0,0);
    tab.map_select.b = IVec2(0,0);
}

TEINAPI void me_clear_select ()
{
    if (!current_tab_is_map() || !map_select_box_present()) return;

    Tab& tab = get_current_tab();

    int sx1 = std::min(tab.map_select.a.x, tab.map_select.b.x);
    int sy1 = std::min(tab.map_select.a.y, tab.map_select.b.y);
    int sx2 = std::max(tab.map_select.a.x, tab.map_select.b.x);
    int sy2 = std::max(tab.map_select.a.y, tab.map_select.b.y);

    size_t old_size = tab.map.size();

    tab.map.erase(std::remove_if(tab.map.begin(), tab.map.end(),
    [=](const MapNode& node)
    {
        return (node.x >= sx1 && node.x <= sx2 && node.y >= sy1 && node.y <= sy2);
    }),
    tab.map.end());

    if (old_size != tab.map.size())
    {
        new_map_history_state(tab.map);
        tab.unsaved_changes = true;
    }

    me_deselect();
}

TEINAPI void me_select_all ()
{
    if (!current_tab_is_map()) return;

    Tab& tab = get_current_tab();

    if (!tab.map_node_info.active)
    {
        internal__deselect_active_node();

        int x1 = static_cast<int>(get_min_map_bounds_x()/MAP_NODE_W);
        int y1 = static_cast<int>(get_min_map_bounds_y()/MAP_NODE_H);
        int x2 = static_cast<int>(get_max_map_bounds_x()/MAP_NODE_W);
        int y2 = static_cast<int>(get_max_map_bounds_y()/MAP_NODE_H);

        tab.map_select.a = IVec2(x1,y1);
        tab.map_select.b = IVec2(x2,y2);
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void me_undo ()
{
    Tab& tab = get_current_tab();
    // There is no history or we are already at the beginning.
    if (tab.map_history.current_position <= 0) return;
    tab.map = tab.map_history.state.at(--tab.map_history.current_position);
    tab.unsaved_changes = true;
    internal__deselect_active_node();
}

TEINAPI void me_redo ()
{
    Tab& tab = get_current_tab();
    // There is no history or we are already at the end.
    int maximum = static_cast<int>(tab.map_history.state.size())-1;
    if (tab.map_history.current_position >= maximum) return;
    tab.map = tab.map_history.state.at(++tab.map_history.current_position);
    tab.unsaved_changes = true;
    internal__deselect_active_node();
}

/* -------------------------------------------------------------------------- */

TEINAPI void me_history_begin ()
{
    Tab& tab = get_current_tab();
    while (tab.map_history.current_position > 0) me_undo();
    tab.unsaved_changes = true;
}

TEINAPI void me_history_end ()
{
    Tab& tab = get_current_tab();
    int maximum = static_cast<int>(tab.map_history.state.size())-1;
    while (tab.map_history.current_position < maximum) me_redo();
    tab.unsaved_changes = true;
}

/* -------------------------------------------------------------------------- */

TEINAPI void new_map_history_state (Map& map)
{
    if (!current_tab_is_map()) return;
    Tab& tab = get_current_tab();

    // Clear all the history after the current position, if there is any, as
    // it will no longer apply to the timeline of map editor actions anymore.
    int delete_position = tab.map_history.current_position+1;
    if (delete_position < static_cast<int>(tab.map_history.state.size()))
    {
        auto begin = tab.map_history.state.begin();
        auto end = tab.map_history.state.end();

        tab.map_history.state.erase(begin+delete_position, end);
    }

    tab.map_history.state.push_back(map);
    ++tab.map_history.current_position;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool map_select_box_present ()
{
    if (!current_tab_is_map()) return false;
    Map_Select s = get_current_tab().map_select;
    return (s.a != s.b);
}

TEINAPI void get_map_select_bounds (int* l, int* t, int* r, int* b)
{
    if (l) *l = 0;
    if (t) *t = 0;
    if (r) *r = 0;
    if (b) *b = 0;

    if (!are_there_any_tabs    ()) return;
    if (!map_select_box_present()) return;

    const Tab& tab = get_current_tab();

    if (l) *l = std::min(tab.map_select.a.x, tab.map_select.b.x);
    if (t) *t = std::max(tab.map_select.a.y, tab.map_select.b.y);
    if (r) *r = std::max(tab.map_select.a.x, tab.map_select.b.x);
    if (b) *b = std::min(tab.map_select.a.y, tab.map_select.b.y);
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
