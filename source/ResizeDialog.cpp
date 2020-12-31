static constexpr float gResizeBottomBorder = 26;

static constexpr float gResizeXPad = 8;
static constexpr float gResizeYPad = 8;

static constexpr float gResizeTextBoxHeight = 20;

static constexpr const char* gResizeWidthLabel = "Level Width:  ";
static constexpr const char* gResizeHeightLabel = "Level Height:  ";

static int gCurrentResizeWidth = static_cast<int>(gDefaultLevelWidth);
static int gCurrentResizeHeight = static_cast<int>(gDefaultLevelHeight);

static ResizeDir gResizeDialogDir = ResizeDir::Center;

static const Quad* gNWClip     = &gClipResizeNW;
static const Quad* gNClip      = &gClipResizeN;
static const Quad* gNEClip     = &gClipResizeNE;
static const Quad* gWClip      = &gClipResizeW;
static const Quad* gCenterClip = &gClipBullet;
static const Quad* gEClip      = &gClipResizeE;
static const Quad* gSWClip     = &gClipResizeSW;
static const Quad* gSClip      = &gClipResizeS;
static const Quad* gSEClip     = &gClipResizeSE;

namespace Internal
{
    TEINAPI void CalculateDirClips ()
    {
        switch (gResizeDialogDir)
        {
            case (ResizeDir::NW):
            {
                gNWClip     = &gClipBullet;
                gNClip      = &gClipResizeE;
                gNEClip     = &gClipNone;
                gWClip      = &gClipResizeS;
                gCenterClip = &gClipResizeSE;
                gEClip      = &gClipNone;
                gSWClip     = &gClipNone;
                gSClip      = &gClipNone;
                gSEClip     = &gClipNone;
            } break;
            case (ResizeDir::N):
            {
                gNWClip     = &gClipResizeW;
                gNClip      = &gClipBullet;
                gNEClip     = &gClipResizeE;
                gWClip      = &gClipResizeSW;
                gCenterClip = &gClipResizeS;
                gEClip      = &gClipResizeSE;
                gSWClip     = &gClipNone;
                gSClip      = &gClipNone;
                gSEClip     = &gClipNone;
            } break;
            case (ResizeDir::NE):
            {
                gNWClip     = &gClipNone;
                gNClip      = &gClipResizeW;
                gNEClip     = &gClipBullet;
                gWClip      = &gClipNone;
                gCenterClip = &gClipResizeSW;
                gEClip      = &gClipResizeS;
                gSWClip     = &gClipNone;
                gSClip      = &gClipNone;
                gSEClip     = &gClipNone;
            } break;
            case (ResizeDir::W):
            {
                gNWClip     = &gClipResizeN;
                gNClip      = &gClipResizeNE;
                gNEClip     = &gClipNone;
                gWClip      = &gClipBullet;
                gCenterClip = &gClipResizeE;
                gEClip      = &gClipNone;
                gSWClip     = &gClipResizeS;
                gSClip      = &gClipResizeSE;
                gSEClip     = &gClipNone;
            } break;
            case (ResizeDir::Center):
            {
                gNWClip     = &gClipResizeNW;
                gNClip      = &gClipResizeN;
                gNEClip     = &gClipResizeNE;
                gWClip      = &gClipResizeW;
                gCenterClip = &gClipBullet;
                gEClip      = &gClipResizeE;
                gSWClip     = &gClipResizeSW;
                gSClip      = &gClipResizeS;
                gSEClip     = &gClipResizeSE;
            } break;
            case (ResizeDir::E):
            {
                gNWClip     = &gClipNone;
                gNClip      = &gClipResizeNW;
                gNEClip     = &gClipResizeN;
                gWClip      = &gClipNone;
                gCenterClip = &gClipResizeW;
                gEClip      = &gClipBullet;
                gSWClip     = &gClipNone;
                gSClip      = &gClipResizeSW;
                gSEClip     = &gClipResizeS;
            } break;
            case (ResizeDir::SW):
            {
                gNWClip     = &gClipNone;
                gNClip      = &gClipNone;
                gNEClip     = &gClipNone;
                gWClip      = &gClipResizeN;
                gCenterClip = &gClipResizeNE;
                gEClip      = &gClipNone;
                gSWClip     = &gClipBullet;
                gSClip      = &gClipResizeE;
                gSEClip     = &gClipNone;
            } break;
            case (ResizeDir::S):
            {
                gNWClip     = &gClipNone;
                gNClip      = &gClipNone;
                gNEClip     = &gClipNone;
                gWClip      = &gClipResizeNW;
                gCenterClip = &gClipResizeN;
                gEClip      = &gClipResizeNE;
                gSWClip     = &gClipResizeW;
                gSClip      = &gClipBullet;
                gSEClip     = &gClipResizeE;
            } break;
            case (ResizeDir::SE):
            {
                gNWClip     = &gClipNone;
                gNClip      = &gClipNone;
                gNEClip     = &gClipNone;
                gWClip      = &gClipNone;
                gCenterClip = &gClipResizeNW;
                gEClip      = &gClipResizeN;
                gSWClip     = &gClipNone;
                gSClip      = &gClipResizeW;
                gSEClip     = &gClipBullet;
            } break;
        }
    }

    TEINAPI void DoResizeAlignment (Vec2& cursor)
    {
        // Do the long horizontal separator first.
        float w = GetViewport().w - (gResizeXPad*2);

        AdvancePanelCursor(gResizeYPad*1.5f);
        DoSeparator(w);
        AdvancePanelCursor(gResizeYPad*1.5f);

        SetPanelCursorDir(UiDir::Right);

        float bw = 25;
        float bh = 25;

        float x = (GetViewport().w/2) - ((bw*3)/2);

        cursor.x = x;
        cursor.y -= 2; // Just to get the spacing above and below even.

        float qx1 = cursor.x - 1;
        float qy1 = cursor.y - 1;
        float qx2 = cursor.x + (bw * 3) + 1;
        float qy2 = cursor.y + (bh * 3) + 1;

        SetDrawColor(gUiColorExDark);
        FillQuad(qx1,qy1,qx2,qy2);

        ResizeDir oldDir = gResizeDialogDir;

        if (DoImageButton(NULL, bw,bh, UiFlag::None, gNWClip)) gResizeDialogDir = ResizeDir::NW;
        if (DoImageButton(NULL, bw,bh, UiFlag::None, gNClip)) gResizeDialogDir = ResizeDir::N;
        if (DoImageButton(NULL, bw,bh, UiFlag::None, gNEClip)) gResizeDialogDir = ResizeDir::NE;

        cursor.x = x;
        cursor.y += bh;

        if (DoImageButton(NULL, bw,bh, UiFlag::None, gWClip)) gResizeDialogDir = ResizeDir::W;
        if (DoImageButton(NULL, bw,bh, UiFlag::None, gCenterClip)) gResizeDialogDir = ResizeDir::Center;
        if (DoImageButton(NULL, bw,bh, UiFlag::None, gEClip)) gResizeDialogDir = ResizeDir::E;

        cursor.x = x;
        cursor.y += bh;

        if (DoImageButton(NULL, bw,bh, UiFlag::None, gSWClip)) gResizeDialogDir = ResizeDir::SW;
        if (DoImageButton(NULL, bw,bh, UiFlag::None, gSClip)) gResizeDialogDir = ResizeDir::S;
        if (DoImageButton(NULL, bw,bh, UiFlag::None, gSEClip)) gResizeDialogDir = ResizeDir::SE;

        if (oldDir != gResizeDialogDir)
        {
            CalculateDirClips();
        }
    }

    TEINAPI void OkayResize ()
    {
        if (gCurrentResizeWidth < gMinimumLevelWidth || gCurrentResizeHeight < gMinimumLevelHeight)
        {
            ShowAlert("Warning", FormatString("Minimum level size is %dx%d!", gMinimumLevelWidth, gMinimumLevelHeight), AlertType::Warning, AlertButton::Ok, "WINRESIZE");
            return;
        }

        LevelEditorResizeOkay();
        HideWindow("WINRESIZE");
    }
}

TEINAPI void OpenResize (int levelWidth, int levelHeight)
{
    SetWindowPos("WINRESIZE", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED);

    if (levelWidth > 0) gCurrentResizeWidth = levelWidth;
    if (levelHeight > 0) gCurrentResizeHeight = levelHeight;

    gResizeDialogDir = ResizeDir::Center;
    Internal::CalculateDirClips();

    ShowWindow("WINRESIZE");
}

TEINAPI void DoResize ()
{
    Quad p1, p2;

    p1.x = gWindowBorder;
    p1.y = gWindowBorder;
    p1.w = GetViewport().w - (gWindowBorder * 2);
    p1.h = GetViewport().h - (gWindowBorder * 2);

    SetUiFont(&GetEditorRegularFont());

    BeginPanel(p1, UiFlag::None, gUiColorExDark);

    float bb = gResizeBottomBorder;

    float vw = GetViewport().w;
    float vh = GetViewport().h;

    float bw = roundf(vw / 2);
    float bh = bb - gWindowBorder;

    // Bottom buttons for okaying or cancelling the resize.
    Vec2 buttonCursor(0, gWindowBorder);
    BeginPanel(0, vh-bb, vw, bb, UiFlag::None, gUiColorMedium);

    SetPanelCursorDir(UiDir::Right);
    SetPanelCursor(&buttonCursor);

    // Just to make sure that we always reach the end of the panel space.
    float bw2 = vw - bw;

    if (DoTextButton(NULL, bw,bh, UiFlag::None, "Resize")) Internal::OkayResize();
    if (DoTextButton(NULL, bw2,bh, UiFlag::None, "Cancel")) CancelResize();

    // Add a separator to the left for symmetry.
    buttonCursor.x = 1;
    DoSeparator(bh);

    EndPanel();

    p2.x = 1;
    p2.y = 1;
    p2.w = vw - 2;
    p2.h = vh - p2.y - bb - 1;

    BeginPanel(p2, UiFlag::None, gUiColorMedium);

    Vec2 cursor(gResizeXPad, gResizeYPad);

    SetPanelCursorDir(UiDir::Down);
    SetPanelCursor(&cursor);

    float labelWidthWidth = GetTextWidthScaled(GetEditorRegularFont(), gResizeWidthLabel);
    float labelHeightWidth = GetTextWidthScaled(GetEditorRegularFont(), gResizeHeightLabel);

    float textBoxWidth = (vw-(gResizeXPad*2));
    float labelWidth = std::max(labelWidthWidth, labelHeightWidth);

    std::string widthString(std::to_string(gCurrentResizeWidth));
    std::string heightString(std::to_string(gCurrentResizeHeight));

    DoTextBoxLabeled(textBoxWidth, gResizeTextBoxHeight, UiFlag::Numeric, widthString, labelWidth, gResizeWidthLabel, "0");
    AdvancePanelCursor(gResizeYPad);
    DoTextBoxLabeled(textBoxWidth, gResizeTextBoxHeight, UiFlag::Numeric, heightString, labelWidth, gResizeHeightLabel, "0");

    if (atoi(widthString.c_str()) > gMaximumLevelWidth) widthString = std::to_string(gMaximumLevelWidth);
    if (atoi(heightString.c_str()) > gMaximumLevelHeight) heightString = std::to_string(gMaximumLevelHeight);

    int oldResizeWidth = gCurrentResizeWidth;
    int newResizeWidth = atoi(widthString.c_str());
    if (newResizeWidth != oldResizeWidth)
    {
        gCurrentResizeWidth = newResizeWidth;
    }
    int oldResizeHeight = gCurrentResizeHeight;
    int newResizeHeight = atoi(heightString.c_str());
    if (newResizeHeight != oldResizeHeight)
    {
        gCurrentResizeHeight = newResizeHeight;
    }

    Internal::DoResizeAlignment(cursor);

    EndPanel();
    EndPanel();
}

TEINAPI void CancelResize ()
{
    HideWindow("WINRESIZE");
}

TEINAPI void HandleResizeEvents ()
{
    if (!IsWindowFocused("WINRESIZE")) return;

    if (!TextBoxIsActive())
    {
        if (gMainEvent.type == SDL_KEYDOWN)
        {
            switch (gMainEvent.key.keysym.sym)
            {
                case (SDLK_RETURN): Internal::OkayResize(); break;
                case (SDLK_ESCAPE): CancelResize(); break;
            }
        }
    }
}

TEINAPI int GetResizeWidth ()
{
    return gCurrentResizeWidth;
}
TEINAPI int GetResizeHeight ()
{
    return gCurrentResizeHeight;
}

TEINAPI ResizeDir GetResizeDir ()
{
    return gResizeDialogDir;
}

TEINAPI bool ResizeDirIsNorth (ResizeDir dir)
{
    return (dir == ResizeDir::NW || dir == ResizeDir::N || dir == ResizeDir::NE);
}
TEINAPI bool ResizeDirIsEast (ResizeDir dir)
{
    return (dir == ResizeDir::NE || dir == ResizeDir::E || dir == ResizeDir::SE);
}
TEINAPI bool ResizeDirIsSouth (ResizeDir dir)
{
    return (dir == ResizeDir::SW || dir == ResizeDir::S || dir == ResizeDir::SE);
}
TEINAPI bool ResizeDirIsWest (ResizeDir dir)
{
    return (dir == ResizeDir::NW || dir == ResizeDir::W || dir == ResizeDir::SW);
}
