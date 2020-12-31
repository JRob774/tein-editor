static constexpr size_t gTooltipMaxLineLength = 96;
static constexpr float gTooltipAppearTime = .5f;

static std::string gTooltipName;
static std::string gTooltipDesc;

static bool gTooltipSetThisUpdate;
static bool gTooltipVisible;

static SDL_TimerID gTooltipTimer;

namespace Internal
{
    TEINAPI U32 TooltipCallback (U32 interval, void* userData)
    {
        PushEditorEvent(EditorEvent::ShowTooltip, NULL, NULL);
        return 0;
    }

    TEINAPI void ResetTooltip ()
    {
        gTooltipName = "";
        gTooltipDesc = "";
        gTooltipVisible = false;
    }
}

TEINAPI void SetCurrentTooltip (std::string name, std::string desc)
{
    // This is how we know we are still hovered over the tooltip item and don't need to hide it.
    if (gTooltipName == name && gTooltipDesc == desc)
    {
        gTooltipSetThisUpdate = true;
        return;
    }
    if (gTooltipTimer)
    {
        SDL_RemoveTimer(gTooltipTimer);
    }

    Internal::ResetTooltip();

    gTooltipName = name;
    gTooltipDesc = desc;

    gTooltipTimer = SDL_AddTimer(static_cast<U32>(gTooltipAppearTime*1000), Internal::TooltipCallback, NULL);
    if (!gTooltipTimer) LogError(ErrorLevel::Min, "Failed to setup tooltip timer! (%s)", SDL_GetError());

    gTooltipSetThisUpdate = true;
}

TEINAPI void DoTooltip ()
{
    if (!gEditorSettings.showTooltips) return;

    if (!gTooltipSetThisUpdate)
    {
        Internal::ResetTooltip();
    }
    if (gTooltipVisible)
    {
        // Word wrap the description if it is too large for the tooltip box.
        std::string desc = gTooltipDesc;
        size_t maxWidth = std::max<size_t>(gTooltipMaxLineLength, gTooltipName.length());
        int lineCount = 1;
        if (maxWidth < desc.length())
        {
            size_t offset = 0;
            for (size_t pos=0; pos!=std::string::npos; pos=desc.find(' ', pos))
            {
                if (lineCount <= (pos-offset))
                {
                    desc.at(pos) = '\n';
                    offset = pos;
                    lineCount++;
                }
                ++pos;
            }
        }

        Font& font = GetEditorRegularFont();
        SetUiFont(&font);

        constexpr float XPad = 4;
        constexpr float YPad = 8;

        float xPad = XPad;
        float yPad = YPad;

        if (desc.empty()) yPad /= 2; // Looks nicer!

        float nw = GetTextWidthScaled(font, gTooltipName);
        float nh = GetTextHeightScaled(font, gTooltipName);
        float dw = GetTextWidthScaled(font, desc);
        float dh = GetTextHeightScaled(font, desc);

        Vec2 mouse = GetMousePos();

        float tx = mouse.x;
        float ty = mouse.y + 16; // Some constant just so it doesn't get covered by the mouse.
        float tw = std::max<float>(nw,dw) + (xPad*2);
        float th = (nh+dh)                + (yPad*2);

        // Make sure that the tooltip is always on-screen and not out-of-bounds.
        if (tx+tw >= GetRenderTargetWidth()) tx = mouse.x - tw;
        if (ty+th >= GetRenderTargetHeight()) ty = mouse.y - th;

        BeginPanel(tx,ty,tw,th, UiFlag::None, Vec4(0,0,0,.8f));

        Vec2 cursor(xPad, yPad);

        SetPanelCursor(&cursor);
        SetPanelCursorDir(UiDir::Down);

        // The set panel flags are just a hack to get the text drawing nicely in the tooltip box..
        SetPanelFlags(UiFlag::Tooltip);
        DoLabel(UiAlign::Left,UiAlign::Center, tw, nh, gTooltipName);
        SetPanelFlags(UiFlag::Tooltip|UiFlag::Darken);
        DoLabel(UiAlign::Left,UiAlign::Top, tw, dh+yPad, desc);

        EndPanel();
    }
}

TEINAPI void HandleTooltipEvents ()
{
    gTooltipSetThisUpdate = false;

    if (gMainEvent.type == SDL_USEREVENT)
    {
        if (gMainEvent.user.code == static_cast<U32>(EditorEvent::ShowTooltip))
        {
            gTooltipVisible = true;
        }
    }
}
