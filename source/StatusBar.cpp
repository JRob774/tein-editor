static constexpr const char* gStatusBarDefaultMessage = "Hover over items and widgets for more information.";
static constexpr float gStatusBarLabelWidth = .125f; // NDC!

static std::string gStatusBarPushedMessage;

TEINAPI void PushStatusBarMessage (const char* format, ...)
{
    va_list args;
    va_start(args, format);
    gStatusBarPushedMessage = FormatStringV(format, args);
    va_end(args);
}

TEINAPI void DoStatusBar ()
{
    // If there is a message that has been pushed we display that, if
    // not then we just display a default message for the status bar.
    std::string message = gStatusBarDefaultMessage;
    if (!gStatusBarPushedMessage.empty())
    {
        message = gStatusBarPushedMessage;
    }

    Vec2 cursor(gStatusBarInnerPad, 0);

    float x = GetToolbarWidth() + 1;
    float y = GetViewport().h - gStatusBarHeight;
    float w = GetViewport().w - GetToolbarWidth() - (GetControlPanelWidth()) - 2;
    float h = gStatusBarHeight;

    // To account for the control panel and toolbar disappearing.
    if (!CurrentTabIsLevel()) w += 1;
    if (!AreThereAnyTabs()) x -= 1, w += 1;

    float statusBarWidth = w - (gStatusBarInnerPad * 2);
    float advance = gStatusBarInnerPad;

    float l2Width = roundf(statusBarWidth * gStatusBarLabelWidth); // Mouse.
    float l3Width = roundf(statusBarWidth * gStatusBarLabelWidth); // Select.

    // Get the mouse position.
    int mx = 0, my = 0;
    if (IsWindowFocused("WINMAIN"))
    {
        if (CurrentTabIsLevel())
        {
            if (PointInBoundsXYWH(GetMousePos(), gLevelEditor.viewport))
            {
                mx = static_cast<int>(gLevelEditor.mouseTile.x);
                my = static_cast<int>(gLevelEditor.mouseTile.y);
            }
        }
        else if (CurrentTabIsMap())
        {
            if (PointInBoundsXYWH(GetMousePos(), gMapEditor.viewport))
            {
                mx = static_cast<int>(gMapEditor.mouseTile.x);
                my = static_cast<int>(gMapEditor.mouseTile.y);
            }
        }
    }

    // Get the select bounds.
    int sx = 0, sy = 0, sw = 0, sh = 0;
    if (CurrentTabIsLevel() && AreAnySelectBoxesVisible())
    {
        int l,t,r,b;
        GetTotalSelectBoundary(&l,&t,&r,&b);
        sx = l, sy = b, sw = (r-l)+1, sh = (t-b)+1;
    }
    else if (CurrentTabIsMap() && MapSelectBoxPresent())
    {
        int l,t,r,b;
        GetMapSelectBounds(&l,&t,&r,&b);
        sx = l, sy = b, sw = (r-l)+1, sh = (t-b)+1;
    }

    std::string mouseString = FormatString("Position (%d,%d)", mx,my);
    std::string selectString = FormatString("Selection (%d,%d,%d,%d)", sx,sy,sw,sh);

    // We ensure that the mouse and select labels are always big enough to
    // show their entire content and they take priority over the tool-tip.
    float l2TextWidth = GetTextWidthScaled(GetEditorRegularFont(), mouseString);
    if (l2Width < l2TextWidth) l2Width = l2TextWidth;
    float l3TextWidth = GetTextWidthScaled(GetEditorRegularFont(), selectString);
    if (l3Width < l3TextWidth) l3Width = l3TextWidth;

    // Now we can calculate how much space is left for the tool-tip label.
    float l1Width = (statusBarWidth - (l2Width + l3Width)) - (advance * 2);

    SetUiFont(&GetEditorRegularFont());

    // Status bar is a horizontal list of editor status information.
    BeginPanel(x,y,w,h, UiFlag::None, gUiColorMedium);

    SetPanelCursor(&cursor);
    SetPanelCursorDir(UiDir::Right);

    DoLabel(UiAlign::Left,  UiAlign::Center, l1Width, h, message);
    AdvancePanelCursor(gStatusBarInnerPad);
    DoLabel(UiAlign::Right, UiAlign::Center, l2Width, h, mouseString);
    AdvancePanelCursor(gStatusBarInnerPad);
    DoLabel(UiAlign::Right, UiAlign::Center, l3Width, h, selectString);

    EndPanel();

    // Reset the pushed status bar message for the next update.
    gStatusBarPushedMessage.clear();
}
