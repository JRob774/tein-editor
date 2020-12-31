namespace Internal
{
    TEINAPI bool MouseInsideMapEditorViewport ()
    {
        Vec2 m = gMapEditor.mouse;
        Quad v = gMapEditor.viewport;

        // We do this check for the disabling of cursor drawing during a resize.
        // As once the resize is done normally this function would end up being
        // true and would then draw the cursor at the wrong place after resize
        // so this check prevents that from happening and looks visually better.
        if (!SDL_GetMouseFocus()) return false;

        return ((m.x>=v.x) && (m.y>=v.y) && (m.x<=(v.x+v.w)) && (m.y<=(v.y+v.h)));
    }

    TEINAPI std::string GetTileset (std::string level)
    {
        if (level == "..") return level;
        if (level.empty()) return "";
        if (level.at(0) == '$') level.erase(0, 1);
        size_t end = level.find_first_of("-");
        if (end == std::string::npos) return "";
        return level.substr(0, end);
    }

    TEINAPI Vec4 GetNodeTextColor (Vec4 bg)
    {
        if (gMapEditor.textColorMap.count(bg)) return gMapEditor.textColorMap.at(bg);

        Vec4 bg2 = bg;

        // Useful way of determining whether we want white or black text based on the brightness of the node's bg color.
        // The second W3C guideline-based method was used from this answer here: https://stackoverflow.com/a/3943023
        if (bg.r <= 0.03928f) bg.r /= 12.92f; else bg.r = powf(((bg.r + .055f) / 1.055f), 2.4f);
        if (bg.g <= 0.03928f) bg.g /= 12.92f; else bg.g = powf(((bg.g + .055f) / 1.055f), 2.4f);
        if (bg.b <= 0.03928f) bg.b /= 12.92f; else bg.b = powf(((bg.b + .055f) / 1.055f), 2.4f);

        float l = .2126f * bg.r + .7152f * bg.g + .0722f * bg.b;

        gMapEditor.textColorMap.insert({ bg2, ((l > .179f) ? Vec4(0,0,0,1) : Vec4(1,1,1,1)) });
        return gMapEditor.textColorMap.at(bg2);
    }

    TEINAPI Vec4 GetNodeShadowColor (Vec4 bg)
    {
        return ((GetNodeTextColor(bg) == Vec4(1,1,1,1)) ? Vec4(0,0,0,1) : Vec4(1,1,1,1));
    }

    TEINAPI Vec2 MouseToNodePosition ()
    {
        Vec2 m = gMapEditor.mouseWorld;
        m.x = floorf((m.x - gMapEditor.bounds.x) / gMapNodeWidth);
        m.y = floorf((m.y - gMapEditor.bounds.y) / gMapNodeHeight);
        return m;
    }

    TEINAPI IVec2 MouseToNodePositionInt ()
    {
        Vec2 m = gMapEditor.mouseWorld;
        m.x = floorf((m.x - gMapEditor.bounds.x) / gMapNodeWidth);
        m.y = floorf((m.y - gMapEditor.bounds.y) / gMapNodeHeight);
        return IVec2(static_cast<int>(m.x), static_cast<int>(m.y));
    }

    TEINAPI U32 MapCursorBlinkCallback (U32 interval, void* userData)
    {
        PushEditorEvent(EDITOR_EVENT_BLINK_CURSOR, NULL, NULL);
        return interval;
    }

    TEINAPI void InitMapEditorCursor ()
    {
        gMapEditor.cursorVisible = true;
        gMapEditor.cursorBlinkTimer = SDL_AddTimer(gUiCursorBlinkInterval, MapCursorBlinkCallback, NULL);
        if (!gMapEditor.cursorBlinkTimer)
        {
            LogError(ERR_MIN, "Failed to setup cursor blink timer! (%s)", SDL_GetError());
        }
    }
    TEINAPI void QuitMapEditorCursor ()
    {
        if (gMapEditor.cursorBlinkTimer)
        {
            SDL_RemoveTimer(gMapEditor.cursorBlinkTimer);
            gMapEditor.cursorBlinkTimer = NULL;
        }
    }

    TEINAPI void CreateNewActiveNode ()
    {
        if (!CurrentTabIsMap()) return;
        Tab& tab = GetCurrentTab();
        tab.map.push_back({ tab.mapNodeInfo.activePos.x, tab.mapNodeInfo.activePos.y, "" });
        tab.mapNodeInfo.active = &tab.map.back();
        tab.mapNodeInfo.select = 0;
        tab.mapNodeInfo.cursor = 0;
        tab.mapNodeInfo.cachedLevelText = "";
        SDL_StartTextInput();
    }

    TEINAPI bool RemoveActiveNodeWithNoContent ()
    {
        if (!CurrentTabIsMap()) return false;

        Tab& tab = GetCurrentTab();
        if (tab.mapNodeInfo.active && tab.mapNodeInfo.active->lvl.empty())
        {
            for (size_t i=0; i<tab.map.size(); ++i)
            {
                auto& node = tab.map.at(i);
                if (tab.mapNodeInfo.activePos.x == node.x && tab.mapNodeInfo.activePos.y == node.y)
                {
                    tab.map.erase(tab.map.begin()+i);
                    return true;
                }
            }
        }
        return false;
    }

    TEINAPI void DeselectActiveNode ()
    {
        if (!CurrentTabIsMap()) return;

        Tab& tab = GetCurrentTab();

        QuitMapEditorCursor();
        if (!RemoveActiveNodeWithNoContent()) // Didn't remove.
        {
            if (tab.mapNodeInfo.active)
            {
                if (tab.mapNodeInfo.cachedLevelText != tab.mapNodeInfo.active->lvl)
                {
                    NewMapHistoryState(tab.map);
                    tab.unsavedChanges = true;
                }
            }
        }
        else
        {
            if (!tab.mapNodeInfo.cachedLevelText.empty())
            {
                NewMapHistoryState(tab.map);
                tab.unsavedChanges = true;
            }
        }

        tab.mapNodeInfo.selecting = false;
        tab.mapNodeInfo.active = NULL;

        SDL_StopTextInput();
    }

    TEINAPI bool IsTextSelectActive ()
    {
        if (!CurrentTabIsMap()) return false;
        const Tab& tab = GetCurrentTab();
        return (tab.mapNodeInfo.cursor != tab.mapNodeInfo.select);
    }

    TEINAPI bool MapClipboardEmpty ()
    {
        return (gMapEditor.clipboard.empty());
    }

    TEINAPI void MapCopy ()
    {
        if (!CurrentTabIsMap()) return;
        if (!MapSelectBoxPresent()) return;

        Tab& tab = GetCurrentTab();

        int sx1 = std::min(tab.mapSelect.a.x, tab.mapSelect.b.x);
        int sy1 = std::min(tab.mapSelect.a.y, tab.mapSelect.b.y);
        int sx2 = std::max(tab.mapSelect.a.x, tab.mapSelect.b.x);
        int sy2 = std::max(tab.mapSelect.a.y, tab.mapSelect.b.y);

        gMapEditor.clipboard.clear();

        int minNodeX = INT_MAX;
        int minNodeY = INT_MAX;

        // Determine the position of the top-left most node in the selection so we can offset all the clipboard data relative to that.
        for (auto& node: tab.map)
        {
            if (node.x >= sx1 && node.x <= sx2 && node.y >= sy1 && node.y <= sy2) // Inside select bounds.
            {
                minNodeX = std::min(minNodeX, node.x);
                minNodeY = std::min(minNodeY, node.y);
            }
        }
        for (auto& node: tab.map)
        {
            if (node.x >= sx1 && node.x <= sx2 && node.y >= sy1 && node.y <= sy2) // Inside select bounds.
            {
                gMapEditor.clipboard.push_back({ node.x-minNodeX, node.y-minNodeY, node.lvl });
            }
        }
    }

    TEINAPI void DrawMapClipboard ()
    {
        Tab& tab = GetCurrentTab();

        float px = (1 / tab.camera.zoom);

        float x = gMapEditor.bounds.x;
        float y = gMapEditor.bounds.y;

        IVec2 m = MouseToNodePositionInt();

        Font& font = (IsEditorFontOpenSans()) ? gResourceFontRegularLibMono : gResourceFontMonoDyslexic;
        SetTextBatchFont(font);

        for (auto node: gMapEditor.clipboard)
        {
            float nx = static_cast<float>(node.x + m.x) * gMapNodeWidth;
            float ny = static_cast<float>(node.y + m.y) * gMapNodeHeight;

            float x1 = nx;
            float y1 = ny;
            float x2 = nx+gMapNodeWidth;
            float y2 = ny+gMapNodeHeight;

            std::string tileset(GetTileset(node.lvl));
            SetDrawColor(0,0,0,1);
            FillQuad(x1,y1,x2,y2);
            x2 -= px;
            y2 -= px;

            Vec4 bg = GetTilesetMainColor(tileset);
            SetDrawColor(bg);
            FillQuad(x1,y1,x2,y2);

            // Don't bother drawing text when it's this zoomed out (can't even see it).
            if (tab.camera.zoom >= gMapEditorTextCutOff)
            {
                float tw = GetTextWidthScaled(font, node.lvl);
                float th = GetTextHeightScaled(font, node.lvl);
                float tx = x1+gMapEditorTextPad;
                float ty = y1+roundf(((gMapNodeHeight/2)+(th/4)));

                if (tw > (gMapNodeWidth-(gMapEditorTextPad*2)))
                {
                    Vec2 sa(WorldToScreen(Vec2(x+x1+gMapEditorTextPad, y+y1)));
                    Vec2 sb(WorldToScreen(Vec2(x+x2-gMapEditorTextPad, y+y2)));

                    float scx = floorf(sa.x);
                    float scy = floorf(sa.y);
                    float scw = ceilf (sb.x - scx);
                    float sch = ceilf (sb.y - scy);

                    FlushBatchedText();
                    BeginScissor(scx,scy,scw,sch);
                }

                SetTextBatchColor(GetNodeShadowColor(bg));
                DrawBatchedText(tx+1, ty+1, node.lvl);
                SetTextBatchColor(GetNodeTextColor(bg));
                DrawBatchedText(tx, ty, node.lvl);

                if (tw > (gMapNodeWidth-(gMapEditorTextPad*2)))
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
}

TEINAPI void InitMapEditor ()
{
    gMapEditor.mouseWorld = Vec2(0,0);
    gMapEditor.mouse = Vec2(0,0);
    gMapEditor.mouseTile = Vec2(0,0);

    gMapEditor.cursorBlinkTimer = NULL;
    gMapEditor.cursorVisible = true;

    gMapEditor.bounds = { 0,0,0,0 };
    gMapEditor.viewport = { 0,0,0,0 };
}

TEINAPI void DoMapEditor ()
{
    SetCursorType(Cursor::ARROW);

    Quad p1;

    p1.x = GetToolbarWidth() + 1;
    p1.y = gTabBarHeight  + 1;
    p1.w = GetViewport().w - GetToolbarWidth() - (GetControlPanelWidth()) - 2;
    p1.h = GetViewport().h - gStatusBarHeight - gTabBarHeight - 2;

    // To account for the control panel disappearing.
    p1.w += 1;

    BeginPanel(p1.x, p1.y, p1.w, p1.h, UI_NONE, gEditorSettings.backgroundColor);

    // We cache the mouse position so that systems such as paste which can
    // potentially happen outside of this section of code (where the needed
    // transforms will be applied) can use the mouse position reliably as
    // prior to doing this there were bugs with the cursor's position being
    // slightly off during those operations + it's probably a bit faster.
    PushEditorCameraTransform();
    gMapEditor.mouseWorld = ScreenToWorld(GetMousePos());
    gMapEditor.mouse = GetMousePos();
    gMapEditor.mouseTile = Internal::MouseToNodePosition();
    PopEditorCameraTransform();

    // We cache this just in case anyone else wants to use it (status bar).
    gMapEditor.viewport = GetViewport();

    const Tab& tab = GetCurrentTab();

    PushEditorCameraTransform();

    gMapEditor.bounds.x = 0;
    gMapEditor.bounds.y = 0;
    gMapEditor.bounds.w = GetMapWidth(tab.map) * gMapNodeWidth;
    gMapEditor.bounds.h = GetMapHeight(tab.map) * gMapNodeHeight;

    float x = gMapEditor.bounds.x;
    float y = gMapEditor.bounds.y;
    float w = gMapEditor.bounds.w;
    float h = gMapEditor.bounds.h;

    // Because we mess with the orthographic projection matrix a pixel is no
    // longer 1.0f so we need to adjust by the current zoom to get a pixel.
    //
    // We do this because otherwise the outer border gets scaled incorrectly
    // and looks quite ugly. This method ensures it always remains 1px thick.
    float px = (1 / tab.camera.zoom);

    bool activeNodePosBeenDrawn = false;
    bool mouseOverNode = false;

    // Determine if we are going to draw the clipboard or not.
    bool drawClipboard = false;
    if (!tab.mapNodeInfo.active)
    {
        if (!IsAWindowResizing() && Internal::MouseInsideMapEditorViewport())
        {
            if (!Internal::MapClipboardEmpty() && IsKeyModStateActive(GetKeyBinding(gKbPaste).mod))
            {
                drawClipboard = true;
            }
        }
    }

    // DRAW NODES
    Font& font = (IsEditorFontOpenSans()) ? gResourceFontRegularLibMono : gResourceFontMonoDyslexic;
    SetTextBatchFont(font);
    for (auto node: tab.map)
    {
        float nx = static_cast<float>(node.x) * gMapNodeWidth;
        float ny = static_cast<float>(node.y) * gMapNodeHeight;

        float x1 = nx;
        float y1 = ny;
        float x2 = nx+gMapNodeWidth;
        float y2 = ny+gMapNodeHeight;

        std::string tileset(Internal::GetTileset(node.lvl));
        SetDrawColor(0,0,0,1);
        FillQuad(x1,y1,x2,y2);
        x2 -= px;
        y2 -= px;

        Vec4 bg = GetTilesetMainColor(tileset);

        // Highlight the moused over node and the active node.
        IVec2 m = Internal::MouseToNodePositionInt();
        if (!drawClipboard)
        {
            if ((m.x == node.x && m.y == node.y) || ((tab.mapNodeInfo.active) && (tab.mapNodeInfo.activePos.x == node.x && tab.mapNodeInfo.activePos.y == node.y)))
            {
                if ((m.x == node.x && m.y == node.y))
                {
                    PushStatusBarMessage(node.lvl.c_str());
                    mouseOverNode = true;

                    if (IsWindowFocused("WINMAIN"))
                    {
                        bg.r += ((1-bg.r)*.4f);
                        bg.g += ((1-bg.g)*.4f);
                        bg.b += ((1-bg.b)*.4f);
                    }
                }
                if (((tab.mapNodeInfo.active) && (tab.mapNodeInfo.activePos.x == node.x && tab.mapNodeInfo.activePos.y == node.y)))
                {
                    activeNodePosBeenDrawn = true;
                    bg = gUiColorExLight;
                }
            }
        }

        SetDrawColor(bg);
        FillQuad(x1,y1,x2,y2);

        // Don't bother drawing text when it's this zoomed out (can't even see it).
        if (tab.camera.zoom >= gMapEditorTextCutOff)
        {
            float tw = GetTextWidthScaled (font, node.lvl);
            float th = GetTextHeightScaled(font, node.lvl);
            float tx = x1+gMapEditorTextPad;
            float ty = y1+roundf(((gMapNodeHeight/2)+(th/4)));

            if ((tw > (gMapNodeWidth-(gMapEditorTextPad*2))) || ((tab.mapNodeInfo.active) && (tab.mapNodeInfo.activePos.x == node.x && tab.mapNodeInfo.activePos.y == node.y)))
            {
                Vec2 sa(WorldToScreen(Vec2(x+x1+gMapEditorTextPad, y+y1)));
                Vec2 sb(WorldToScreen(Vec2(x+x2-gMapEditorTextPad, y+y2)));

                float scx = floorf(sa.x);
                float scy = floorf(sa.y);
                float scw = ceilf (sb.x - scx);
                float sch = ceilf (sb.y - scy);

                FlushBatchedText();
                BeginScissor(scx,scy,scw,sch);
            }

            float xOff = 0;
            if (tab.mapNodeInfo.active && (tab.mapNodeInfo.activePos.x == node.x && tab.mapNodeInfo.activePos.y == node.y))
            {
                std::string sub(node.lvl.substr(0, tab.mapNodeInfo.cursor));
                float cursorX = tx+GetTextWidthScaled(font, sub);
                if (cursorX > tx+(gMapNodeWidth-(gMapEditorTextPad*2)))
                {
                    float diff = abs((gMapNodeWidth-(gMapEditorTextPad*2)) - GetTextWidthScaled(font, sub));
                    xOff = -diff;
                }
            }

            SetTextBatchColor(Internal::GetNodeShadowColor(bg));
            DrawBatchedText(tx+xOff+1, ty+1, node.lvl);
            SetTextBatchColor(Internal::GetNodeTextColor(bg));
            DrawBatchedText(tx+xOff, ty, node.lvl);

            if ((tw > (gMapNodeWidth-(gMapEditorTextPad*2))) || ((tab.mapNodeInfo.active) && (tab.mapNodeInfo.activePos.x == node.x && tab.mapNodeInfo.activePos.y == node.y)))
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
    if (!drawClipboard)
    {
        if (!mouseOverNode && Internal::MouseInsideMapEditorViewport())
        {
            if (IsWindowFocused("WINMAIN"))
            {
                Vec2 m = Internal::MouseToNodePosition();
                m.x *= gMapNodeWidth;
                m.y *= gMapNodeHeight;
                SetDrawColor(1,1,1,.5f);
                FillQuad(m.x,m.y,m.x+gMapNodeWidth,m.y+gMapNodeHeight);
            }
        }
        if (tab.mapNodeInfo.active && !activeNodePosBeenDrawn)
        {
            IVec2 m = Internal::MouseToNodePositionInt();
            float nx = tab.mapNodeInfo.activePos.x * gMapNodeWidth;
            float ny = tab.mapNodeInfo.activePos.y * gMapNodeHeight;
            SetDrawColor(((IsUiLight()) ? Vec4(1,1,1,1) : gUiColorExLight));
            FillQuad(nx,ny,nx+gMapNodeWidth,ny+gMapNodeHeight);
        }
    }

    // DRAW TEXT CURSOR/SELECT
    if (tab.mapNodeInfo.active)
    {
        if (tab.camera.zoom >= gMapEditorTextCutOff)
        {
            std::string text = (tab.mapNodeInfo.active) ? tab.mapNodeInfo.active->lvl : "";

            float nx = static_cast<float>(tab.mapNodeInfo.activePos.x) * gMapNodeWidth;
            float ny = static_cast<float>(tab.mapNodeInfo.activePos.y) * gMapNodeHeight;

            float x1 = nx;
            float y1 = ny;
            float x2 = nx+gMapNodeWidth;
            float y2 = ny+gMapNodeHeight;

            float tw = GetTextWidthScaled(font, text);
            float th = GetTextHeightScaled(font, text);

            // So the cursor still draws when there is no text present.
            if (th <= 0) th = font.lineGap.at(font.currentPointSize) * GetFontDrawScale();

            float tx = x1+gMapEditorTextPad;
            float ty = y1+roundf(((gMapNodeHeight/2)+(th/4)));

            float xOff = 0;
            std::string sub(text.substr(0, tab.mapNodeInfo.cursor));
            float cursorX = tx+GetTextWidthScaled(font, sub);
            if (cursorX > tx+(gMapNodeWidth-(gMapEditorTextPad*2)))
            {
                float diff = abs((gMapNodeWidth-(gMapEditorTextPad*2)) - GetTextWidthScaled(font, sub));
                xOff = -diff;
            }

            float xo = GetTextWidthScaled(font, sub);
            float yo = ((y2-y1)-th)/2; // Center the cursor vertically.
            // Just looks nicer...
            if ((tab.mapNodeInfo.cursor != 0 && text.length()) || (!text.length()))
            {
                xo += 1;
            }

            if (Internal::IsTextSelectActive())
            {
                if (!text.empty())
                {
                    size_t begin = std::min(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                    size_t end = std::max(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);

                    Vec2 sa(WorldToScreen(Vec2(x+x1+gMapEditorTextPad, y+y1)));
                    Vec2 sb(WorldToScreen(Vec2(x+x2-gMapEditorTextPad, y+y2)));

                    float scx = floorf(sa.x);
                    float scy = floorf(sa.y);
                    float scw = ceilf (sb.x - scx);
                    float sch = ceilf (sb.y - scy);

                    BeginScissor(scx,scy,scw,sch);

                    float xOff2 = 0;
                    std::string sub2(text.substr(0, tab.mapNodeInfo.select));
                    float cursorX2 = tx+GetTextWidthScaled(font, sub2);
                    if (cursorX2 > tx+(gMapNodeWidth-(gMapEditorTextPad*2)))
                    {
                        float diff = abs((gMapNodeWidth-(gMapEditorTextPad*2)) - GetTextWidthScaled(font, sub2));
                        xOff2 = -diff;
                    }

                    float xo2 = GetTextWidthScaled(font, sub2);
                    // Just looks nicer...
                    if ((tab.mapNodeInfo.select != 0 && text.length()) || (!text.length()))
                    {
                        xo2 += 1;
                    }

                    SetDrawColor(gEditorSettings.selectColor);
                    FillQuad(tx+xo+xOff, y1+yo, tx+xo2+xOff2, y1+yo+th);

                    EndScissor();
                }
            }
            else
            {
                if (gMapEditor.cursorVisible)
                {
                    SetDrawColor(Internal::GetNodeShadowColor(gUiColorExLight));
                    DrawLine(tx+xo+xOff+1, y1+yo+1, tx+xo+xOff+1, y1+yo+th+1);
                    SetDrawColor(Internal::GetNodeTextColor(gUiColorExLight));
                    DrawLine(tx+xo+xOff, y1+yo, tx+xo+xOff, y1+yo+th);
                }
            }
        }
    }

    // DRAW CLIPBOARD
    if (!tab.mapNodeInfo.active)
    {
        if (!IsAWindowResizing() && Internal::MouseInsideMapEditorViewport())
        {
            if (!Internal::MapClipboardEmpty() && IsKeyModStateActive(GetKeyBinding(gKbPaste).mod))
            {
                Internal::DrawMapClipboard();
            }
        }
    }

    // DRAW SELECT
    if (MapSelectBoxPresent())
    {
        float sx1 = std::min(tab.mapSelect.a.x, tab.mapSelect.b.x) * gMapNodeWidth;
        float sy1 = std::min(tab.mapSelect.a.y, tab.mapSelect.b.y) * gMapNodeHeight;
        float sx2 = std::max(tab.mapSelect.a.x, tab.mapSelect.b.x) * gMapNodeWidth;
        float sy2 = std::max(tab.mapSelect.a.y, tab.mapSelect.b.y) * gMapNodeHeight;

        sx2 += gMapNodeWidth;
        sy2 += gMapNodeHeight;

        SetDrawColor(gEditorSettings.selectColor);
        FillQuad(sx1,sy1,sx2,sy2);
    }

    PopEditorCameraTransform();

    EndPanel();
}

TEINAPI void HandleMapEditorEvents ()
{
    if (!CurrentTabIsMap() || !IsWindowFocused("WINMAIN")) return;

    Tab& tab = GetCurrentTab();

    std::string oldText = (tab.mapNodeInfo.active) ? tab.mapNodeInfo.active->lvl : "";
    size_t oldCursor = tab.mapNodeInfo.cursor;

    bool movementAction = false;
    bool justSelected = false;
    bool doNotSetSelect = false;

    switch (main_event.type)
    {
        case (SDL_MOUSEBUTTONDOWN):
        {
            if (Internal::MouseInsideMapEditorViewport())
            {
                if (main_event.button.button == SDL_BUTTON_LEFT)
                {
                    Internal::DeselectActiveNode();
                    tab.mapNodeInfo.pressedNodePos = Internal::MouseToNodePositionInt();
                    tab.mapSelect.a = tab.mapNodeInfo.pressedNodePos;
                    tab.mapSelect.b = tab.mapNodeInfo.pressedNodePos;
                    gMapEditor.leftPressed = true;
                }
                gMapEditor.pressed = true;
            }
        } break;
        case (SDL_MOUSEBUTTONUP):
        {
            if (Internal::MouseInsideMapEditorViewport())
            {
                if (main_event.button.button == SDL_BUTTON_LEFT)
                {
                    if (gMapEditor.pressed && (tab.mapNodeInfo.pressedNodePos == Internal::MouseToNodePositionInt()))
                    {
                        tab.mapNodeInfo.activePos = Internal::MouseToNodePositionInt();
                        // Find the node and if we can't it means it's empty and we will just create one.
                        for (auto& node: tab.map)
                        {
                            if (tab.mapNodeInfo.activePos.x == node.x && tab.mapNodeInfo.activePos.y == node.y)
                            {
                                tab.mapNodeInfo.active = &node;
                                tab.mapNodeInfo.select = 0;
                                tab.mapNodeInfo.cursor = tab.mapNodeInfo.active->lvl.length();
                                tab.mapNodeInfo.cachedLevelText = tab.mapNodeInfo.active->lvl;
                                SDL_StartTextInput();
                                break;
                            }
                        }
                        if (!tab.mapNodeInfo.active)
                        {
                            Internal::CreateNewActiveNode();
                        }
                        Internal::InitMapEditorCursor();
                        justSelected = true;
                    }
                }
                else if (main_event.button.button == SDL_BUTTON_RIGHT)
                {
                    if (gMapEditor.pressed)
                    {
                        Internal::DeselectActiveNode();
                        tab.mapSelect.a = IVec2(0,0);
                        tab.mapSelect.b = IVec2(0,0);
                        gMapEditor.leftPressed = false;
                    }
                }
            }
            gMapEditor.pressed = false;
            if (main_event.button.button == SDL_BUTTON_LEFT)
            {
                gMapEditor.leftPressed = false;
            }
        } break;
        case (SDL_MOUSEMOTION):
        {
            if (gMapEditor.leftPressed)
            {
                if (Internal::MouseInsideMapEditorViewport())
                {
                    tab.mapSelect.b = Internal::MouseToNodePositionInt();
                }
            }
        } break;
        case (SDL_TEXTINPUT):
        {
            if (tab.mapNodeInfo.active)
            {
                if (Internal::IsTextSelectActive())
                {
                    size_t begin = std::min(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                    size_t end = std::max(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                    tab.mapNodeInfo.active->lvl.erase(begin, end-begin);
                    tab.mapNodeInfo.cursor = begin;
                    for (size_t i=0; i<strlen(main_event.text.text); ++i)
                    {
                        auto pos = tab.mapNodeInfo.active->lvl.begin()+(tab.mapNodeInfo.cursor++);
                        tab.mapNodeInfo.active->lvl.insert(pos, main_event.text.text[i]);
                    }
                    tab.mapNodeInfo.select = tab.mapNodeInfo.cursor;
                }
                else
                {
                    for (size_t i=0; i<strlen(main_event.text.text); ++i)
                    {
                        auto pos = tab.mapNodeInfo.active->lvl.begin()+(tab.mapNodeInfo.cursor++);
                        tab.mapNodeInfo.active->lvl.insert(pos, main_event.text.text[i]);
                    }
                }
            }
        } break;
        case (SDL_KEYDOWN):
        {
            if (tab.mapNodeInfo.active)
            {
                bool shift = (SDL_GetModState() & KMOD_SHIFT);
                bool ctrl = (SDL_GetModState() & KMOD_CTRL);

                switch (main_event.key.keysym.sym)
                {
                    case (SDLK_LEFT):
                    {
                        if (Internal::IsTextSelectActive())
                        {
                            tab.mapNodeInfo.cursor = std::min(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                        }
                        if (tab.mapNodeInfo.cursor > 0)
                        {
                            --tab.mapNodeInfo.cursor;
                        }
                        movementAction = true;
                    } break;
                    case (SDLK_RIGHT):
                    {
                        if (Internal::IsTextSelectActive())
                        {
                            tab.mapNodeInfo.cursor = std::max(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                        }
                        if (tab.mapNodeInfo.cursor < tab.mapNodeInfo.active->lvl.length())
                        {
                            ++tab.mapNodeInfo.cursor;
                        }
                        movementAction = true;
                    } break;
                    case (SDLK_HOME):
                    {
                        tab.mapNodeInfo.cursor = 0;
                        movementAction = true;
                    } break;
                    case (SDLK_END):
                    {
                        tab.mapNodeInfo.cursor = tab.mapNodeInfo.active->lvl.length();
                        movementAction = true;
                    } break;
                    case (SDLK_BACKSPACE):
                    {
                        if (Internal::IsTextSelectActive())
                        {
                            size_t begin = std::min(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                            size_t end = std::max(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                            tab.mapNodeInfo.active->lvl.erase(begin, end-begin);
                            tab.mapNodeInfo.cursor = begin;
                            tab.mapNodeInfo.select = begin;
                        }
                        else
                        {
                            if (tab.mapNodeInfo.cursor != 0)
                            {
                                tab.mapNodeInfo.active->lvl.erase(--tab.mapNodeInfo.cursor, 1);
                            }
                        }
                    } break;
                    case (SDLK_DELETE):
                    {
                        if (Internal::IsTextSelectActive())
                        {
                            size_t begin = std::min(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                            size_t end = std::max(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                            tab.mapNodeInfo.active->lvl.erase(begin, end-begin);
                            tab.mapNodeInfo.cursor = begin;
                            tab.mapNodeInfo.select = begin;
                        }
                        else
                        {
                            if (tab.mapNodeInfo.cursor < tab.mapNodeInfo.active->lvl.length())
                            {
                                tab.mapNodeInfo.active->lvl.erase(tab.mapNodeInfo.cursor, 1);
                            }
                        }
                    } break;
                    case (SDLK_RETURN):
                    {
                        Internal::DeselectActiveNode();
                    } break;
                    case (SDLK_ESCAPE):
                    {
                        tab.mapNodeInfo.active->lvl = tab.mapNodeInfo.cachedLevelText;
                        Internal::DeselectActiveNode();
                    } break;
                    case (SDLK_c):
                    {
                        if (ctrl)
                        {
                            if (Internal::IsTextSelectActive())
                            {
                                size_t begin = std::min(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                                size_t end = std::max(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                                std::string text(tab.mapNodeInfo.active->lvl.substr(begin, end-begin));
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
                            if (Internal::IsTextSelectActive())
                            {
                                size_t begin = std::min(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                                size_t end = std::max(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                                std::string text(tab.mapNodeInfo.active->lvl.substr(begin, end-begin));
                                if (SDL_SetClipboardText(text.c_str()) < 0)
                                {
                                    LogError(ERR_MED, "Failed to set clipboard text! (%s)", SDL_GetError());
                                }
                                else
                                {
                                    tab.mapNodeInfo.active->lvl.erase(begin, end-begin);
                                    tab.mapNodeInfo.cursor = begin;
                                    tab.mapNodeInfo.select = begin;
                                    tab.mapNodeInfo.selecting = false;
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
                                    if (Internal::IsTextSelectActive())
                                    {
                                        size_t begin = std::min(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                                        size_t end = std::max(tab.mapNodeInfo.cursor, tab.mapNodeInfo.select);
                                        tab.mapNodeInfo.active->lvl.erase(begin, end-begin);
                                        tab.mapNodeInfo.cursor = begin;
                                        for (size_t i=0; i<strlen(text); ++i)
                                        {
                                            auto pos = tab.mapNodeInfo.active->lvl.begin()+(tab.mapNodeInfo.cursor++);
                                            if (text[i] != '\n' && text[i] != '\r') {
                                                tab.mapNodeInfo.active->lvl.insert(pos, text[i]);
                                            }
                                        }
                                        tab.mapNodeInfo.select = tab.mapNodeInfo.cursor;
                                        tab.mapNodeInfo.selecting = false;
                                    }
                                    else
                                    {
                                        for (size_t i=0; i<strlen(text); ++i)
                                        {
                                            auto pos = tab.mapNodeInfo.active->lvl.begin()+(tab.mapNodeInfo.cursor++);
                                            tab.mapNodeInfo.active->lvl.insert(pos, text[i]);
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
                            tab.mapNodeInfo.select = 0;
                            tab.mapNodeInfo.cursor = tab.mapNodeInfo.active->lvl.length();
                            doNotSetSelect = true;
                        }
                    } break;
                    case (SDLK_LSHIFT):
                    case (SDLK_RSHIFT):
                    {
                        tab.mapNodeInfo.selecting = true;
                    } break;
                }
            }
        } break;
        case (SDL_KEYUP):
        {
            if (tab.mapNodeInfo.active)
            {
                if (main_event.key.keysym.sym == SDLK_LSHIFT || main_event.key.keysym.sym == SDLK_RSHIFT)
                {
                    tab.mapNodeInfo.selecting = false;
                }
            }
        } break;
        case (SDL_USEREVENT):
        {
            if (main_event.user.code == EDITOR_EVENT_BLINK_CURSOR)
            {
                gMapEditor.cursorVisible = !gMapEditor.cursorVisible;
            }
        } break;
    }

    // Reset the cursor blink interval.
    if (tab.mapNodeInfo.active)
    {
        if (oldText != tab.mapNodeInfo.active->lvl || oldCursor != tab.mapNodeInfo.cursor)
        {
            // If the cursor was blinking before then reset the timer.
            if (gMapEditor.cursorBlinkTimer)
            {
                SDL_RemoveTimer(gMapEditor.cursorBlinkTimer);
                gMapEditor.cursorBlinkTimer = NULL;
            }
            // Start the blinking of the cursor.
            gMapEditor.cursorVisible = true;
            gMapEditor.cursorBlinkTimer = SDL_AddTimer(gUiCursorBlinkInterval, Internal::MapCursorBlinkCallback, NULL);
            if (!gMapEditor.cursorBlinkTimer)
            {
                LogError(ERR_MIN, "Failed to setup cursor blink timer! (%s)", SDL_GetError());
            }
        }
    }

    if (!doNotSetSelect)
    {
        if (!justSelected)
        {
            if ((oldCursor != tab.mapNodeInfo.cursor) || movementAction)
            {
                if (!tab.mapNodeInfo.selecting)
                {
                    tab.mapNodeInfo.select = tab.mapNodeInfo.cursor;
                }
            }
        }
    }

    // Important to stop select issues when holding shift for CAPS.
    if (tab.mapNodeInfo.active)
    {
        if (!justSelected)
        {
            if (oldText != tab.mapNodeInfo.active->lvl)
            {
                tab.mapNodeInfo.select = tab.mapNodeInfo.cursor;
            }
        }
    }
}

TEINAPI void LoadMapTab (std::string fileName)
{
    // If there is just one tab and it is completely empty with no changes
    // then we close this tab before opening the new world map(s) in editor.
    if (gEditor.tabs.size() == 1)
    {
        if (IsCurrentTabEmpty() && !GetCurrentTab().unsavedChanges && GetCurrentTab().name.empty())
        {
            CloseCurrentTab();
        }
    }

    size_t tabIndex = GetTabIndexWithThisFileName(fileName);
    if (tabIndex != gInvalidTab) // This file is already open so just focus on it.
    {
        SetCurrentTab(tabIndex);
    }
    else
    {
        CreateNewMapTabAndFocus();
        Tab& tab = GetCurrentTab();
        tab.name = fileName;
        SetMainWindowSubtitleForTab(tab.name);
        if (!LoadMap(tab, tab.name))
        {
            CloseCurrentTab();
        }
        tab.mapHistory.state.at(0) = tab.map;
    }

    NeedToScrollNextUpdate();
}

TEINAPI bool SaveMapTab (Tab& tab)
{
    // If the current file already has a name (has been saved before) then we
    // just do a normal Save to that file. Otherwise, we perform a Save As.
    if (tab.name.empty())
    {
        std::string fileName = SaveDialog(DialogType::CSV);
        if (fileName.empty()) return false;
        tab.name = fileName;
    }

    SaveMap(tab, tab.name);
    BackupMapTab(tab, tab.name);

    tab.unsavedChanges = false;
    SetMainWindowSubtitleForTab(tab.name);

    return true;
}

TEINAPI void SaveMapTabAs ()
{
    std::string fileName = SaveDialog(DialogType::CSV);
    if (fileName.empty()) return;

    Tab& tab = GetCurrentTab();

    tab.name = fileName;
    SaveMap(tab, tab.name);
    BackupMapTab(tab, tab.name);

    tab.unsavedChanges = false;
    SetMainWindowSubtitleForTab(tab.name);
}

TEINAPI void MapDropFile (Tab* tab, std::string fileName)
{
    fileName = FixPathSlashes(fileName);

    // If there is just one tab and it is completely empty with no changes
    // then we close this tab before opening the new world map(s) in editor.
    if (gEditor.tabs.size() == 1)
    {
        if (IsCurrentTabEmpty() && !GetCurrentTab().unsavedChanges && GetCurrentTab().name.empty())
        {
            CloseCurrentTab();
        }
    }

    size_t tabIndex = GetTabIndexWithThisFileName(fileName);
    if (tabIndex != gInvalidTab) // This file is already open so just focus on it.
    {
        SetCurrentTab(tabIndex);
    }
    else
    {
        CreateNewMapTabAndFocus();
        tab = &GetCurrentTab();
        tab->name = fileName;
        SetMainWindowSubtitleForTab(tab->name);
        if (!LoadMap(*tab, tab->name))
        {
            CloseCurrentTab();
        }
        tab->mapHistory.state.at(0) = tab->map;
    }

    NeedToScrollNextUpdate();
}

TEINAPI void BackupMapTab (const Tab& tab, const std::string& fileName)
{
    // Determine how many backups the user wants saved for a given map.
    int backupCount = gEditorSettings.backupCount;
    if (backupCount <= 0) return; // No backups are wanted!

    std::string mapName((fileName.empty()) ? "untitled" : StripFilePathAndExt(fileName));

    // Create a folder for this particular map's backups if it does not exist.
    // We make separate sub-folders in the backup directory for each map as
    // there was an issue in older versions with the editor freezing when backing
    // up levels to a backups folder with loads of saves. This was because the
    // editor was searching the folder for old backups (leading to a freeze).
    std::string backupPath(MakePathAbsolute("backups/" + mapName + "/"));
    if (!DoesPathExist(backupPath))
    {
        if (!CreatePath(backupPath))
        {
            LogError(ERR_MIN, "Failed to create backup for map \"%s\"!", mapName.c_str());
            return;
        }
    }

    // Determine how many backups are already saved of this map.
    std::vector<std::string> backups;
    ListPathContent(backupPath, backups);

    int mapCount = 0;
    for (auto& file: backups)
    {
        if (IsFile(file))
        {
            // We strip extension twice because there are two extension parts to backups the .bak and the .csv.
            std::string compareName(StripFileExt(StripFilePathAndExt(file)));
            if (InsensitiveCompare(mapName, compareName)) ++mapCount;
        }
    }

    // If there is still room to create a new backup then that is what
    // we do. Otherwise, we overwrite the oldest backup of the map.
    std::string backupName = backupPath + mapName + ".bak";
    if (gEditorSettings.unlimitedBackups || (mapCount < backupCount))
    {
        backupName += std::to_string(mapCount) + ".csv";
        SaveMap(tab, backupName);
    }
    else
    {
        U64 oldest = UINT64_MAX;
        int oldestIndex = 0;

        for (int i=0; i<mapCount; ++i)
        {
            std::string name(backupName + std::to_string(i) + ".csv");
            U64 current = LastFileWriteTime(name);
            if (current < oldest)
            {
                oldest = current;
                oldestIndex = i;
            }
        }

        backupName += std::to_string(oldestIndex) + ".csv";
        SaveMap(tab, backupName);
    }
}

TEINAPI bool IsCurrentMapEmpty ()
{
    if (AreThereAnyMapTabs())
    {
        const Tab& tab = GetCurrentTab();
        if (tab.type == TabType::MAP)
        {
            return (tab.map.empty());
        }
    }
    return false;
}

TEINAPI float GetMinMapBoundsX ()
{
    float x = 0;
    if (CurrentTabIsMap())
    {
        const Tab& tab = GetCurrentTab();
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
    return (x * gMapNodeWidth);
}

TEINAPI float GetMinMapBoundsY ()
{
    float y = 0;
    if (CurrentTabIsMap())
    {
        const Tab& tab = GetCurrentTab();
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
    return (y * gMapNodeHeight);
}

TEINAPI float GetMaxMapBoundsX ()
{
    float x = 0;
    if (CurrentTabIsMap())
    {
        x = FLT_MIN;
        if (!GetCurrentTab().map.empty())
        {
            for (auto& node: GetCurrentTab().map)
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
    return (x * gMapNodeWidth);
}

TEINAPI float GetMaxMapBoundsY ()
{
    float y = 0;
    if (CurrentTabIsMap())
    {
        if (!GetCurrentTab().map.empty())
        {
            y = FLT_MIN;
            for (auto& node: GetCurrentTab().map)
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
    return (y * gMapNodeHeight);
}

TEINAPI void MapEditorCut ()
{
    if (!CurrentTabIsMap() || !MapSelectBoxPresent()) return;
    Tab& tab = GetCurrentTab();
    if (!tab.mapNodeInfo.active)
    {
        Internal::MapCopy();
        MapEditorClearSelect(); // Does deselect and history for us.
    }
}

TEINAPI void MapEditorCopy ()
{
    if (!CurrentTabIsMap() || !MapSelectBoxPresent()) return;
    Tab& tab = GetCurrentTab();
    if (!tab.mapNodeInfo.active)
    {
        Internal::MapCopy();
        MapEditorDeselect();
    }
}

TEINAPI void MapEditorPaste ()
{
    if (!CurrentTabIsMap() || Internal::MapClipboardEmpty()) return;

    Tab& tab = GetCurrentTab();

    if (!tab.mapNodeInfo.active)
    {
        int x1 = Internal::MouseToNodePositionInt().x;
        int y1 = Internal::MouseToNodePositionInt().y;

        // Clear old content from any nodes that are going to be overwritten/overlapped.
        for (auto& newNode: gMapEditor.clipboard)
        {
            tab.map.erase(std::remove_if(tab.map.begin(), tab.map.end(),
            [=](const MapNode& oldNode)
            {
                return (oldNode.x == newNode.x+x1 && oldNode.y == newNode.y+y1);
            }),
            tab.map.end());
        }

        for (auto& node: gMapEditor.clipboard)
        {
            tab.map.push_back({ node.x+x1, node.y+y1, node.lvl });
        }

        NewMapHistoryState(tab.map);
        tab.unsavedChanges = true;
    }
}

TEINAPI void MapEditorDeselect ()
{
    if (!CurrentTabIsMap() || !MapSelectBoxPresent()) return;
    Tab& tab = GetCurrentTab();
    tab.mapSelect.a = IVec2(0,0);
    tab.mapSelect.b = IVec2(0,0);
}

TEINAPI void MapEditorClearSelect ()
{
    if (!CurrentTabIsMap() || !MapSelectBoxPresent()) return;

    Tab& tab = GetCurrentTab();

    int sx1 = std::min(tab.mapSelect.a.x, tab.mapSelect.b.x);
    int sy1 = std::min(tab.mapSelect.a.y, tab.mapSelect.b.y);
    int sx2 = std::max(tab.mapSelect.a.x, tab.mapSelect.b.x);
    int sy2 = std::max(tab.mapSelect.a.y, tab.mapSelect.b.y);

    size_t oldSize = tab.map.size();

    tab.map.erase(std::remove_if(tab.map.begin(), tab.map.end(),
    [=](const MapNode& node)
    {
        return (node.x >= sx1 && node.x <= sx2 && node.y >= sy1 && node.y <= sy2);
    }),
    tab.map.end());

    if (oldSize != tab.map.size())
    {
        NewMapHistoryState(tab.map);
        tab.unsavedChanges = true;
    }

    MapEditorDeselect();
}

TEINAPI void MapEditorSelectAll ()
{
    if (!CurrentTabIsMap()) return;

    Tab& tab = GetCurrentTab();

    if (!tab.mapNodeInfo.active)
    {
        Internal::DeselectActiveNode();

        int x1 = static_cast<int>(GetMinMapBoundsX()/gMapNodeWidth);
        int y1 = static_cast<int>(GetMinMapBoundsY()/gMapNodeHeight);
        int x2 = static_cast<int>(GetMaxMapBoundsX()/gMapNodeWidth);
        int y2 = static_cast<int>(GetMaxMapBoundsY()/gMapNodeHeight);

        tab.mapSelect.a = IVec2(x1,y1);
        tab.mapSelect.b = IVec2(x2,y2);
    }
}

TEINAPI void MapEditorUndo ()
{
    Tab& tab = GetCurrentTab();
    if (tab.mapHistory.currentPosition <= 0) return; // There is no history or we are already at the beginning.
    tab.map = tab.mapHistory.state.at(--tab.mapHistory.currentPosition);
    tab.unsavedChanges = true;
    Internal::DeselectActiveNode();
}
TEINAPI void MapEditorRedo ()
{
    Tab& tab = GetCurrentTab();
    int maximum = static_cast<int>(tab.mapHistory.state.size())-1;
    if (tab.mapHistory.currentPosition >= maximum) return; // There is no history or we are already at the end.
    tab.map = tab.mapHistory.state.at(++tab.mapHistory.currentPosition);
    tab.unsavedChanges = true;
    Internal::DeselectActiveNode();
}

TEINAPI void MapEditorHistoryBegin ()
{
    Tab& tab = GetCurrentTab();
    while (tab.mapHistory.currentPosition > 0) MapEditorUndo();
    tab.unsavedChanges = true;
}
TEINAPI void MapEditorHistoryEnd ()
{
    Tab& tab = GetCurrentTab();
    int maximum = static_cast<int>(tab.mapHistory.state.size())-1;
    while (tab.mapHistory.currentPosition < maximum) MapEditorRedo();
    tab.unsavedChanges = true;
}

TEINAPI void NewMapHistoryState (Map& map)
{
    if (!CurrentTabIsMap()) return;
    Tab& tab = GetCurrentTab();

    // Clear all the history after the current position, if there is any, as
    // it will no longer apply to the timeline of map editor actions anymore.
    int deletePosition = tab.mapHistory.currentPosition+1;
    if (deletePosition < static_cast<int>(tab.mapHistory.state.size()))
    {
        auto begin = tab.mapHistory.state.begin();
        auto end = tab.mapHistory.state.end();

        tab.mapHistory.state.erase(begin+deletePosition, end);
    }

    tab.mapHistory.state.push_back(map);
    ++tab.mapHistory.currentPosition;
}

TEINAPI bool MapSelectBoxPresent ()
{
    if (!CurrentTabIsMap()) return false;
    MapSelect s = GetCurrentTab().mapSelect;
    return (s.a != s.b);
}

TEINAPI void GetMapSelectBounds (int* l, int* t, int* r, int* b)
{
    if (l) *l = 0;
    if (t) *t = 0;
    if (r) *r = 0;
    if (b) *b = 0;

    if (!AreThereAnyTabs()) return;
    if (!MapSelectBoxPresent()) return;

    const Tab& tab = GetCurrentTab();

    if (l) *l = std::min(tab.mapSelect.a.x, tab.mapSelect.b.x);
    if (t) *t = std::max(tab.mapSelect.a.y, tab.mapSelect.b.y);
    if (r) *r = std::max(tab.mapSelect.a.x, tab.mapSelect.b.x);
    if (b) *b = std::min(tab.mapSelect.a.y, tab.mapSelect.b.y);
}
