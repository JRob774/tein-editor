enum class ChannelType { INVALID, R, G, B, A };

static constexpr float gColorPickerChannelWidth = 40;
static constexpr float gColorPickerChannelHeight = 180;

static constexpr float gColorPickerXPad = 8;
static constexpr float gColorPickerYPad = 8;

static constexpr float gColorPickerTextBoxHeight = 20;
static constexpr float gColorPickerAlphaHeight = 20;

static constexpr float gColorPickerBottomBorder = 26;

static constexpr float gColorPickerSwatchLarge = 32;
static constexpr float gColorPickerSwatchSmall = 16;

static constexpr float gColorPickerSwatchXPad = 7;
static constexpr float gColorPickerSwatchYPad = 8;

static constexpr float gColorPickerSwatchGap = 6;

static constexpr int gColorPickerSwatchCount = 9;

static ChannelType gColorPickerActiveChannel;
static bool gColorPickerMousePressed;

static std::deque<Vec4> gColorPickerSwatches;

// When the color picker menu is opened we cache the current color value.
// This allows the color picker menu to modify the current color immediately
// for instant feedback. If the user then wants to cancel the changes made,
// we can just restore the cached version of the color to turn it back.

static Vec4* gCurrentColorPickerColor;
static Vec4 gCachedColorPickerColor;

namespace Internal
{
    TEINAPI void DoColorChannel (Vec2& cursor, Vec4 min, Vec4 max, float& channel, ChannelType type)
    {
        float xPad = gColorPickerXPad;
        float yPad = gColorPickerYPad;

        float cw = gColorPickerChannelWidth;
        float ch = gColorPickerChannelHeight;

        float x1 = cursor.x;
        float y1 = cursor.y;
        float x2 = cursor.x + cw;
        float y2 = cursor.y + ch;

        // The dark and light border surrounding the channel quad.
        SetDrawColor(gUiColorLight);
        FillQuad(x1-2,y1-2,x2+2,y2+2);
        SetDrawColor(gUiColorExDark);
        FillQuad(x1-1,y1-1,x2+1,y2+1);

        BeginDraw(BufferMode::TRIANGLE_STRIP);
        PutVertex(cursor.x   , cursor.y+ch, min); // BL
        PutVertex(cursor.x   , cursor.y   , max); // TL
        PutVertex(cursor.x+cw, cursor.y+ch, min); // BR
        PutVertex(cursor.x+cw, cursor.y   , max); // TR
        EndDraw();

        float percent = std::clamp(roundf(channel*100), 0.0f, 100.0f);
        std::string channelString(std::to_string(static_cast<int>(percent)));
        cursor.y += (ch + yPad);
        DoTextBox(cw, gColorPickerTextBoxHeight, UI_NUMERIC, channelString, "0");
        if (atoi(channelString.c_str()) > 100) channelString = "100";
        float newChannel = static_cast<float>(atoi(channelString.c_str())) / 100;
        if (channel != newChannel) channel = newChannel;
        // Reset cursor after text box.
        cursor.x -= cw;
        cursor.y -= (ch + yPad);

        // Draw the indicator for what value is currently selected.
        float pos = (cursor.y + ch) - roundf(ch * channel);

        float ix1 = cursor.x-2;
        float iy1 = pos-2;
        float ix2 = cursor.x+cw+2;
        float iy2 = pos+2;

        Vec4 cur = min;
        switch (type)
        {
            case (ChannelType::R): cur.r = channel; break;
            case (ChannelType::G): cur.g = channel; break;
            case (ChannelType::B): cur.b = channel; break;
        }

        SetDrawColor(gUiColorLight);
        FillQuad(ix1-2,iy1-2,ix2+2,iy2+2);
        SetDrawColor(gUiColorExDark);
        FillQuad(ix1-1,iy1-1,ix2+1,iy2+1);
        SetDrawColor(cur);
        FillQuad(ix1-0,iy1-0,ix2+0,iy2+0);

        // Handle the indicator both being selected, deselected, and moved.
        if (gColorPickerMousePressed)
        {
            if (MouseInUiBoundsXYWH(ix1-1, iy1-1, cw+2, 7))
            {
                gColorPickerActiveChannel = type;
            }
            else if (MouseInUiBoundsXYWH(cursor.x, cursor.y, cw, ch))
            {
                gColorPickerActiveChannel = type;
                // Determine where the click was and jump to that position.
                float my = ch - (GetMousePos().y - GetViewport().y - cursor.y);
                channel = my / ch;
            }
        }
        if (gColorPickerActiveChannel == type)
        {
            channel -= (UiGetRelativeMouse().y / ch);
            channel = std::clamp(channel, 0.0f, 1.0f);
        }

        // Move the cursor into position for next channel.
        cursor.x += cw + (xPad*3);
    }

    TEINAPI void DoColorPreview (Vec2& cursor, Vec4 color, float size)
    {
        const Texture& texture = (static_cast<int>(size) % 14 == 0) ? gResourceChecker14 : gResourceChecker16;

        float x = cursor.x;
        float y = cursor.y;

        // The dark and light border surrounding the color.
        SetDrawColor(gUiColorLight);
        FillQuad(x-2,y-2,x+size+2,y+size+2);
        SetDrawColor(gUiColorExDark);
        FillQuad(x-1,y-1,x+size+1,y+size+1);

        float tx = x + (size / 2);
        float ty = y + (size / 2);

        Quad clip = { 0,0,size,size };
        DrawTexture(texture, tx,ty, &clip);

        Vec4 max = color;
        Vec4 min = color;

        max.a = 1;

        BeginDraw(BufferMode::TRIANGLE_STRIP);
        PutVertex(x     , y+size, min); // BL
        PutVertex(x     , y     , max); // TL
        PutVertex(x+size, y+size, min); // BR
        PutVertex(x+size, y     , max); // TR
        EndDraw();
    }

    TEINAPI void DoSwatchPanel (Vec2& cursor)
    {
        assert(gCurrentColorPickerColor);

        cursor.y += gColorPickerSwatchLarge + gColorPickerYPad;

        SetDrawColor(gUiColorMedDark);
        DrawQuad(cursor.x, cursor.y, cursor.x+gColorPickerSwatchLarge, GetPanelHeight()-gColorPickerYPad);

        float x = cursor.x+1;
        float y = cursor.y+1;
        float w = gColorPickerSwatchLarge-2;
        float h = (GetPanelHeight()-gColorPickerYPad)-cursor.y-2;

        Vec2 cursor2(gColorPickerSwatchXPad, gColorPickerSwatchYPad);
        BeginPanel(x,y,w,h, UI_NONE);

        for (auto& swatch: gColorPickerSwatches)
        {
            DoColorPreview(cursor2, swatch, gColorPickerSwatchSmall);
            if (gColorPickerMousePressed)
            {
                if (MouseInUiBoundsXYWH(cursor2.x, cursor2.y, gColorPickerSwatchSmall, gColorPickerSwatchSmall))
                {
                    *gCurrentColorPickerColor = swatch;
                }
            }

            cursor2.y += gColorPickerSwatchSmall;
            cursor2.y += gColorPickerSwatchGap;
        }

        EndPanel();
    }

    TEINAPI void DoAlphaChannel (Vec2& cursor, Vec4& color)
    {
        float xPad = gColorPickerXPad;
        float yPad = gColorPickerYPad;

        float cw = gColorPickerChannelWidth;
        float ch = gColorPickerChannelHeight;

        cursor.x = xPad;
        cursor.y = (yPad*3.25f) + ch + gColorPickerTextBoxHeight;

        SetPanelCursorDir(UI_DIR_DOWN);
        DoSeparator((cw*3)+(xPad*6));
        cursor.y -= 1;
        SetPanelCursorDir(UI_DIR_RIGHT);

        cursor.y += (yPad*1.25f);

        float percent = std::clamp(roundf(color.a*100), 0.0f, 100.0f);
        std::string alphaString(std::to_string(static_cast<int>(percent)));
        DoTextBox(cw, gColorPickerTextBoxHeight, UI_NUMERIC, alphaString, "0");
        if (atoi(alphaString.c_str()) > 100) alphaString = "100";
        float newAlpha = static_cast<float>(atoi(alphaString.c_str())) / 100;
        if (color.a != newAlpha) color.a = newAlpha;

        cursor.x += xPad;

        float x1 = cursor.x;
        float y1 = cursor.y;
        float x2 = cursor.x + ((cw*2)+(xPad*5));
        float y2 = cursor.y + gColorPickerAlphaHeight;

        SetDrawColor(gUiColorLight);
        FillQuad(x1-2,y1-2,x2+2,y2+2);
        SetDrawColor(gUiColorExDark);
        FillQuad(x1-1,y1-1,x2+1,y2+1);

        float tw = x2-x1;
        float th = gColorPickerAlphaHeight;
        float tx = cursor.x + (tw / 2);
        float ty = cursor.y + (th / 2);

        Quad clip1 = { 0,0,tw,th };
        DrawTexture(gResourceChecker20, tx,ty, &clip1);

        Vec4 min = color;
        Vec4 max = color;

        min.a = 0;
        max.a = 1;

        BeginDraw(BufferMode::TRIANGLE_STRIP);
        PutVertex(x1, y2, min); // BL
        PutVertex(x1, y1, min); // TL
        PutVertex(x2, y2, max); // BR
        PutVertex(x2, y1, max); // TR
        EndDraw();

        // Draw the indicator for what value is currently selected.
        float pos = (cursor.x + tw) - roundf(tw * (1-color.a));

        float ix1 = pos-2;
        float iy1 = cursor.y-2;
        float ix2 = pos+2;
        float iy2 = cursor.y+th+2;

        SetDrawColor(gUiColorLight);
        FillQuad(ix1-2,iy1-2,ix2+2,iy2+2);
        SetDrawColor(gUiColorExDark);
        FillQuad(ix1-1,iy1-1,ix2+1,iy2+1);

        float itw = ix2-ix1;
        float ith = iy2-iy1;
        float itx = ix1 + (itw / 2);
        float ity = iy1 + (ith / 2);

        Quad clip2 = { ix1-cursor.x, -2, itw, ith };
        DrawTexture(gResourceChecker20, itx, ity, &clip2);

        SetDrawColor(color);
        FillQuad(ix1,iy1,ix2,iy2);

        // Handle the indicator both being selected, deselected, and moved.
        if (gColorPickerMousePressed)
        {
            if (MouseInUiBoundsXYWH(ix1-1,iy1-1,7,th+2))
            {
                gColorPickerActiveChannel = ChannelType::A;
            }
            else if (MouseInUiBoundsXYWH(cursor.x, cursor.y, tw, th))
            {
                gColorPickerActiveChannel = ChannelType::A;
                // Determine where the click was and jump to that position.
                float mx = tw - (GetMousePos().x - GetViewport().x - cursor.x);
                color.a = 1 - (mx / tw);
            }
        }
        if (gColorPickerActiveChannel == ChannelType::A)
        {
            color.a += (UiGetRelativeMouse().x / tw);
            color.a = std::clamp(color.a, 0.0f, 1.0f);
        }
    }

    TEINAPI void SaveColorSwatch ()
    {
        assert(gCurrentColorPickerColor);
        gColorPickerSwatches.pop_front();
        gColorPickerSwatches.push_back(*gCurrentColorPickerColor);
    }
}

TEINAPI void InitColorPicker ()
{
    gCurrentColorPickerColor = NULL;
    // Add some empty swatches to the color picker on start-up.
    for (int i=0; i<gColorPickerSwatchCount; ++i)
    {
        gColorPickerSwatches.push_back(Vec4(1,1,1,0));
    }
}

TEINAPI void OpenColorPicker (Vec4* color)
{
    RaiseWindow("WINCOLOR");

    gColorPickerActiveChannel = ChannelType::INVALID;
    gColorPickerMousePressed = false;

    assert(color);

    gCurrentColorPickerColor = color;
    gCachedColorPickerColor = *color;

    if (IsWindowHidden("WINCOLOR")) ShowWindow("WINCOLOR");
}

TEINAPI void DoColorPicker ()
{
    if (IsWindowHidden("WINCOLOR")) return;

    Quad p1, p2;

    p1.x = gWindowBorder;
    p1.y = gWindowBorder;
    p1.w = GetViewport().w - (gWindowBorder * 2);
    p1.h = GetViewport().h - (gWindowBorder * 2);

    SetUiFont(&GetEditorRegularFont());

    BeginPanel(p1, UI_NONE, gUiColorExDark);

    float bb = gColorPickerBottomBorder;

    float vw = GetViewport().w;
    float vh = GetViewport().h;

    float bw = roundf(vw / 3);
    float bh = bb - gWindowBorder;

    // Bottom buttons for okaying or cancelling the color picker.
    Vec2 buttonCursor(0, gWindowBorder);
    BeginPanel(0,vh-bb,vw,bb, UI_NONE, gUiColorMedium);

    SetPanelCursorDir(UI_DIR_RIGHT);
    SetPanelCursor(&buttonCursor);

    // Just to make sure that we always reach the end of the panel space.
    float bw2 = vw - (bw*2);

    if (DoTextButton(NULL, bw ,bh, UI_NONE, "Okay"  )) OkayColorPicker();
    if (DoTextButton(NULL, bw ,bh, UI_NONE, "Save"  )) Internal::SaveColorSwatch();
    if (DoTextButton(NULL, bw2,bh, UI_NONE, "Cancel")) CancelColorPicker();

    // Add a separator to the left for symmetry.
    buttonCursor.x = 1;
    DoSeparator(bh);

    EndPanel();

    p2.x = 1;
    p2.y = 1;
    p2.w = vw - 2;
    p2.h = vh - p2.y - bb - 1;

    BeginPanel(p2, UI_NONE, gUiColorMedium);

    assert(gCurrentColorPickerColor);

    Vec4& color = *gCurrentColorPickerColor; // So much shorter to type out...
    Vec2 cursor(gColorPickerXPad, gColorPickerYPad);

    SetPanelCursorDir(UI_DIR_RIGHT);
    SetPanelCursor(&cursor);

    // Draw the three R G B channel selectors/sliders.
    Vec4 rMin(      0, color.g, color.b, 1);
    Vec4 rMax(      1, color.g, color.b, 1);
    Vec4 gMin(color.r,       0, color.b, 1);
    Vec4 gMax(color.r,       1, color.b, 1);
    Vec4 bMin(color.r, color.g,       0, 1);
    Vec4 bMax(color.r, color.g,       1, 1);

    Internal::DoColorChannel(cursor, rMin, rMax, color.r, ChannelType::R);
    Internal::DoColorChannel(cursor, gMin, gMax, color.g, ChannelType::G);
    Internal::DoColorChannel(cursor, bMin, bMax, color.b, ChannelType::B);

    // Draw the current color to showcase to the user.
    Internal::DoColorPreview(cursor, color, gColorPickerSwatchLarge);
    // Draw box surrounding the saved swatches.
    Internal::DoSwatchPanel(cursor);
    // Draw the alpha/opacity text box section.
    Internal::DoAlphaChannel(cursor, color);

    EndPanel();
    EndPanel();
}

TEINAPI void OkayColorPicker ()
{
    // Special checks for background and tile grid color because of the defaulting.
    if (gCurrentColorPickerColor == &gEditorSettings.backgroundColor) gEditorSettings.backgroundColorDefaulted = false;
    if (gCurrentColorPickerColor == &gEditorSettings.tileGridColor) gEditorSettings.tileGridColorDefaulted = false;
    HideWindow("WINCOLOR");
}

TEINAPI void CancelColorPicker ()
{
    assert(gCurrentColorPickerColor);
    *gCurrentColorPickerColor = gCachedColorPickerColor;
    HideWindow("WINCOLOR");
}

TEINAPI void HandleColorPickerEvents ()
{
    gColorPickerMousePressed = false;

    if (!IsWindowFocused("WINCOLOR")) return;

    // Determine if the mouse was pressed this update/cycle.
    switch (gMainEvent.type)
    {
        case (SDL_MOUSEBUTTONDOWN):
        {
            if (gMainEvent.button.button == SDL_BUTTON_LEFT)
            {
                gColorPickerMousePressed = true;
            }
        } break;
        case (SDL_MOUSEBUTTONUP):
        {
            if (gMainEvent.button.button == SDL_BUTTON_LEFT)
            {
                gColorPickerActiveChannel = ChannelType::INVALID;
            }
        } break;
        case (SDL_KEYDOWN):
        {
            if (!TextBoxIsActive())
            {
                switch (gMainEvent.key.keysym.sym)
                {
                    case (SDLK_RETURN): OkayColorPicker(); break;
                    case (SDLK_ESCAPE): CancelColorPicker(); break;
                }
            }
        } break;
    }
}
