static constexpr float gNewVerticalFrameHeight = 26;

static constexpr float gNewXPad = 8;
static constexpr float gNewYPad = 8;

static constexpr float gNewTextBoxHeight = 20;

static constexpr const char* gNewWidthLabel = "Level Width:  ";
static constexpr const char* gNewHeightLabel = "Level Height:  ";

static int gCurrentNewWidth = static_cast<int>(gDefaultLevelWidth);
static int gCurrentNewHeight = static_cast<int>(gDefaultLevelHeight);

static TabType gCurrentTabType = TabType::LEVEL;

namespace Internal
{
    TEINAPI void OkayNew ()
    {
        if (gCurrentNewWidth < gMinimumLevelWidth || gCurrentNewHeight < gMinimumLevelHeight)
        {
            ShowAlert("Warning", FormatString("Minimum level size is %dx%d!", gMinimumLevelWidth, gMinimumLevelHeight), ALERT_TYPE_WARNING, ALERT_BUTTON_OK, "WINNEW");
            return;
        }
        switch (gCurrentTabType)
        {
            case (TabType::LEVEL): CreateNewLevelTabAndFocus(GetNewWidth(), GetNewHeight()); break;
            case (TabType::MAP): CreateNewMapTabAndFocus(); break;
        }
        HideWindow("WINNEW");
    }
}

TEINAPI void OpenNew ()
{
    gCurrentNewWidth = static_cast<int>(gDefaultLevelWidth);
    gCurrentNewHeight = static_cast<int>(gDefaultLevelHeight);

    // Default to level because people make more levels than they do maps.
    gCurrentTabType = TabType::LEVEL;

    ShowWindow("WINNEW");
}

TEINAPI void DoNew ()
{
    Quad p1, p2;

    p1.x = gWindowBorder;
    p1.y = gWindowBorder;
    p1.w = GetViewport().w - (gWindowBorder * 2);
    p1.h = GetViewport().h - (gWindowBorder * 2);

    SetUiFont(&GetEditorRegularFont());

    BeginPanel(p1, UI_NONE, gUiColorExDark);

    Vec2 cursor;

    float nvfh = gNewVerticalFrameHeight;

    float vw = GetViewport().w;
    float vh = GetViewport().h;

    float bw = roundf(vw / 2);
    float bh = nvfh - gWindowBorder;

    // Top tabs for switching type of file to create.
    cursor = Vec2(0,0);
    BeginPanel(0, 0, vw, nvfh, UI_NONE, gUiColorMedium);

    SetPanelCursorDir(UI_DIR_RIGHT);
    SetPanelCursor(&cursor);

    UiFlag levelFlags = ((gCurrentTabType == TabType::LEVEL) ? UI_HIGHLIGHT : UI_INACTIVE);
    UiFlag mapFlags = ((gCurrentTabType == TabType::MAP) ? UI_HIGHLIGHT : UI_INACTIVE);

    if (DoTextButton(NULL, bw,bh, levelFlags, "Level")) gCurrentTabType = TabType::LEVEL;
    if (DoTextButton(NULL, bw,bh, mapFlags, "World Map")) gCurrentTabType = TabType::MAP;

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

    if (DoTextButton(NULL, bw,bh, UI_NONE, "Create")) Internal::OkayNew();
    if (DoTextButton(NULL, bw2,bh, UI_NONE, "Cancel")) CancelNew();

    // Add a separator to the left for symmetry.
    cursor.x = 1;
    DoSeparator(bh);

    EndPanel();

    p2.x =                    1;
    p2.y = nvfh             + 1;
    p2.w = vw               - 2;
    p2.h = vh - p2.y - nvfh - 1;

    UiFlag panelFlags = ((gCurrentTabType == TabType::LEVEL) ? UI_NONE : UI_LOCKED);
    BeginPanel(p2, panelFlags, gUiColorMedium);

    cursor = Vec2(gNewXPad, gNewYPad);

    SetPanelCursorDir(UI_DIR_DOWN);
    SetPanelCursor(&cursor);

    float labelWidthWidth = GetTextWidthScaled(GetEditorRegularFont(), gNewWidthLabel);
    float labelHeightWidth = GetTextWidthScaled(GetEditorRegularFont(), gNewHeightLabel);

    float textBoxWidth = (vw-(gNewXPad*2));
    float labelWidth = std::max(labelWidthWidth, labelHeightWidth);

    std::string widthString(std::to_string(gCurrentNewWidth));
    std::string heightString(std::to_string(gCurrentNewHeight));

    DoTextBoxLabeled(textBoxWidth, gNewTextBoxHeight, UI_NUMERIC, widthString, labelWidth, gNewWidthLabel, "0");
    AdvancePanelCursor(gNewYPad);
    DoTextBoxLabeled(textBoxWidth, gNewTextBoxHeight, UI_NUMERIC, heightString, labelWidth, gNewHeightLabel, "0");

    if (atoi(widthString.c_str()) > gMaximumLevelWidth) widthString = std::to_string(gMaximumLevelWidth);
    if (atoi(heightString.c_str()) > gMaximumLevelHeight) heightString = std::to_string(gMaximumLevelHeight);

    int oldNewWidth = gCurrentNewWidth;
    int newNewWidth = atoi(widthString.c_str());
    if (newNewWidth != oldNewWidth)
    {
        gCurrentNewWidth = newNewWidth;
    }
    int oldNewHeight = gCurrentNewHeight;
    int newNewHeight = atoi(heightString.c_str());
    if (newNewHeight != oldNewHeight)
    {
        gCurrentNewHeight = newNewHeight;
    }

    EndPanel();

    EndPanel();
}

TEINAPI void CancelNew ()
{
    HideWindow("WINNEW");
}

TEINAPI void HandleNewEvents ()
{
    if (IsWindowFocused("WINNEW"))
    {
        if (!TextBoxIsActive())
        {
            if (gMainEvent.type == SDL_KEYDOWN)
            {
                switch (gMainEvent.key.keysym.sym)
                {
                    case (SDLK_RETURN): Internal::OkayNew(); break;
                    case (SDLK_ESCAPE): CancelNew(); break;
                }
            }
        }
    }
}

TEINAPI int GetNewWidth ()
{
    return gCurrentNewWidth;
}
TEINAPI int GetNewHeight ()
{
    return gCurrentNewHeight;
}
