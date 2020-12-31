static const Vec4 gUiDarkColorBlack     = Vec4( .00f,  .00f, 0.00f, 1.0f);
static const Vec4 gUiDarkColorExDark    = Vec4( .20f,  .20f, 0.20f, 1.0f);
static const Vec4 gUiDarkColorDark      = Vec4( .20f,  .20f, 0.20f, 1.0f);
static const Vec4 gUiDarkColorMedDark   = Vec4( .25f,  .25f, 0.25f, 1.0f);
static const Vec4 gUiDarkColorMedium    = Vec4( .33f,  .33f, 0.33f, 1.0f);
static const Vec4 gUiDarkColorMedLight  = Vec4( .43f,  .43f, 0.43f, 1.0f);
static const Vec4 gUiDarkColorLight     = Vec4( .60f,  .60f, 0.60f, 1.0f);
static const Vec4 gUiDarkColorExLight   = Vec4( .90f,  .90f, 0.90f, 1.0f);
static const Vec4 gUiDarkColorWhite     = Vec4(1.00f, 1.00f, 1.00f, 1.0f);
static const Vec4 gUiLightColorBlack    = Vec4( .15f,  .15f,  .15f, 1.0f);
static const Vec4 gUiLightColorExDark   = Vec4( .35f,  .35f,  .35f, 1.0f);
static const Vec4 gUiLightColorDark     = Vec4( .55f,  .55f,  .55f, 1.0f);
static const Vec4 gUiLightColorMedDark  = Vec4( .70f,  .70f,  .70f, 1.0f);
static const Vec4 gUiLightColorMedium   = Vec4( .80f,  .80f,  .80f, 1.0f);
static const Vec4 gUiLightColorMedLight = Vec4( .93f,  .93f,  .93f, 1.0f);
static const Vec4 gUiLightColorLight    = Vec4( .93f,  .93f,  .93f, 1.0f);
static const Vec4 gUiLightColorExLight  = Vec4( .96f,  .96f,  .96f, 1.0f);
static const Vec4 gUiLightColorWhite    = Vec4(1.00f, 1.00f, 1.00f, 1.0f);

struct Panel
{
    Quad absoluteBounds; // Panel position and size on the window.
    Quad viewport;       // Viewport clipped inside parent.
    Vec2 relativeOffset; // Panel position relative to its viewport.

    UiFlag flags;        // Flags that get applied to a panel's content.

    Vec2* cursor;
    UiDir cursorDir;

    bool cursorAdvanceEnabled;
};

typedef U32 UiID;

static constexpr UiID gUiInvalidID = UINT32_MAX;

static std::stack<Panel> gUiPanels;

static UiID gUiCurrentID;

static UiID gUiHotID;
static UiID gUiHitID;

static UiID gUiActiveTextBox;
static UiID gUiHotTextBox;
static size_t gUiTextBoxCursor;

static UiID gUiHotHyperlink;

static UiID gUiActiveHotkeyRebind;

static Texture* gUiTexture;
static Font* gUiFont;

static Vec2 gUiMouseRelative;

static bool gUiMouseLeftUp;
static bool gUiMouseLeftDown;
static bool gUiMouseRightUp;
static bool gUiMouseRightDown;

static bool gUiIsLight;

// We also store the active window ID at the time of the tab press so that the
// system does not move from window-to-window when the user presseses tab.
static WindowID gUiTextBoxTabWindowID;
static bool gUiMakeNextTextBoxActive;
static bool gUiTabHandled;

enum class UiTextEventType: U32 { TEXT, KEY };

struct UiTextEvent
{
    UiTextEventType type;
    std::string text;
    SDL_Keycode key;
};

static std::vector<UiTextEvent> gUiTextEvents;

static SDL_TimerID gUiCursorBlinkTimer;
static bool gUiCursorVisible;

namespace Internal
{
    TEINAPI U32 CursorBlinkCallback (U32 interval, void* userData)
    {
        PushEditorEvent(EDITOR_EVENT_BLINK_CURSOR, NULL, NULL);
        return interval;
    }

    TEINAPI bool IsHot ()
    {
        return (gUiCurrentID == gUiHotID);
    }
    TEINAPI bool IsHit ()
    {
        return (gUiCurrentID == gUiHitID);
    }

    TEINAPI bool IsValidFilePath (char c)
    {
        return (std::string("<>\"|?*").find(c) == std::string::npos);
    }

    // Clip the widget's bounds to be within the panel's visible area.
    // This stops the user being able to click on invisible portions.
    //
    // We also make the bounds relative to the window, so that they can
    // properly be compared with the mouse cursor without any issues.
    TEINAPI Quad GetClippedBounds (float x, float y, float w, float h)
    {
        const Quad& v = GetViewport();
        Quad clippedBounds;

        clippedBounds.x1 = std::max(x,        0.0f) + v.x;
        clippedBounds.y1 = std::max(y,        0.0f) + v.y;
        clippedBounds.x2 = std::min(x + (w-1), v.w) + v.x;
        clippedBounds.y2 = std::min(y + (h-1), v.h) + v.y;

        return clippedBounds;
    }
    TEINAPI Quad GetClippedBounds (Quad& p)
    {
        const Quad& v = GetViewport();
        Quad clippedBounds;

        clippedBounds.x1 = std::max(p.x,          0.0f) + v.x;
        clippedBounds.y1 = std::max(p.y,          0.0f) + v.y;
        clippedBounds.x2 = std::min(p.x + (p.w-1), v.w) + v.x;
        clippedBounds.y2 = std::min(p.y + (p.h-1), v.h) + v.y;

        return clippedBounds;
    }

    TEINAPI bool HandleWidget (float x, float y, float w, float h, bool locked)
    {
        bool result = false;

        // Don't bother handling widgets during resize to avoid ugly redraw stuff.
        if (!IsAWindowResizing())
        {
            if (GetRenderTarget()->focus && GetRenderTarget()->mouse)
            {
                Quad clippedBounds = GetClippedBounds(x,y,w,h);
                Vec2 mouse = GetMousePos();

                // Determine the hot and active states for the global UI context.
                bool inside = PointInBoundsXYXY(mouse, clippedBounds);
                if (!locked && (gUiHitID == gUiCurrentID))
                {
                    if (gUiMouseLeftUp)
                    {
                        if (gUiHotID == gUiCurrentID)
                        {
                            result = true;
                        }
                        gUiHitID = gUiInvalidID;
                    }
                }
                if (gUiHotID == gUiCurrentID)
                {
                    if (!locked && inside)
                    {
                        if (gUiMouseLeftDown)
                        {
                            gUiHitID = gUiCurrentID;
                        }
                    }
                    else
                    {
                        gUiHotID = gUiInvalidID;
                    }
                }
                if (inside)
                {
                    gUiHotID = gUiCurrentID;
                }
            }
        }

        // If true then the widget being checked was activated this frame.
        return result;
    }

    TEINAPI bool HandleWidget (Quad b, bool locked)
    {
        return HandleWidget(b.x,b.y,b.w,b.h, locked);
    }

    TEINAPI Vec2& GetCursorRef (Panel& panel)
    {
        assert(panel.cursor);
        return *panel.cursor;
    }
    TEINAPI Vec2 GetCursor (Panel& panel)
    {
        assert(panel.cursor);
        return *panel.cursor;
    }

    TEINAPI Vec2 GetRelativeCursor (Panel& panel)
    {
        Vec2 cur = GetCursor(panel);
        cur.x += panel.relativeOffset.x;
        cur.y += panel.relativeOffset.y;
        return cur;
    }

    TEINAPI void DrawSeparator (Vec2 cursor, UiDir dir, float w, float h, Vec4 color)
    {
        float x1 = cursor.x;
        float y1 = cursor.y;
        float x2 = cursor.x;
        float y2 = cursor.y;

        switch (dir)
        {
            case (UI_DIR_UP   ): { x1+=1; x2+=(w-1); y1+=1; y2+= 1;    } break;
            case (UI_DIR_RIGHT): { x1+=w; x2+= w;    y1+=1; y2+=(h-1); } break;
            case (UI_DIR_DOWN ): { x1+=1; x2+=(w-1); y1+=h; y2+= h;    } break;
            case (UI_DIR_LEFT ): { x1+=1; x2+= 1;    y1+=1; y2+=(h-1); } break;
        }

        SetDrawColor(color);
        DrawLine(x1,y1,x2,y2);
    }

    TEINAPI void AdvanceUiCursorStart (Panel& panel, float w, float h)
    {
        if (!panel.cursorAdvanceEnabled) return;
        Vec2& cur = GetCursorRef(panel);
        switch (panel.cursorDir)
        {
            case(UI_DIR_UP): cur.y -= h; break;
            case(UI_DIR_LEFT): cur.x -= w; break;
        }
    }
    TEINAPI void AdvanceUiCursorEnd (Panel& panel, float w, float h)
    {
        if (!panel.cursorAdvanceEnabled) return;
        Vec2& cur = GetCursorRef(panel);
        switch (panel.cursorDir)
        {
            case(UI_DIR_RIGHT): cur.x += w; break;
            case(UI_DIR_DOWN): cur.y += h; break;
        }
    }

    TEINAPI void AlignText (UiAlign horz, UiAlign vert, float& x, float& y, float tw, float th, float w, float h)
    {
        // Determine how to place the text based on alignment.
        switch (horz)
        {
            case (UI_ALIGN_LEFT  ): /* No need to do anything. */ break;
            case (UI_ALIGN_RIGHT ): x += roundf( (w-tw));         break;
            case (UI_ALIGN_CENTER): x += roundf(((w-tw)/2));      break;
        }
        switch (vert)
        {
            case (UI_ALIGN_TOP   ): y += gUiFont->lineGap.at(gUiFont->currentPointSize); break;
            case (UI_ALIGN_BOTTOM): y += roundf(((h)  -(th/4)));                         break;
            case (UI_ALIGN_CENTER): y += roundf(((h/2)+(th/4)));                         break;
        }
    }

    TEINAPI bool IsUiMouseLeftDown ()
    {
        return ((GetRenderTarget()->focus) ? gUiMouseLeftDown : false);
    }
    TEINAPI bool IsUiMouseRightDown ()
    {
        return ((GetRenderTarget()->focus) ? gUiMouseRightDown : false);
    }

    TEINAPI std::string DoMarkdownFormatting (std::vector<std::string>& lines, float w)
    {
        Font& font = GetEditorRegularFont();

        std::string text;
        for (auto& line: lines)
        {
            if (line.at(0) == '*') // Looks nicer.
            {
                line.at(0) = '>';
            }

            if (GetTextWidthScaled(font, line) >= w) // Word-wrap.
            {
                float xOff = 0.0f;

                int i = 0;
                int p = 0;

                for (int j=0; j<static_cast<int>(line.length()); ++j)
                {
                    xOff += GetGlyphAdvance(font, line.at(j), i,p);

                    if (line.at(j) == '\n')
                    {
                        xOff = 0.0f;
                    }

                    if (xOff >= w)
                    {
                        for (int k=j; k>=0; --k)
                        {
                            if (line.at(k) == '\n' || k == 0)
                            {
                                line.at(k) = '\n';
                                xOff = 0.0f;
                                j = k;
                                break;
                            }
                            if (line.at(k) == ' ')
                            {
                                line.insert(k, "\n");
                                xOff = 0.0f;
                                break;
                            }
                        }
                    }
                }
            }

            text += line + "\n";
        }

        if (text.back() == '\n')
        {
            text.pop_back();
        }

        return text;
    }
}

TEINAPI bool InitUiSystem ()
{
    gUiHotID = gUiInvalidID;
    gUiHitID = gUiInvalidID;

    gUiActiveTextBox = gUiInvalidID;
    gUiHotTextBox = gUiInvalidID;
    gUiTextBoxCursor = std::string::npos;

    gUiHotHyperlink = gUiInvalidID;

    gUiActiveHotkeyRebind = gUiInvalidID;

    gUiTexture = NULL;
    gUiFont = NULL;

    gUiMouseRelative = Vec2(0,0);

    gUiMouseLeftUp = false;
    gUiMouseLeftDown = false;
    gUiMouseRightUp = false;
    gUiMouseRightDown = false;

    gUiTextBoxTabWindowID = 0;
    gUiMakeNextTextBoxActive = false;
    gUiTabHandled = false;

    gUiCursorBlinkTimer = NULL;
    gUiCursorVisible = true;

    return true;
}

TEINAPI void LoadUiTheme ()
{
    std::string theme = gEditorSettings.uiTheme;

    if (theme == "dark")
    {
        gUiColorBlack = gUiDarkColorBlack;
        gUiColorExDark = gUiDarkColorExDark;
        gUiColorDark = gUiDarkColorDark;
        gUiColorMedDark = gUiDarkColorMedDark;
        gUiColorMedium = gUiDarkColorMedium;
        gUiColorMedLight = gUiDarkColorMedLight;
        gUiColorLight = gUiDarkColorLight;
        gUiColorExLight = gUiDarkColorExLight;
        gUiColorWhite = gUiDarkColorWhite;
        gUiIsLight = false;
    }
    else
    {
        gUiColorBlack = gUiLightColorBlack;
        gUiColorExDark = gUiLightColorExDark;
        gUiColorDark = gUiLightColorDark;
        gUiColorMedDark = gUiLightColorMedDark;
        gUiColorMedium = gUiLightColorMedium;
        gUiColorMedLight = gUiLightColorMedLight;
        gUiColorLight = gUiLightColorLight;
        gUiColorExLight = gUiLightColorExLight;
        gUiColorWhite = gUiLightColorWhite;
        gUiIsLight = true;
    }

    // Reload cursors because the beam cursor changes based on theme.
    LoadEditorCursors();

    // These colors, if default, also depend on the UI theme.
    Vec4 defaultBackgroundColor = gUiColorLight;
    Vec4 defaultTileGridColor = IsUiLight() ? gUiColorBlack : gUiColorExDark;

    if (gEditorSettings.backgroundColorDefaulted) gEditorSettings.backgroundColor = defaultBackgroundColor;
    if (gEditorSettings.tileGridColorDefaulted) gEditorSettings.tileGridColor = defaultTileGridColor;
}

TEINAPI void ResetUiState ()
{
    // Reset the internal UI ID back to the beginning for a new update/cycle.
    gUiCurrentID = 0;

    gUiTextEvents.clear();
    gUiTabHandled = false;

    // We do this during every event otherwise we can end up with some weird
    // values provided by SDL_GetRelativeMouseState, so we just cache here.
    int mouseX, mouseY;
    SDL_GetRelativeMouseState(&mouseX, &mouseY);
    gUiMouseRelative = IVec2(mouseX, mouseY);

    gUiMouseLeftDown = false;
    gUiMouseLeftUp = false;
    gUiMouseRightDown = false;
    gUiMouseRightUp = false;
}

TEINAPI void HandleUiEvents ()
{
    bool prevDown = gUiMouseLeftDown;
    bool prevUp = gUiMouseLeftUp;

    switch (gMainEvent.type)
    {
        case (SDL_MOUSEBUTTONDOWN):
        {
            if (gMainEvent.button.button == SDL_BUTTON_LEFT) gUiMouseLeftDown = true;
            else if (gMainEvent.button.button == SDL_BUTTON_RIGHT) gUiMouseRightDown = true;
        } break;
        case (SDL_MOUSEBUTTONUP):
        {
            if (gMainEvent.button.button == SDL_BUTTON_LEFT) gUiMouseLeftUp = true;
            else if (gMainEvent.button.button == SDL_BUTTON_RIGHT) gUiMouseRightUp = true;
        } break;
        case (SDL_WINDOWEVENT):
        {
            // When focus changes to a new window then the current text box should be deselected.
            // We also do not want the current hit and hot IDs to persist to the new window.
            if (gMainEvent.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
            {
                DeselectActiveTextBox();
                DeselectActiveHotkeyRebind();

                gUiHotID = gUiInvalidID;
                gUiHitID = gUiInvalidID;
            }
        } break;
        case (SDL_USEREVENT):
        {
            if (gMainEvent.user.code == EDITOR_EVENT_BLINK_CURSOR)
            {
                gUiCursorVisible = !gUiCursorVisible;
            }
        } break;
    }

    // Handle events specifically for UI text boxes.
    if (gUiActiveTextBox != gUiInvalidID)
    {
        UiTextEvent textEvent;
        switch (gMainEvent.type)
        {
            case (SDL_TEXTINPUT):
            {
                textEvent.type = UiTextEventType::TEXT;
                textEvent.text = gMainEvent.text.text;
                gUiTextEvents.push_back(textEvent);
            } break;
            case (SDL_KEYDOWN):
            {
                textEvent.type = UiTextEventType::KEY;
                textEvent.key = gMainEvent.key.keysym.sym;
                gUiTextEvents.push_back(textEvent);
            } break;
        }
    }
    else
    {
        if (gMainEvent.type == SDL_KEYDOWN)
        {
            // We do this so we can focus on the first text box in the window!
            if (gMainEvent.key.keysym.sym == SDLK_TAB)
            {
                gUiMakeNextTextBoxActive = true;
                gUiTextBoxTabWindowID = GetFocusedWindow().id;
            }
        }
    }

    // The UI has not changed state so we don't have to worry about redraws.
    if ((prevDown == gUiMouseLeftDown) && (prevUp == gUiMouseLeftUp)) return;

    // This event exists to perform a second redraw of the user interface. Due
    // to the fact we are using an immediate mode GUI, there is a frame delay
    // that causes some gross visual issues due to our program being mainly
    // event-driven. This method of redrawing again prevents the issue easily.
    gShouldPushUiRedrawEvent = true;
}

TEINAPI bool IsUiLight ()
{
    return gUiIsLight;
}

TEINAPI Vec2 UiGetRelativeMouse ()
{
    return gUiMouseRelative;
}

TEINAPI bool MouseInUiBoundsXYWH (float x, float y, float w, float h)
{
    Quad clippedBounds = Internal::GetClippedBounds(x,y,w,h);
    Vec2 mouse = GetMousePos();
    return PointInBoundsXYXY(mouse, clippedBounds);
}
TEINAPI bool MouseInUiBoundsXYWH (Quad b)
{
    return MouseInUiBoundsXYWH(b.x,b.y,b.w,b.h);
}

TEINAPI void SetUiTexture (Texture* texture)
{
    gUiTexture = texture;
}
TEINAPI void SetUiFont (Font* font)
{
    gUiFont = font;
}

TEINAPI bool IsThereAHotUiElement ()
{
    return (gUiHotID != gUiInvalidID);
}
TEINAPI bool IsThereAHitUiElement ()
{
    return (gUiHitID != gUiInvalidID);
}

TEINAPI void DeselectActiveTextBox (std::string& text, std::string defaultText)
{
    // If specified and the text box is empty on exit then we assign
    // the content of the text box to be the passed in default value.
    if (!defaultText.empty() && !text.length()) text = defaultText;
    DeselectActiveTextBox();
}
TEINAPI void DeselectActiveTextBox ()
{
    if (gUiCursorBlinkTimer)
    {
        SDL_RemoveTimer(gUiCursorBlinkTimer);
        gUiCursorBlinkTimer = NULL;
    }

    gUiTextBoxCursor = std::string::npos;
    gUiActiveTextBox = gUiInvalidID;

    SDL_StopTextInput();
}

TEINAPI bool TextBoxIsActive ()
{
    return (gUiActiveTextBox != gUiInvalidID);
}

TEINAPI bool HotkeyIsActive ()
{
    return (gUiActiveHotkeyRebind != gUiInvalidID);
}

TEINAPI void DeselectActiveHotkeyRebind ()
{
    gUiActiveHotkeyRebind = gUiInvalidID;
}

TEINAPI void BeginPanel (float x, float y, float w, float h, UiFlag flags, Vec4 color)
{
    Panel panel;

    // The method of adding a new panel varies depending on whether the panel
    // is a child to an existing panel or if it is a lone panel in the window.
    panel.absoluteBounds = { x,y,w,h };
    if (gUiPanels.size() > 0)
    {
        const Quad& pAbsolute = gUiPanels.top().absoluteBounds;
        const Vec2& pOffset = gUiPanels.top().relativeOffset;
        const Quad& pViewport = gUiPanels.top().viewport;

        Quad& cAbsolute = panel.absoluteBounds;
        Vec2& cOffset = panel.relativeOffset;
        Quad& cViewport = panel.viewport;

        cAbsolute.x += pAbsolute.x + pOffset.x;
        cAbsolute.y += pAbsolute.y + pOffset.y;

        cViewport = cAbsolute;

        // We also clip the panel's viewport to be inside of the
        // parent panel to avoid issues with overlapping/spill.
        float dX = cViewport.x - pViewport.x;
        float dY = cViewport.y - pViewport.y;

        if (cViewport.x < pViewport.x) cViewport.x = pViewport.x, cViewport.w -= roundf(abs(dX)), dX = cViewport.x - pViewport.x;
        if (cViewport.y < pViewport.y) cViewport.y = pViewport.y, cViewport.h -= roundf(abs(dY)), dY = cViewport.y - pViewport.y;

        if (cViewport.x+cViewport.w > pViewport.x+pViewport.w) cViewport.w = pViewport.w - roundf(abs(dX));
        if (cViewport.y+cViewport.h > pViewport.y+pViewport.h) cViewport.h = pViewport.h - roundf(abs(dY));

        if (cViewport.w < 0) cViewport.w = 0;
        if (cViewport.h < 0) cViewport.h = 0;

        // And determine the panel's offset to its viewport.
        cOffset.x = cAbsolute.x - cViewport.x;
        cOffset.y = cAbsolute.y - cViewport.y;

        // Inherit the parent panel's flags.
        panel.flags = flags | gUiPanels.top().flags;
    }
    else
    {
        panel.viewport = panel.absoluteBounds;
        panel.relativeOffset = Vec2(0,0);
        panel.flags = flags;
    }

    panel.cursor = NULL;
    panel.cursorDir = UI_DIR_RIGHT;
    panel.cursorAdvanceEnabled = true;

    SetViewport(panel.viewport);
    gUiPanels.push(panel);

    SetDrawColor(color);
    FillQuad(0, 0, panel.viewport.w, panel.viewport.h);
}

TEINAPI void BeginPanel (Quad bounds, UiFlag flags, Vec4 color)
{
    BeginPanel(bounds.x,bounds.y,bounds.w,bounds.h, flags, color);
}

TEINAPI bool BeginClickPanel (UiAction action, float w, float h, UiFlag flags, std::string info)
{
    Panel& parent = gUiPanels.top();

    Vec2 relCursor = Internal::GetRelativeCursor(parent);
    Vec2 cursor = Internal::GetCursor(parent);

    // Cache the panel's flags so they are easily accessible.
    bool locked = (flags & UI_LOCKED);
    bool highlight = (flags & UI_HIGHLIGHT);

    bool result = Internal::HandleWidget(relCursor.x, relCursor.y, w, h, locked);
    if (result && action) action(); // Make sure action is valid!

    Vec4 back = gUiColorMedium;

    if      (locked)            back = gUiColorMedDark;
    else if (Internal::IsHit()) back = gUiColorDark;
    else if (Internal::IsHot()) back = gUiColorMedLight;

    BeginPanel(cursor.x, cursor.y, w, h, flags, back);
    Internal::AdvanceUiCursorStart(parent, w, h);

    if (highlight && !Internal::IsHit())
    {
        Vec4 color = gUiColorMedLight;
        color.a = .66f;
        SetDrawColor(color);
        FillQuad(0, 0, GetViewport().w, GetViewport().h);
    }

    Vec4 separatorColor = (locked) ? gUiColorDark : gUiColorMedDark;
    Vec2 offset = gUiPanels.top().relativeOffset;

    Internal::DrawSeparator(offset, parent.cursorDir, w, h, separatorColor);
    Internal::AdvanceUiCursorEnd(parent, w, h);

    // If we are currently hot then we push our info to the status bar.
    if (!locked && !info.empty() && Internal::IsHot())
    {
        PushStatusBarMessage(info.c_str());
    }

    ++gUiCurrentID;

    return result;
}

TEINAPI void EndPanel ()
{
    gUiPanels.pop();

    // We either go back to a previous nested panel or this is the last panel
    // and we go back to placing things relative to the entire program window.
    if (gUiPanels.size() > 0) SetViewport(gUiPanels.top().viewport);
    else SetViewport(0, 0, GetRenderTargetWidth(), GetRenderTargetHeight());
}

TEINAPI float GetPanelWidth ()
{
    return gUiPanels.top().absoluteBounds.w;
}
TEINAPI float GetPanelHeight ()
{
    return gUiPanels.top().absoluteBounds.h;
}

TEINAPI Vec2 GetPanelOffset ()
{
    return gUiPanels.top().relativeOffset;
}

TEINAPI Vec2 GetPanelCursor ()
{
    return Internal::GetCursor(gUiPanels.top());
}

TEINAPI void DisablePanelCursorAdvance ()
{
    gUiPanels.top().cursorAdvanceEnabled = false;
}

TEINAPI void EnablePanelCursorAdvance ()
{
    gUiPanels.top().cursorAdvanceEnabled = true;
}

TEINAPI void AdvancePanelCursor (float advance)
{
    Internal::AdvanceUiCursorStart(gUiPanels.top(), advance, advance);
    Internal::AdvanceUiCursorEnd(gUiPanels.top(), advance, advance);
}

TEINAPI void SetPanelCursor (Vec2* cursor)
{
    gUiPanels.top().cursor = cursor;
}

TEINAPI void SetPanelCursorDir (UiDir dir)
{
    gUiPanels.top().cursorDir = dir;
}

TEINAPI void SetPanelFlags (UiFlag flags)
{
    gUiPanels.top().flags = flags;
}

TEINAPI UiFlag GetPanelFlags ()
{
    return gUiPanels.top().flags;
}

TEINAPI float CalculateTextButtonWidth (std::string text)
{
    // Important to return ceiled value otherwise the next button using the
    // cursor to position itself might overlap the previous button by 1px.
    constexpr float XPadding = 20;
    assert(gUiFont);
    return (ceilf(GetTextWidthScaled(*gUiFont, text)) + XPadding);
}

TEINAPI bool DoImageButton (UiAction action, float w, float h, UiFlag flags, const Quad* clip, std::string info, std::string kb, std::string name)
{
    // Make sure that the necessary components are assigned.
    assert(gUiTexture);

    flags |= gUiPanels.top().flags;

    bool inactive = (flags & UI_INACTIVE);
    bool locked = (flags & UI_LOCKED);
    bool highlight = (flags & UI_HIGHLIGHT);

    Internal::AdvanceUiCursorStart(gUiPanels.top(), w, h);

    Texture& texture = *gUiTexture;

    Vec2 cursor = Internal::GetRelativeCursor(gUiPanels.top());

    // We scissor the contents to avoid image overspill.
    BeginScissor(cursor.x, cursor.y, w, h);
    Defer { EndScissor(); };

    bool result = Internal::HandleWidget(cursor.x, cursor.y, w, h, locked);
    if (result && action) action(); // Make sure action is valid!

    Vec4 front = (IsUiLight()) ? Vec4(.4f,.4f,.4f, 1) : Vec4(.73f,.73f,.73f, 1);
    Vec4 back = gUiColorMedium;
    Vec4 shadow = (IsUiLight()) ? Vec4(.9f,.9f,.9f, 1) : Vec4(.16f,.16f,.16f, 1);

    if      (locked)            back = gUiColorMedDark;
    else if (Internal::IsHit()) back = gUiColorDark;
    else if (Internal::IsHot()) back = gUiColorMedLight;

    if (locked || inactive)
    {
        shadow.a = .5f;
        front.a = .5f;
    }

    SetDrawColor(back); // Draw the button's background quad.
    FillQuad(cursor.x, cursor.y, cursor.x + w, cursor.y + h);

    if (highlight && !Internal::IsHit())
    {
        Vec4 color = gUiColorMedLight;
        color.a = .66f;
        SetDrawColor(color);
        FillQuad(0, 0, GetViewport().w, GetViewport().h);
    }

    // The ((w)-1) and ((h)-1) are used to ensure the separator does
    // not mess with the centering of the image based on direction.

    UiDir dir = gUiPanels.top().cursorDir;

    float w2 = (dir == UI_DIR_RIGHT || dir == UI_DIR_LEFT) ? ((w)-1) : (w);
    float h2 = (dir == UI_DIR_UP    || dir == UI_DIR_DOWN) ? ((h)-1) : (h);

    // Center the image within the button.
    float x = roundf(cursor.x + (w2 / 2) + ((dir == UI_DIR_LEFT) ? 1 : 0));
    float y = roundf(cursor.y + (h2 / 2) + ((dir == UI_DIR_UP)   ? 1 : 0));

    float offset = (IsUiLight()) ? -1.0f : 1.0f;

    texture.color = shadow;
    DrawTexture(texture, x, y-offset, clip);
    texture.color = front;
    DrawTexture(texture, x, y, clip);

    Internal::DrawSeparator(Internal::GetRelativeCursor(gUiPanels.top()), gUiPanels.top().cursorDir, w, h, gUiColorMedDark);
    Internal::AdvanceUiCursorEnd(gUiPanels.top(), w, h);

    // If we are currently hot then we push our info to the status bar.
    if (!locked && !info.empty() && Internal::IsHot())
    {
        std::string kbInfo;
        if (!kb.empty())
        {
            kbInfo = FormatString("(%s)", GetKeyBindingMainString(kb).c_str());
            if (GetKeyBinding(kb).altCode && GetKeyBinding(kb).altMod)
            {
                kbInfo += FormatString(" or (%s)", GetKeyBindingAltString(kb).c_str());
            }
        }
        std::string infoText((kbInfo.empty()) ? info : FormatString("%s %s", kbInfo.c_str(), info.c_str()));
        PushStatusBarMessage(infoText.c_str());
    }
    // If we are currently hot then set the tooltip.
    if (!locked && !name.empty() && Internal::IsHot())
    {
        SetCurrentTooltip(name);
    }

    ++gUiCurrentID;

    return result;
}

TEINAPI bool DoTextButton (UiAction action, float w, float h, UiFlag flags, std::string text, std::string info, std::string kb, std::string name)
{
    // Make sure that the necessary components are assigned.
    assert(gUiFont);

    flags |= gUiPanels.top().flags;

    bool inactive = (flags & UI_INACTIVE);
    bool locked = (flags & UI_LOCKED);
    bool highlight = (flags & UI_HIGHLIGHT);
    bool single = (flags & UI_SINGLE);

    Internal::AdvanceUiCursorStart(gUiPanels.top(), w, h);

    Font& font = *gUiFont;

    Vec2 cursor = Internal::GetRelativeCursor(gUiPanels.top());

    // We scissor the contents to avoid text overspill.
    BeginScissor(cursor.x, cursor.y, w, h);
    Defer { EndScissor(); };

    // Locked buttons cannot be interacted with.
    bool result = Internal::HandleWidget(cursor.x, cursor.y, w, h, locked);
    if (result && action) action(); // Make sure action is valid!

    Vec4 front = (IsUiLight()) ? gUiColorBlack : gUiColorExLight;
    Vec4 back = gUiColorMedium;
    Vec4 shadow = (IsUiLight()) ? gUiColorExLight : gUiColorBlack;

    if      (locked)            back = gUiColorMedDark;
    else if (Internal::IsHit()) back = gUiColorDark;
    else if (Internal::IsHot()) back = gUiColorMedLight;

    if (locked || inactive)
    {
        shadow.a = .5f;
        front.a = .5f;
    }

    SetDrawColor(back); // Draw the button's background quad.
    FillQuad(cursor.x, cursor.y, cursor.x + w, cursor.y + h);

    if (highlight && !Internal::IsHit())
    {
        Vec4 color = gUiColorMedLight;
        color.a = .66f;
        SetDrawColor(color);
        FillQuad(0, 0, GetViewport().w, GetViewport().h);
    }

    float w2 = GetTextWidthScaled(font, text);
    float h2 = font.lineGap.at(font.currentPointSize) * GetFontDrawScale();
    // Center the text within the button.
    float x = roundf(cursor.x + ((w - w2) / 2));
    float y = roundf(cursor.y + ((h / 2) + (h2 / 4)));

    float offset = (IsUiLight()) ? -1.0f : 1.0f;

    font.color = shadow;
    DrawText(font, x, y-offset, text);
    font.color = front;
    DrawText(font, x, y, text);

    if (!single)
    {
        Internal::DrawSeparator(Internal::GetRelativeCursor(gUiPanels.top()), gUiPanels.top().cursorDir, w, h, gUiColorMedDark);
    }

    Internal::AdvanceUiCursorEnd(gUiPanels.top(), w, h);

    // If we are currently hot then we push our info to the status bar.
    if (!locked && !info.empty() && Internal::IsHot())
    {
        std::string kbInfo;
        if (!kb.empty())
        {
            kbInfo = FormatString("(%s)", GetKeyBindingMainString(kb).c_str());
            if (GetKeyBinding(kb).altCode && GetKeyBinding(kb).altMod)
            {
                kbInfo += FormatString(" or (%s)", GetKeyBindingAltString(kb).c_str());
            }
        }
        std::string infoText((kbInfo.empty()) ? info : FormatString("%s %s", kbInfo.c_str(), info.c_str()));
        PushStatusBarMessage(infoText.c_str());
    }
    // If we are currently hot then set the tooltip.
    if (!locked && !name.empty() && Internal::IsHot())
    {
        SetCurrentTooltip(name);
    }

    ++gUiCurrentID;

    return result;
}

TEINAPI bool DoTextButton (UiAction action, float h, UiFlag flags, std::string text, std::string info, std::string kb, std::string name)
{
    // Important to return ceiled value otherwise the next button using the
    // cursor to position itself might overlap the previous button by 1px.
    constexpr float XPadding = 20;
    float w = ceilf(GetTextWidthScaled(*gUiFont, text)) + XPadding;
    return DoTextButton(action, w, h, flags, text, info, kb, name);
}

TEINAPI void DoLabel (UiAlign horz, UiAlign vert, float w, float h, std::string text, Vec4 bgColor)
{
    // Make sure that the necessary components are assigned.
    assert(gUiFont);

    UiFlag flags = gUiPanels.top().flags;

    bool inactive = (flags & UI_INACTIVE);
    bool locked = (flags & UI_LOCKED);
    bool tooltip = (flags & UI_TOOLTIP);
    bool darken = (flags & UI_DARKEN);

    Internal::AdvanceUiCursorStart(gUiPanels.top(), w, h);

    Font& font = *gUiFont;

    Vec2 cursor = Internal::GetRelativeCursor(gUiPanels.top());

    // We scissor the contents to avoid text overspill.
    BeginScissor(cursor.x, cursor.y, w, h);
    Defer { EndScissor(); };

    SetDrawColor(bgColor); // Draw the label's background.
    FillQuad(cursor.x, cursor.y, cursor.x + w, cursor.y + h);

    float tw = GetTextWidthScaled(font, text);
    float th = GetTextHeightScaled(font, text);

    // If text is a single line we calculate how much we can fit in the width
    // and if necessary trim any off and replace the end with and ellipsis.
    bool textClipped = false;
    std::string clippedText(text);
    if (std::count(clippedText.begin(), clippedText.end(), '\n') <= 1)
    {
        if (tw > w) // Our text goes out of the label bounds.
        {
            textClipped = true;
            if (clippedText.length() <= 3)
            {
                clippedText = "...";
                tw = GetTextWidthScaled(font, clippedText);
            }
            else
            {
                clippedText.replace(clippedText.length()-3, 3, "...");
                while (tw > w && clippedText.length() > 3)
                {
                    clippedText.erase(clippedText.length()-4, 1);
                    tw = GetTextWidthScaled(font, clippedText);
                }
            }
        }
    }

    float x = cursor.x;
    float y = cursor.y;

    Internal::AlignText(horz, vert, x,y, tw,th, w,h);

    float offset = (IsUiLight()) ? -1.0f : 1.0f;

    Vec4 shadow = (IsUiLight()) ? gUiColorExLight : gUiColorBlack;
    Vec4 front = (IsUiLight()) ? gUiColorBlack : gUiColorExLight;

    if (locked || inactive)
    {
        shadow.a = .5f;
        front.a = .5f;
    }

    if (tooltip)
    {
        shadow = gUiDarkColorBlack;
        front = gUiDarkColorExLight;
        if (darken)
        {
            front = Vec4(.7f,.7f,.7f, 1);
        }
    }

    font.color = shadow;
    DrawText(font, x, y-offset, clippedText);
    font.color = front;
    DrawText(font, x, y, clippedText);

    Quad clippedBounds = Internal::GetClippedBounds(cursor.x, cursor.y, w, h);
    Vec2 mouse = GetMousePos();
    bool inside = PointInBoundsXYXY(mouse, clippedBounds);
    if (textClipped && inside) SetCurrentTooltip(text);

    Internal::AdvanceUiCursorEnd(gUiPanels.top(), w, h);
}

TEINAPI void DoLabel (UiAlign horz, UiAlign vert, float h, std::string text, Vec4 bgColor)
{
    // Important to return ceiled value otherwise the next label using the
    // cursor to position itself might overlap the previous label by 1px.
    assert(gUiFont);
    float w = ceilf(GetTextWidthScaled(*gUiFont, text));
    return DoLabel(horz, vert, w, h, text, bgColor);
}

TEINAPI void DoLabelHyperlink (UiAlign horz, UiAlign vert, float w, float h, std::string text, std::string link, std::string href, Vec4 bgColor)
{
    // Make sure that the necessary components are assigned.
    assert(gUiFont);

    Internal::AdvanceUiCursorStart(gUiPanels.top(), w, h);

    Font& font = *gUiFont;

    Vec2 cursor = Internal::GetRelativeCursor(gUiPanels.top());

    // We scissor the contents to avoid text overspill.
    BeginScissor(cursor.x, cursor.y, w, h);
    Defer { EndScissor(); };

    SetDrawColor(bgColor); // Draw the label's background.
    FillQuad(cursor.x, cursor.y, cursor.x + w, cursor.y + h);

    float tw = GetTextWidthScaled(font, text);
    float th = GetTextHeightScaled(font, text);

    // If text is a single line we calculate how much we can fit in the width
    // and if necessary trim any off and replace the end with and ellipsis.
    std::string clippedText(text);
    if (std::count(clippedText.begin(), clippedText.end(), '\n') <= 1)
    {
        if (tw > w) // Our text goes out of the label bounds.
        {
            if (clippedText.length() <= 3)
            {
                clippedText = "...";
                tw = GetTextWidthScaled(font, clippedText);
            }
            else
            {
                clippedText.replace(clippedText.length()-3, 3, "...");
                while (tw > w && clippedText.length() > 3)
                {
                    clippedText.erase(clippedText.length()-4, 1);
                    tw = GetTextWidthScaled(font, clippedText);
                }
            }
        }
    }

    // Handle setting the application's cursor to the correct graphic.
    if (Internal::IsHot())
    {
        gUiHotHyperlink = gUiCurrentID;
        SetCursorType(Cursor::POINTER);
    }
    else
    {
        if (gUiHotHyperlink == gUiCurrentID)
        {
            gUiHotHyperlink = gUiInvalidID;
        }

        // We have this check so that we can know it's okay to set the cursor back to arrow as no text box elements are hot.
        if (gUiHotHyperlink == gUiInvalidID && gUiHotTextBox == gUiInvalidID)
        {
            // NOTE: Kind of hacky to put this here, but it prevents issues with
            // the flickering of the cursor due to hyperlinks. Could be cleaned.
            if (CurrentTabIsLevel() && MouseInsideLevelEditorViewport() && IsWindowFocused("WINMAIN"))
            {
                switch (gLevelEditor.toolType)
                {
                    case (ToolType::BRUSH): SetCursorType(Cursor::BRUSH); break;
                    case (ToolType::FILL): SetCursorType(Cursor::FILL); break;
                    case (ToolType::SELECT): SetCursorType(Cursor::SELECT); break;
                }
            }
            else
            {
                SetCursorType(Cursor::ARROW);
            }
        }
    }

    float wx = cursor.x + GetTextWidthScaled(font, clippedText);
    float wy = cursor.y;
    float ww = GetTextWidthScaled(font, link);
    float wh = GetTextHeightScaled(font, link);

    if (Internal::HandleWidget(wx,wy,ww,wh, false)) LoadWebpage(href);

    float x = cursor.x;
    float y = cursor.y;

    Internal::AlignText(horz, vert, x,y, tw,th, w,h);

    float offset = (IsUiLight()) ? -1.0f : 1.0f;

    Vec4 shadow = (IsUiLight()) ? gUiColorExLight : gUiColorBlack;
    Vec4 front = (IsUiLight()) ? gUiColorBlack : gUiColorExLight;
    Vec4 linkColor = front;

    if (Internal::IsHit() || Internal::IsHot())
    {
        linkColor = (IsUiLight()) ? gUiColorExDark : gUiColorWhite;
    }

    font.color = shadow;
    DrawText(font, x, y-offset, clippedText);
    font.color = front;
    DrawText(font, x, y, clippedText);

    x += GetTextWidthScaled(font, clippedText);

    SetDrawColor(shadow);
    DrawLine(x, (y+2)-offset, x+ww, (y+2)-offset);
    SetDrawColor(linkColor);
    DrawLine(x, y+2, x+ww, y+2);

    font.color = shadow;
    DrawText(font, x, y-offset, link);
    font.color = linkColor;
    DrawText(font, x, y, link);

    Internal::AdvanceUiCursorEnd(gUiPanels.top(), w, h);

    ++gUiCurrentID;
}

TEINAPI void DoMarkdown (float w, float h, std::string text)
{
    Font& font = GetEditorRegularFont();

    UiFlag flags = gUiPanels.top().flags;

    bool inactive = (flags & UI_INACTIVE);
    bool locked = (flags & UI_LOCKED);

    Internal::AdvanceUiCursorStart(gUiPanels.top(), w, h);

    Vec2 cursor = Internal::GetRelativeCursor(gUiPanels.top());

    // We scissor the contents to avoid text overspill.
    BeginScissor(cursor.x, cursor.y, w, h);
    Defer { EndScissor(); };

    std::vector<std::string> lines;
    TokenizeString(text, "\r\n", lines);

    float x = cursor.x;
    float y = cursor.y + font.lineGap[font.currentPointSize];

    float offset = (IsUiLight()) ? -1.0f : 1.0f;

    Vec4 shadow = (IsUiLight()) ? gUiColorExLight : gUiColorBlack;
    Vec4 front = (IsUiLight()) ? gUiColorBlack : gUiColorExLight;

    if (locked || inactive)
    {
        shadow.a = .5f;
        front.a = .5f;
    }

    Internal::DoMarkdownFormatting(lines, w);

    for (auto& line: lines)
    {
        std::vector<std::string> subLines;
        TokenizeString(line, "\r\n", subLines);

        for (size_t i=0; i<subLines.size(); ++i)
        {
            x = cursor.x;
            if (i != 0) x += GetTextWidthScaled(font, ">");
            font.color = shadow;
            DrawText(font, x, y-offset, subLines.at(i));
            font.color = front;
            DrawText(font, x, y, subLines.at(i));
            y += font.lineGap[font.currentPointSize];
        }
    }

    Internal::AdvanceUiCursorEnd(gUiPanels.top(), w, h);
}

TEINAPI float GetMarkdownHeight (float w, std::string text)
{
    Font& font = GetEditorRegularFont();
    std::vector<std::string> lines;
    TokenizeString(text, "\r\n", lines);
    std::string markdownText = Internal::DoMarkdownFormatting(lines, w);
    return GetTextHeightScaled(font, markdownText);
}

TEINAPI void DoTextBox (float w, float h, UiFlag flags, std::string& text, std::string defaultText, UiAlign hAlign)
{
    // Make sure that the necessary components are assigned.
    assert(gUiFont);

    flags |= gUiPanels.top().flags;
    bool locked = (flags & UI_LOCKED);

    Internal::AdvanceUiCursorStart(gUiPanels.top(), w, h);

    Font& font = *gUiFont;

    Vec2 cursor = Internal::GetRelativeCursor(gUiPanels.top());

    if (!locked)
    {
        if (Internal::HandleWidget(cursor.x, cursor.y, w, h, locked)) {
            // If the cursor was blinking before then reset the timer.
            if (gUiCursorBlinkTimer)
            {
                SDL_RemoveTimer(gUiCursorBlinkTimer);
                gUiCursorBlinkTimer = NULL;
            }

            // Start the blinking of the cursor.
            gUiCursorVisible = true;
            gUiCursorBlinkTimer = SDL_AddTimer(gUiCursorBlinkInterval, Internal::CursorBlinkCallback, NULL);
            if (!gUiCursorBlinkTimer)
            {
                LogError(ERR_MIN, "Failed to setup cursor blink timer! (%s)", SDL_GetError());
            }

            gUiTextBoxCursor = text.length();
            gUiActiveTextBox = gUiCurrentID;

            SDL_StartTextInput();
        }
    }
    else if (gUiActiveTextBox == gUiCurrentID)
    {
        DeselectActiveTextBox(text, defaultText);
    }

    // If we are the active text box and the mouse was pressed this update
    // and we're not hit then that means the press was outside of us and
    // therefore we need to become deselected and can no longer be active.
    if (gUiActiveTextBox == gUiCurrentID && Internal::IsUiMouseLeftDown() && !Internal::IsHit())
    {
        DeselectActiveTextBox(text, defaultText);
    }
    // If the right mouse button is pressed then we just always deselect.
    if (gUiActiveTextBox == gUiCurrentID && Internal::IsUiMouseRightDown())
    {
        DeselectActiveTextBox(text, defaultText);
    }

    // Handle setting the application's cursor to the correct graphic.
    if (Internal::IsHot())
    {
        gUiHotTextBox = gUiCurrentID;
        SetCursorType(Cursor::BEAM);
    }
    else
    {
        if (gUiHotTextBox == gUiCurrentID)
        {
            gUiHotTextBox = gUiInvalidID;
        }

        // We have this check so that we can know it's okay to set
        // the cursor back to arrow as no text box elements are hot.
        if ((gUiHotTextBox == gUiInvalidID) && (gUiHotHyperlink == gUiInvalidID))
        {
            // NOTE: Kind of hacky to put this here, but it prevents issues with
            // the flickering of the cursor due to text boxes. Could be cleaned.
            if (CurrentTabIsLevel() && MouseInsideLevelEditorViewport() && IsWindowFocused("WINMAIN"))
            {
                switch (gLevelEditor.toolType)
                {
                    case (ToolType::BRUSH): SetCursorType(Cursor::BRUSH); break;
                    case (ToolType::FILL): SetCursorType(Cursor::FILL); break;
                    case (ToolType::SELECT): SetCursorType(Cursor::SELECT); break;
                }
            }
            else
            {
                SetCursorType(Cursor::ARROW);
            }
        }
    }

    if (!locked && gUiMakeNextTextBoxActive && GetRenderTarget()->id == gUiTextBoxTabWindowID)
    {
        gUiTextBoxCursor = std::string::npos;
        gUiActiveTextBox = gUiCurrentID;
        gUiMakeNextTextBoxActive = false;
        gUiTextBoxTabWindowID = 0;
    }

    Vec4 front = (IsUiLight()) ? gUiColorBlack : gUiColorExLight;
    Vec4 shadow = (IsUiLight()) ? gUiColorExLight : gUiColorBlack;
    Vec4 outline = gUiColorDark;
    Vec4 back = gUiColorMedDark;

    if (locked)
    {
        outline = gUiColorMedDark;
        back = gUiColorMedium;
        shadow.a = .5f;
        front.a = .5f;
    }

    SetDrawColor(outline); // Draw the text box's outline quad.
    FillQuad(cursor.x, cursor.y, cursor.x+w, cursor.y+h);
    SetDrawColor(back); // Draw the text box's background quad.
    FillQuad(cursor.x+1, cursor.y+1, cursor.x+w-1, cursor.y+h-1);

    constexpr float XPad = 5;
    constexpr float YPad = 2;

    float bx = cursor.x+(XPad  );
    float by = cursor.y+(YPad  );
    float bw = w       -(XPad*2);
    float bh = h       -(YPad*2);

    // Handle text input events if we are the active text box.
    if (gUiActiveTextBox == gUiCurrentID)
    {
        // Make sure that the cursor is in the bounds of the string.
        if (gUiTextBoxCursor > text.length())
        {
            gUiTextBoxCursor = text.length();
        }

        if (GetRenderTarget()->focus)
        {
            std::string oldText = text;
            size_t oldCursor = gUiTextBoxCursor;

            for (auto& textEvent: gUiTextEvents)
            {
                switch (textEvent.type)
                {
                    case (UiTextEventType::TEXT):
                    {
                        bool invalidText = false;
                        for (auto c: textEvent.text)
                        {
                            if ((flags & UI_ALPHANUM) && !isalnum(c)) invalidText = true;
                            if ((flags & UI_ALPHABETIC) && !isalpha(c)) invalidText = true;
                            if ((flags & UI_NUMERIC) && !isdigit(c)) invalidText = true;
                            if ((flags & UI_FILEPATH) && !Internal::IsValidFilePath(c)) invalidText = true;
                        }
                        if (invalidText)
                        {
                            break;
                        }
                        // Clear out the default text and enter what the user actually wants.
                        if (!defaultText.empty() && (text == defaultText))
                        {
                            gUiTextBoxCursor = 0;
                            text.clear();
                        }
                        for (auto c: textEvent.text)
                        {
                            auto pos = text.begin()+(gUiTextBoxCursor++);
                            text.insert(pos, c);
                        }
                    } break;
                    case (UiTextEventType::KEY):
                    {
                        switch (textEvent.key)
                        {
                            case (SDLK_TAB):
                            {
                                if (!gUiTabHandled)
                                {
                                    gUiMakeNextTextBoxActive = true;
                                    gUiTextBoxTabWindowID = GetRenderTarget()->id;
                                    gUiTabHandled = true;
                                }
                            } break;
                            case (SDLK_LEFT):
                            {
                                if (gUiTextBoxCursor > 0)
                                {
                                    --gUiTextBoxCursor;
                                }
                            } break;
                            case (SDLK_RIGHT): {
                                if (gUiTextBoxCursor < text.length())
                                {
                                    ++gUiTextBoxCursor;
                                }
                            } break;
                            case (SDLK_UP):
                            {
                                if (flags & UI_NUMERIC)
                                {
                                    if (atoi(text.c_str()) < INT_MAX)
                                    {
                                        text = std::to_string(atoi(text.c_str())+1);
                                        gUiTextBoxCursor = text.length();
                                    }
                                }
                            } break;
                            case (SDLK_DOWN):
                            {
                                if (flags & UI_NUMERIC)
                                {
                                    if (atoi(text.c_str()) > 0)
                                    {
                                        text = std::to_string(atoi(text.c_str())-1);
                                        gUiTextBoxCursor = text.length();
                                    }
                                }
                            } break;
                            case (SDLK_HOME):
                            {
                                gUiTextBoxCursor = 0;
                            } break;
                            case (SDLK_END):
                            {
                                gUiTextBoxCursor = text.length();
                            } break;
                            case (SDLK_BACKSPACE):
                            {
                                if (gUiTextBoxCursor != 0)
                                {
                                    text.erase(--gUiTextBoxCursor, 1);
                                }
                            } break;
                            case (SDLK_DELETE):
                            {
                                if (gUiTextBoxCursor < text.length())
                                {
                                    text.erase(gUiTextBoxCursor, 1);
                                }
                            } break;
                            case (SDLK_RETURN):
                            {
                                DeselectActiveTextBox(text, defaultText);
                            } break;
                            case (SDLK_v):
                            {
                                if (SDL_GetModState() & KMOD_CTRL)
                                {
                                    if (SDL_HasClipboardText())
                                    {
                                        char* clipboardText = SDL_GetClipboardText();
                                        if (clipboardText)
                                        {
                                            Defer { SDL_free(clipboardText); }; // Docs say we need to free!

                                            bool addText = true;
                                            std::string t(clipboardText);

                                            for (auto c: t)
                                            {
                                                if ((flags & UI_ALPHANUM) && !isalnum(c)) { addText = false; break; }
                                                if ((flags & UI_ALPHABETIC) && !isalpha(c)) { addText = false; break; }
                                                if ((flags & UI_NUMERIC) && !isdigit(c)) { addText = false; break; }
                                                if ((flags & UI_FILEPATH) && !Internal::IsValidFilePath(c)) { addText = false; break; }
                                            }

                                            if (addText)
                                            {
                                                // Clear out the default text and enter what the user actually wants.
                                                if (!defaultText.empty() && text == defaultText)
                                                {
                                                    gUiTextBoxCursor = 0;
                                                    text.clear();
                                                }

                                                text.insert(gUiTextBoxCursor, t);
                                                gUiTextBoxCursor += t.length();
                                            }
                                        }
                                    }
                                }
                            } break;
                        }
                    } break;
                }
            }

            gUiTextEvents.clear();

            // Reset the cursor blink interval.
            if (oldText != text || oldCursor != gUiTextBoxCursor)
            {
                // If the cursor was blinking before then reset the timer.
                if (gUiCursorBlinkTimer)
                {
                    SDL_RemoveTimer(gUiCursorBlinkTimer);
                    gUiCursorBlinkTimer = NULL;
                }
                // Start the blinking of the cursor.
                gUiCursorVisible = true;
                gUiCursorBlinkTimer = SDL_AddTimer(gUiCursorBlinkInterval, Internal::CursorBlinkCallback, NULL);
                if (!gUiCursorBlinkTimer)
                {
                    LogError(ERR_MIN, "Failed to setup cursor blink timer! (%s)", SDL_GetError());
                }
            }
        }

        // Cursor should always be at the end of the default text.
        if (!defaultText.empty() && text == defaultText)
        {
            gUiTextBoxCursor = text.length();
        }
    }

    // We scissor the contents to avoid text overspill.
    BeginScissor(bx,by,bw,bh);

    float tx = bx;
    float ty = by;
    float tw = GetTextWidthScaled(font, text);
    float th = GetTextHeightScaled(font, text);

    if (th == 0) th = bh;

    Internal::AlignText(hAlign, UI_ALIGN_CENTER, tx,ty, tw,th, bw,bh);

    float xOff = 0;
    float yOff = (IsUiLight()) ? -1.0f : 1.0f;

    // Adjust text position/offsetrun based on the current cursor.
    if (gUiActiveTextBox == gUiCurrentID)
    {
        if (hAlign == UI_ALIGN_LEFT)
        {
            std::string sub(text.substr(0, gUiTextBoxCursor));
            float cursorX = tx+GetTextWidthScaled(font, sub);
            if (cursorX > bx+bw)
            {
                float diff = abs(bw - GetTextWidthScaled(font, sub));
                xOff = -diff;
            }
        }
        else
        {
            std::string sub(text.substr(0, gUiTextBoxCursor));
            float cursorX = tx+GetTextWidthScaled(font, sub);
            if (cursorX < bx)
            {
                xOff = (bx - cursorX);
            }
        }
    }

    font.color = shadow;
    DrawText(font, tx+xOff, ty-yOff, text);
    font.color = front;
    DrawText(font, tx+xOff, ty, text);

    EndScissor();

    // If we're active then draw the text box cursor as well.
    if ((gUiActiveTextBox == gUiCurrentID) && gUiCursorVisible)
    {
        BeginScissor(bx-1, by-1, bw+2, bh+2);

        std::string sub(text.substr(0, gUiTextBoxCursor));
        float xo = GetTextWidthScaled(font, sub);
        float yo = (bh-th)/2; // Center the cursor vertically.
        // Just looks nicer...
        if ((gUiTextBoxCursor != 0 && text.length()) || (!text.length()))
        {
            xo += 1;
        }
        SetDrawColor((IsUiLight()) ? gUiColorBlack : gUiColorExLight);
        DrawLine(tx+xo+xOff, by+yo, tx+xo+yOff, by+yo+th);

        EndScissor();
    }

    Internal::AdvanceUiCursorEnd(gUiPanels.top(), w, h);

    ++gUiCurrentID;
}

TEINAPI void DoTextBoxLabeled (float w, float h, UiFlag flags, std::string& text, float labelWidth, std::string label, std::string defaultText, UiAlign hAlign)
{
    // Make sure that the necessary components are assigned.
    assert(gUiFont);

    float lw = labelWidth;
    float tw = w - lw;

    if (tw < 0) return; // Won't draw anything!

    // Cache this stuff because we are going to temporarily change it.
    Vec2 cursor = *gUiPanels.top().cursor;
    UiDir dir = gUiPanels.top().cursorDir;

    SetPanelCursorDir(UI_DIR_RIGHT);
    DoLabel(UI_ALIGN_LEFT, UI_ALIGN_CENTER, lw, h, label);

    SetPanelCursorDir(dir);
    DoTextBox(tw, h, flags, text, defaultText, hAlign);

    // Reset the X location of the cursor for the caller.
    if (dir == UI_DIR_UP || dir == UI_DIR_DOWN)
    {
        gUiPanels.top().cursor->x = cursor.x;
    }
}

TEINAPI void DoHotkeyRebindMain (float w, float h, UiFlag flags, KeyBinding& kb)
{
    // Make sure that the necessary components are assigned.
    assert(gUiFont);

    flags |= gUiPanels.top().flags;
    bool locked = (flags & UI_LOCKED);

    Internal::AdvanceUiCursorStart(gUiPanels.top(), w, h);

    Font& font = *gUiFont;

    Vec2 cursor = Internal::GetRelativeCursor(gUiPanels.top());

    if (!locked)
    {
        if (Internal::HandleWidget(cursor.x, cursor.y, w, h, locked))
        {
            gUiActiveHotkeyRebind = gUiCurrentID;
        }
    }
    else if (gUiActiveHotkeyRebind == gUiCurrentID)
    {
        DeselectActiveHotkeyRebind();
    }

    // If we are the active KB rebind and the mouse was pressed this update
    // and we're not hit then that means the press was outside of us and
    // therefore we need to become deselected and can no longer be active.
    if ((gUiActiveHotkeyRebind == gUiCurrentID) && Internal::IsUiMouseLeftDown() && !Internal::IsHit())
    {
        DeselectActiveHotkeyRebind();
    }
    // If the right mouse button is pressed then we just always deselect.
    if ((gUiActiveHotkeyRebind == gUiCurrentID) && Internal::IsUiMouseRightDown())
    {
        DeselectActiveHotkeyRebind();
    }

    Vec4 front = (IsUiLight()) ? gUiColorBlack : gUiColorExLight;
    Vec4 shadow = (IsUiLight()) ? gUiColorExLight : gUiColorBlack;
    Vec4 outline = gUiColorDark;
    Vec4 back = gUiColorMedDark;

    if (locked)
    {
        outline = gUiColorMedDark;
        back = gUiColorMedium;
        shadow.a = .5f;
        front.a = .5f;
    }

    SetDrawColor(outline); // Draw the rebind's outline quad.
    FillQuad(cursor.x, cursor.y, cursor.x+w, cursor.y+h);
    SetDrawColor(back); // Draw the rebind's background quad.
    FillQuad(cursor.x+1, cursor.y+1, cursor.x+w-1, cursor.y+h-1);

    constexpr float XPad = 5;
    constexpr float YPad = 2;

    float bx = cursor.x+(XPad  );
    float by = cursor.y+(YPad  );
    float bw = w       -(XPad*2);
    float bh = h       -(YPad*2);

    // If we're active then we check if the user has entered a new binding.
    if (gUiActiveHotkeyRebind == gUiCurrentID)
    {
        if (gMainEvent.type == SDL_KEYDOWN)
        {
            SDL_Keycode k = gMainEvent.key.keysym.sym;
            // We do not want the key binding to be set when just the mod is pressed!
            if (k != SDLK_LCTRL  && k != SDLK_RCTRL  &&
                k != SDLK_LALT   && k != SDLK_RALT   &&
                k != SDLK_MODE                       &&
                k != SDLK_RSHIFT && k != SDLK_LSHIFT &&
                k != SDLK_LGUI   && k != SDLK_RGUI)
            {
                kb.code = gMainEvent.key.keysym.sym;

                // Remove CAPSLOCK and NUMLOCK because we don't care about them at all.
                kb.mod = (SDL_GetModState() & ~(KMOD_NUM|KMOD_CAPS));

                // We do not care whether the right or left variants have been pressed.
                if (kb.mod&KMOD_LCTRL  || kb.mod&KMOD_RCTRL)  kb.mod |= KMOD_CTRL;
                if (kb.mod&KMOD_LALT   || kb.mod&KMOD_RALT)   kb.mod |= KMOD_ALT;
                if (kb.mod&KMOD_LSHIFT || kb.mod&KMOD_RSHIFT) kb.mod |= KMOD_SHIFT;
                if (kb.mod&KMOD_LGUI   || kb.mod&KMOD_RGUI)   kb.mod |= KMOD_GUI;

                DeselectActiveHotkeyRebind();
            }
        }
    }

    // We scissor the contents to avoid text overspill.
    BeginScissor(bx,by,bw,bh);

    // The text to display depends on if we're active or not.
    std::string text;
    if (gUiActiveHotkeyRebind == gUiCurrentID)
    {
        text = "Enter new key binding...";
    }
    else
    {
        text = GetKeyBindingMainString(kb);
    }

    // Calculate the position of the text and draw it
    float tx = bx;
    float ty = by;
    float tw = GetTextWidthScaled(font, text);
    float th = GetTextHeightScaled(font, text);

    Internal::AlignText(UI_ALIGN_RIGHT, UI_ALIGN_CENTER, tx,ty, tw,th, bw,bh);

    float offset = (IsUiLight()) ? -1.0f : 1.0f;

    font.color = shadow;
    DrawText(font, tx, ty-offset, text);
    font.color = front;
    DrawText(font, tx, ty, text);

    EndScissor();

    Internal::AdvanceUiCursorEnd(gUiPanels.top(), w, h);

    ++gUiCurrentID;
}

TEINAPI void DoHotkeyRebindAlt (float w, float h, UiFlag flags, KeyBinding& kb)
{
    // Make sure that the necessary components are assigned.
    assert(gUiFont);

    // Cache the rebind's flags so they are easily accessible.
    flags |= gUiPanels.top().flags;
    bool locked = (flags & UI_LOCKED);

    Internal::AdvanceUiCursorStart(gUiPanels.top(), w, h);

    Font& font = *gUiFont;

    Vec2 cursor = Internal::GetRelativeCursor(gUiPanels.top());

    if (!locked)
    {
        if (Internal::HandleWidget(cursor.x, cursor.y, w, h, locked))
        {
            gUiActiveHotkeyRebind = gUiCurrentID;
        }
    }
    else if (gUiActiveHotkeyRebind == gUiCurrentID)
    {
        DeselectActiveHotkeyRebind();
    }

    // If we are the active KB rebind and the mouse was pressed this update
    // and we're not hit then that means the press was outside of us and
    // therefore we need to become deselected and can no longer be active.
    if ((gUiActiveHotkeyRebind == gUiCurrentID) && Internal::IsUiMouseLeftDown() && !Internal::IsHit())
    {
        DeselectActiveHotkeyRebind();
    }
    // If the right mouse button is pressed then we just always deselect.
    if ((gUiActiveHotkeyRebind == gUiCurrentID) && Internal::IsUiMouseRightDown())
    {
        DeselectActiveHotkeyRebind();
    }

    Vec4 front = (IsUiLight()) ? gUiColorBlack : gUiColorExLight;
    Vec4 shadow = (IsUiLight()) ? gUiColorExLight : gUiColorBlack;
    Vec4 outline = gUiColorDark;
    Vec4 back = gUiColorMedDark;

    if (locked)
    {
        outline = gUiColorMedDark;
        back = gUiColorMedium;
        shadow.a = .5f;
        front.a = .5f;
    }

    SetDrawColor(outline); // Draw the rebind's outline quad.
    FillQuad(cursor.x, cursor.y, cursor.x+w, cursor.y+h);
    SetDrawColor(back); // Draw the rebind's background quad.
    FillQuad(cursor.x+1, cursor.y+1, cursor.x+w-1, cursor.y+h-1);

    constexpr float XPad = 5;
    constexpr float YPad = 2;

    float bx = cursor.x+(XPad  );
    float by = cursor.y+(YPad  );
    float bw = w       -(XPad*2);
    float bh = h       -(YPad*2);

    // If we're active then we check if the user has entered a new binding.
    if (gUiActiveHotkeyRebind == gUiCurrentID)
    {
        if (gMainEvent.type == SDL_KEYDOWN)
        {
            SDL_Keycode k = gMainEvent.key.keysym.sym;
            // We do not want the key binding to be set when just the mod is pressed!
            if (k != SDLK_LCTRL  && k != SDLK_RCTRL  &&
                k != SDLK_LALT   && k != SDLK_RALT   &&
                k != SDLK_MODE                       &&
                k != SDLK_RSHIFT && k != SDLK_LSHIFT &&
                k != SDLK_LGUI   && k != SDLK_RGUI)
            {
                kb.altCode = gMainEvent.key.keysym.sym;

                // Remove CAPSLOCK and NUMLOCK because we don't care about them at all.
                kb.altMod = (SDL_GetModState() & ~(KMOD_NUM|KMOD_CAPS));

                // We do not care whether the right or left variants have been pressed.
                if ((kb.altMod & KMOD_LCTRL ) || (kb.altMod & KMOD_RCTRL )) kb.altMod |= KMOD_CTRL;
                if ((kb.altMod & KMOD_LALT  ) || (kb.altMod & KMOD_RALT  )) kb.altMod |= KMOD_ALT;
                if ((kb.altMod & KMOD_LSHIFT) || (kb.altMod & KMOD_RSHIFT)) kb.altMod |= KMOD_SHIFT;
                if ((kb.altMod & KMOD_LGUI  ) || (kb.altMod & KMOD_RGUI  )) kb.altMod |= KMOD_GUI;

                DeselectActiveHotkeyRebind();

                kb.hasAlt = true;
            }
        }
    }

    // We scissor the contents to avoid text overspill.
    BeginScissor(bx,by,bw,bh);

    // The text to display depends on if we're active or not.
    std::string text;
    if (gUiActiveHotkeyRebind == gUiCurrentID)
    {
        text = "Enter new key binding...";
    }
    else
    {
        text = GetKeyBindingAltString(kb);
    }

    // Calculate the position of the text and draw it
    float tx = bx;
    float ty = by;
    float tw = GetTextWidthScaled(font, text);
    float th = GetTextHeightScaled(font, text);

    Internal::AlignText(UI_ALIGN_RIGHT, UI_ALIGN_CENTER, tx,ty, tw,th, bw,bh);

    float offset = (IsUiLight()) ? -1.0f : 1.0f;

    font.color = shadow;
    DrawText(font, tx, ty-offset, text);
    font.color = front;
    DrawText(font, tx, ty, text);

    EndScissor();

    Internal::AdvanceUiCursorEnd(gUiPanels.top(), w, h);

    ++gUiCurrentID;
}

/* -------------------------------------------------------------------------- */

TEINAPI void DoIcon (float w, float h, Texture& texture, const Quad* clip)
{
    UiID flags = gUiPanels.top().flags;

    bool inactive = (flags & UI_INACTIVE);
    bool locked = (flags & UI_LOCKED);

    Internal::AdvanceUiCursorStart(gUiPanels.top(), w, h);

    Vec2 cursor = Internal::GetRelativeCursor(gUiPanels.top());

    // We scissor the contents to avoid image overspill.
    BeginScissor(cursor.x, cursor.y, w, h);
    Defer { EndScissor(); };

    Vec4 front = (IsUiLight()) ? Vec4(.4f,.4f,.4f, 1) : Vec4(.73f,.73f,.73f, 1);
    Vec4 shadow = (IsUiLight()) ? Vec4(.9f,.9f,.9f, 1) : Vec4(.16f,.16f,.16f, 1);

    if (locked || inactive)
    {
        shadow.a = .5f;
        front.a = .5f;
    }

    UiDir dir = gUiPanels.top().cursorDir;

    // Center the image within the space.
    float x = roundf(cursor.x + (w / 2) + ((dir == UI_DIR_LEFT) ? 1 : 0));
    float y = roundf(cursor.y + (h / 2) + ((dir == UI_DIR_UP)   ? 1 : 0));

    float offset = (IsUiLight()) ? -1.0f : 1.0f;

    texture.color = shadow;
    DrawTexture(texture, x, y-offset, clip);
    texture.color = front;
    DrawTexture(texture, x, y, clip);

    Internal::AdvanceUiCursorEnd(gUiPanels.top(), w, h);
}

TEINAPI void DoQuad (float w, float h, Vec4 color)
{
    UiID flags = gUiPanels.top().flags;

    Internal::AdvanceUiCursorStart(gUiPanels.top(), w, h);

    Vec2 cursor = Internal::GetRelativeCursor(gUiPanels.top());

    bool inactive = (flags & UI_INACTIVE);
    bool locked = (flags & UI_LOCKED);

    if (locked || inactive) color.a = .5f;

    SetDrawColor(color);
    FillQuad(cursor.x, cursor.y, cursor.x+w, cursor.y+h);

    Internal::AdvanceUiCursorEnd(gUiPanels.top(), w, h);
}

TEINAPI void DoSeparator (float size)
{
    float w = (gUiPanels.top().cursorDir == UI_DIR_RIGHT || gUiPanels.top().cursorDir == UI_DIR_LEFT) ? 0 : size;
    float h = (gUiPanels.top().cursorDir == UI_DIR_UP    || gUiPanels.top().cursorDir == UI_DIR_DOWN) ? 0 : size;

    Internal::AdvanceUiCursorStart(gUiPanels.top(), 1, 1);
    Internal::DrawSeparator(Internal::GetRelativeCursor(gUiPanels.top()), gUiPanels.top().cursorDir, w, h, gUiColorMedDark);
    Internal::AdvanceUiCursorEnd(gUiPanels.top(), 1, 1);
}

TEINAPI void DoScrollbar (Quad bounds, float contentHeight, float& scrollOffset)
{
    DoScrollbar(bounds.x, bounds.y, bounds.w, bounds.h, contentHeight, scrollOffset);
}

TEINAPI void DoScrollbar (float x, float y, float w, float h, float contentHeight, float& scrollOffset)
{
    // Allows scrollbars to be outside the panel they are scrolling.
    SetViewport(0, 0, GetRenderTargetWidth(), GetRenderTargetHeight());
    Defer { SetViewport(gUiPanels.top().viewport); };

    x += gUiPanels.top().viewport.x;
    y += gUiPanels.top().viewport.y;

    // We scissor the contents to avoid any overspill.
    BeginScissor(x,y,w,h);
    Defer { EndScissor(); };

    constexpr float Pad = 1;

    float bx = x +  Pad;
    float by = y +  Pad;
    float bw = w - (Pad*2);
    float bh = h;

    // Determine the percentage of content visible.
    float percentVisible = GetPanelHeight() / contentHeight;
    if (percentVisible > 1) percentVisible = 1;

    // Represent this amount in the scrollbar.
    bh = (bh*percentVisible) - (Pad*2);

    // Ensure the normalized scroll offset is in bounds.
    float ndcHeight = (h-bh-(Pad*2)) / (h-(Pad*2));
    scrollOffset = std::clamp(scrollOffset, 0.0f, ndcHeight);

    // Convert the normalized scroll offset into pixel offset.
    by += scrollOffset * (h-(Pad*2));

    Internal::HandleWidget(bx,by, bw,bh, false);

    // Adjust the offset by however much the mouse has moved.
    if (Internal::IsHit())
    {
        scrollOffset += (gUiMouseRelative.y / (h-(Pad*2)));
        scrollOffset = std::clamp(scrollOffset, 0.0f, ndcHeight);
    }

    SetDrawColor(gUiColorExDark);
    FillQuad(x,y,x+w,y+h);

    Vec4 color = gUiColorMedDark;

    if (Internal::IsHit()) color = gUiColorMedLight;
    else if (Internal::IsHot()) color = gUiColorMedium;

    SetDrawColor(color);
    FillQuad(bx,by,bx+bw,by+bh);

    // Draw the three lines on the scrollbar (avoid overspill).
    //
    // We do not bother though if it's really small because then
    // adding these lines just looks sort of gross and cluttered.
    if (bh > 10)
    {
        constexpr float LineGap = 2;

        BeginScissor(bx,by,bw,bh);
        SetDrawColor(gUiColorExDark);

        float y1 = by+(bh/2) - LineGap;
        float y2 = by+(bh/2);
        float y3 = by+(bh/2) + LineGap;

        DrawLine(bx+1, y1, bx+bw-1, y1);
        DrawLine(bx+1, y2, bx+bw-1, y2);
        DrawLine(bx+1, y3, bx+bw-1, y3);

        EndScissor();
    }

    // Set the panels relative offset to apply the actual scroll.
    //
    // We do this extra check stuff because towards the end of
    // scrolling there can be offset issues which result in
    // moving past the content bounds so we just perform manual
    // adjustment of the scroll in order to correct this issue.
    float finalOffset = roundf(contentHeight * scrollOffset);
    float resultingHeight = contentHeight - finalOffset;
    if (scrollOffset != 0 && resultingHeight < GetPanelHeight())
    {
        float difference = GetPanelHeight() - resultingHeight;
        gUiPanels.top().relativeOffset.y -= (finalOffset - difference);
    }
    else
    {
        gUiPanels.top().relativeOffset.y -= roundf(contentHeight * scrollOffset);
    }

    ++gUiCurrentID;
}

TEINAPI void BeginPanelGradient (float x, float y, float w, float h, UiFlag flags, Vec4 leftColor, Vec4 rightColor)
{
    Panel panel;

    // The method of adding a new panel varies depending on whether the panel
    // is a child to an existing panel or if it is a lone panel in the window.
    panel.absoluteBounds = { x,y,w,h };
    if (gUiPanels.size() > 0)
    {
        const Quad& pAbsolute = gUiPanels.top().absoluteBounds;
        const Vec2& pOffset = gUiPanels.top().relativeOffset;
        const Quad& pViewport = gUiPanels.top().viewport;

        Quad& cAbsolute = panel.absoluteBounds;
        Vec2& cOffset = panel.relativeOffset;
        Quad& cViewport = panel.viewport;

        cAbsolute.x += pAbsolute.x + pOffset.x;
        cAbsolute.y += pAbsolute.y + pOffset.y;

        cViewport = cAbsolute;

        // We also clip the panel's viewport to be inside of the
        // parent panel to avoid issues with overlapping/spill.
        float dX = cViewport.x - pViewport.x;
        float dY = cViewport.y - pViewport.y;

        if (cViewport.x < pViewport.x) cViewport.x = pViewport.x, cViewport.w -= roundf(abs(dX)), dX = cViewport.x - pViewport.x;
        if (cViewport.y < pViewport.y) cViewport.y = pViewport.y, cViewport.h -= roundf(abs(dY)), dY = cViewport.y - pViewport.y;

        if (cViewport.x+cViewport.w > pViewport.x+pViewport.w) cViewport.w = pViewport.w - roundf(abs(dX));
        if (cViewport.y+cViewport.h > pViewport.y+pViewport.h) cViewport.h = pViewport.h - roundf(abs(dY));

        if (cViewport.w < 0) cViewport.w = 0;
        if (cViewport.h < 0) cViewport.h = 0;

        // And determine the panel's offset to its viewport.
        cOffset.x = cAbsolute.x - cViewport.x;
        cOffset.y = cAbsolute.y - cViewport.y;

        // Inherit the parent panel's flags.
        panel.flags = flags | gUiPanels.top().flags;
    }
    else
    {
        panel.viewport = panel.absoluteBounds;
        panel.relativeOffset = Vec2(0,0);
        panel.flags = flags;
    }

    panel.cursor = NULL;
    panel.cursorDir = UI_DIR_RIGHT;
    panel.cursorAdvanceEnabled = true;

    SetViewport(panel.viewport);
    gUiPanels.push(panel);

    BeginDraw(BufferMode::TRIANGLE_STRIP);
    PutVertex(0,                panel.viewport.h, leftColor); // BL
    PutVertex(0,                0,                leftColor); // TL
    PutVertex(panel.viewport.w, panel.viewport.h, rightColor); // BR
    PutVertex(panel.viewport.w, 0,                rightColor); // TR
    EndDraw();
}

TEINAPI void BeginPanelGradient (Quad bounds, UiFlag flags, Vec4 leftColor, Vec4 rightColor)
{
    BeginPanelGradient(bounds.x, bounds.y, bounds.w, bounds.h, flags, leftColor, rightColor);
}

TEINAPI bool BeginClickPanelGradient (UiAction action, float w, float h, UiFlag flags, std::string info)
{
    Panel& parent = gUiPanels.top();

    Vec2 relCursor = Internal::GetRelativeCursor(parent);
    Vec2 cursor = Internal::GetCursor(parent);

    bool locked = (flags & UI_LOCKED);
    bool highlight = (flags & UI_HIGHLIGHT);

    bool result = Internal::HandleWidget(relCursor.x, relCursor.y, w, h, locked);
    if (result && action) action(); // Make sure action is valid!

    Vec4 bgLeft = gUiColorMedium;
    Vec4 bgRight = gUiColorMedium;

    if      (locked)            bgLeft = gUiColorMedDark, bgRight = gUiColorMedDark;
    else if (Internal::IsHit()) bgLeft = gUiColorDark;
    else if (Internal::IsHot()) bgLeft = gUiColorMedLight;

    BeginPanelGradient(cursor.x, cursor.y, w, h, flags, bgLeft, bgRight);
    Internal::AdvanceUiCursorStart(parent, w, h);

    if (highlight && !Internal::IsHit())
    {
        Vec4 color = gUiColorMedLight;
        color.a = .66f;
        SetDrawColor(color);
        FillQuad(0, 0, GetViewport().w, GetViewport().h);
    }
    if (highlight && Internal::IsHit())
    {
        Vec4 color = gUiColorMedLight;
        color.a = .66f;
        BeginDraw(BufferMode::TRIANGLE_STRIP);
        PutVertex(0,               GetViewport().h, Vec4(0,0,0,0)); // BL
        PutVertex(0,               0,               Vec4(0,0,0,0)); // TL
        PutVertex(GetViewport().w, GetViewport().h,         color); // BR
        PutVertex(GetViewport().w, 0,                       color); // TR
        EndDraw();
    }

    Vec4 separatorColor = (locked) ? gUiColorDark : gUiColorMedDark;
    Vec2 offset = gUiPanels.top().relativeOffset;

    Internal::DrawSeparator(offset, parent.cursorDir, w, h, separatorColor);
    Internal::AdvanceUiCursorEnd(parent, w, h);

    // If we are currently hot then we push our info to the status bar.
    if (!locked && !info.empty() && Internal::IsHot())
    {
        PushStatusBarMessage(info.c_str());
    }

    ++gUiCurrentID;

    return result;
}

TEINAPI bool DoImageButtonGradient (UiAction action, float w, float h, UiFlag flags, const Quad* clip, std::string info, std::string kb, std::string name)
{
    // Make sure that the necessary components are assigned.
    assert(gUiTexture);

    flags |= gUiPanels.top().flags;

    bool inactive = (flags & UI_INACTIVE);
    bool locked = (flags & UI_LOCKED);
    bool highlight = (flags & UI_HIGHLIGHT);

    Internal::AdvanceUiCursorStart(gUiPanels.top(), w, h);

    Texture& texture = *gUiTexture;

    Vec2 cursor = Internal::GetRelativeCursor(gUiPanels.top());

    // We scissor the contents to avoid image overspill.
    BeginScissor(cursor.x, cursor.y, w, h);
    Defer { EndScissor(); };

    // Locked buttons cannot be interacted with.
    bool result = Internal::HandleWidget(cursor.x, cursor.y, w, h, locked);
    if (result && action) action(); // Make sure action is valid!

    Vec4 front = (IsUiLight()) ? Vec4(.4f,.4f,.4f, 1) : Vec4(.73f,.73f,.73f, 1);
    Vec4 bgLeft = gUiColorMedium;
    Vec4 bgRight = gUiColorMedium;
    Vec4 shadow = (IsUiLight()) ? Vec4(.9f,.9f,.9f, 1) : Vec4(.16f,.16f,.16f, 1);

    if      (locked)            bgRight = gUiColorMedDark, bgLeft = gUiColorMedDark;
    else if (Internal::IsHit()) bgRight = gUiColorDark;
    else if (Internal::IsHot()) bgRight = gUiColorMedLight;

    if (locked || inactive)
    {
        shadow.a = .5f;
        front.a = .5f;
    }

    // Draw the button's background quad.
    BeginDraw(BufferMode::TRIANGLE_STRIP);
    PutVertex(cursor.x,   cursor.y+h, bgLeft); // BL
    PutVertex(cursor.x,   cursor.y,   bgLeft); // TL
    PutVertex(cursor.x+w, cursor.y+h, bgRight); // BR
    PutVertex(cursor.x+w, cursor.y,   bgRight); // TR
    EndDraw();

    if (highlight && !Internal::IsHit())
    {
        Vec4 color = gUiColorMedLight;
        color.a = .66f;
        SetDrawColor(color);
        FillQuad(0, 0, GetViewport().w, GetViewport().h);
    }
    if (highlight && Internal::IsHit())
    {
        Vec4 color = gUiColorMedLight;
        color.a = .66f;
        BeginDraw(BufferMode::TRIANGLE_STRIP);
        PutVertex(cursor.x,   cursor.y+h,         color); // BL
        PutVertex(cursor.x,   cursor.y,           color); // TL
        PutVertex(cursor.x+w, cursor.y+h, Vec4(0,0,0,0)); // BR
        PutVertex(cursor.x+w, cursor.y,   Vec4(0,0,0,0)); // TR
        EndDraw();
    }

    // The ((w)-1) and ((h)-1) are used to ensure the separator does
    // not mess with the centering of the image based on direction.

    UiDir dir = gUiPanels.top().cursorDir;

    float w2 = (dir == UI_DIR_RIGHT || dir == UI_DIR_LEFT) ? ((w)-1) : (w);
    float h2 = (dir == UI_DIR_UP    || dir == UI_DIR_DOWN) ? ((h)-1) : (h);

    // Center the image within the button.
    float x = roundf(cursor.x + (w2 / 2) + ((dir == UI_DIR_LEFT) ? 1 : 0));
    float y = roundf(cursor.y + (h2 / 2) + ((dir == UI_DIR_UP)   ? 1 : 0));

    float offset = (IsUiLight()) ? -1.0f : 1.0f;

    texture.color = shadow;
    DrawTexture(texture, x, y-offset, clip);
    texture.color = front;
    DrawTexture(texture, x, y, clip);

    Internal::DrawSeparator(Internal::GetRelativeCursor(gUiPanels.top()), gUiPanels.top().cursorDir, w, h, gUiColorMedDark);
    Internal::AdvanceUiCursorEnd(gUiPanels.top(), w, h);

    // If we are currently hot then we push our info to the status bar.
    if (!locked && !info.empty() && Internal::IsHot())
    {
        std::string kbInfo;
        if (kb.empty())
        {
            kbInfo = FormatString("(%s)", GetKeyBindingMainString(kb).c_str());
            if (GetKeyBinding(kb).altCode && GetKeyBinding(kb).altMod)
            {
                kbInfo += FormatString(" or (%s)", GetKeyBindingAltString(kb).c_str());
            }
        }
        std::string infoText((kbInfo.empty()) ? info : FormatString("%s %s", kbInfo.c_str(), info.c_str()));
        PushStatusBarMessage(infoText.c_str());
    }
    // If we are currently hot then set the tooltip.
    if (!locked && !name.empty() && Internal::IsHot())
    {
        SetCurrentTooltip(name);
    }

    ++gUiCurrentID;

    return result;
}
