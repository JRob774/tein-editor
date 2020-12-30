static constexpr float gPathDialogBottomBorder = 26;
static std::string gTempGamePath;

TEINAPI void OpenPath ()
{
    gTempGamePath = gEditorSettings.gamePath;
    ShowWindow("WINPATH");
}

TEINAPI void DoPath ()
{
    Quad p1, p2;

    p1.x = gWindowBorder;
    p1.y = gWindowBorder;
    p1.w = GetViewport().w - (gWindowBorder * 2);
    p1.h = GetViewport().h - (gWindowBorder * 2);

    SetUiFont(&GetEditorRegularFont());

    BeginPanel(p1, UI_NONE, gUiColorExDark);

    float bb = gPathDialogBottomBorder;

    float vw = GetViewport().w;
    float vh = GetViewport().h;

    float bw = roundf(vw / 2);
    float bh = bb - gWindowBorder;

    // Bottom buttons for okaying or cancelling the path dialog.
    Vec2 buttonCursor(0, gWindowBorder);
    BeginPanel(0, vh-bb, vw, bb, UI_NONE, gUiColorMedium);

    SetPanelCursorDir(UI_DIR_RIGHT);
    SetPanelCursor(&buttonCursor);

    // Just to make sure that we always reach the end of the panel space.
    float bw2 = vw - bw;

    if (DoTextButton(NULL, bw,bh, UI_NONE, "Okay")) OkayPath();
    if (DoTextButton(NULL, bw2,bh, UI_NONE, "Cancel")) CancelPath();

    // Add a separator to the left for symmetry.
    buttonCursor.x = 1;
    DoSeparator(bh);

    EndPanel();

    p2.x = 1;
    p2.y = 1;
    p2.w = vw - 2;
    p2.h = vh - p2.y - bb - 1;

    BeginPanel(p2, UI_NONE, gUiColorMedium);

    constexpr float XPad = 8;
    constexpr float YPad = 4;

    Vec2 cursor(XPad, YPad);

    SetPanelCursorDir(UI_DIR_DOWN);
    SetPanelCursor(&cursor);

    constexpr float ButtonWidth = 80;
    constexpr float LabelHeight = 24;

    cursor.x += 2;
    DoLabel(UI_ALIGN_LEFT,UI_ALIGN_CENTER, LabelHeight, "Please locate 'The End is Nigh' executable:");
    cursor.x -= 2;

    SetPanelCursorDir(UI_DIR_RIGHT);

    float tw = GetViewport().w - ButtonWidth - (XPad*2);
    float th = 24;

    cursor.y += 2;
    DoTextBox(tw,th, UI_FILEPATH, gTempGamePath, "", UI_ALIGN_LEFT);
    cursor.y += 1;

    float buttonHeight = th-2;

    float x1 = cursor.x - 1;
    float y1 = cursor.y - 1;
    float x2 = cursor.x + ButtonWidth + 1;
    float y2 = cursor.y + buttonHeight + 1;

    // Create a nice border so the button's bounds are actually visible!
    SetDrawColor(gUiColorExDark);
    FillQuad(x1,y1,x2,y2);

    if (DoTextButton(NULL, ButtonWidth,buttonHeight, UI_SINGLE, "Search"))
    {
        auto result = OpenDialog(DialogType::EXE, false);
        if (!result.empty()) gTempGamePath = result.at(0);
    }

    EndPanel();
    EndPanel();
}

TEINAPI void OkayPath ()
{
    gTempGamePath = FixPathSlashes(gTempGamePath);
    gEditorSettings.gamePath = gTempGamePath;

    save_preferences();
    HideWindow("WINPATH");

    if (!RunExecutable(gEditorSettings.gamePath))
    {
        LogError(ERR_MED, "Failed to launch The End is Nigh executable!");
    }
}

TEINAPI void CancelPath ()
{
    gTempGamePath.clear();
    HideWindow("WINPATH");
}

TEINAPI void HandlePathEvents ()
{
    if (!IsWindowFocused("WINPATH")) return;

    if (!TextBoxIsActive())
    {
        if (main_event.type == SDL_KEYDOWN)
        {
            switch (main_event.key.keysym.sym)
            {
                case (SDLK_RETURN): OkayPath(); break;
                case (SDLK_ESCAPE): CancelPath(); break;
            }
        }
    }
}
