static constexpr const char* gTileLayerInfo  = "Toggle this tile layer's visibility (invisible layers can't be interacted with).";

static const Vec4 gLayerColors[LEVEL_LAYER_TOTAL]
{
{ .75f, .38f, .38f, 1 }, // Tag
{ .44f, .51f, .72f, 1 }, // Overlay
{ .79f, .69f, .20f, 1 }, // Active
{ .45f, .59f, .41f, 1 }, // Back 1
{ .57f, .48f, .71f, 1 }  // Back 2
};

static constexpr float gLayerPanelInnerPad = 3;
static constexpr float gLayerPanelButtonHeight = 24;

static float gLayerPanelContentHeight;
static float gLayerPanelHeight;
static float gLayerPanelScrollOffset;

static Quad gLayerPanelBounds;

namespace Internal
{
    TEINAPI bool DoLayerButton (UiFlag flags, int layer, std::string name, std::string info)
    {
        const Quad& clip = ((flags & UI_INACTIVE) ? gClipCross : gClipEye);

        constexpr float Pad = 5;
        Vec2 cursor(Pad, 0);

        float bw = GetPanelWidth();
        float bh = gLayerPanelButtonHeight;

        // If not inactive then we need to determine if this is the active layer.
        if (!(flags & UI_INACTIVE))
        {
            // If the tool is the select tool then technically all of the layers
            // are active (except for disabled ones). So it makes sense to just
            // highlight ever single layer when using this particular tool.
            if (gLevelEditor.toolType == ToolType::SELECT)
            {
                flags |= UI_HIGHLIGHT;
            }
            else
            {
                TileCategory category = GetSelectedCategory();
                if (CategoryToLayer(category) == static_cast<LevelLayer>(layer))
                {
                    flags |= UI_HIGHLIGHT;
                }
            }
        }

        bool result = BeginClickPanel(NULL, bw,bh, flags, info);

        SetPanelCursor(&cursor);
        SetPanelCursorDir(UI_DIR_RIGHT);

        float w = 10;
        float h = (gLayerPanelButtonHeight-4)-1; // -1 due to separator!

        cursor.y = (bh-h)/2;
        DoQuad(w, h, gLayerColors[layer]);
        cursor.y = 0;
        AdvancePanelCursor(Pad);
        DoIcon(24, GetPanelHeight(), gResourceIcons, &clip);
        AdvancePanelCursor(Pad);
        DoLabel(UI_ALIGN_LEFT, UI_ALIGN_CENTER, GetPanelHeight(), name);

        EndPanel();
        return result;
    }

    TEINAPI void ToggleLayer (LevelLayer layer)
    {
        if (CurrentTabIsLevel())
        {
            Tab& tab = GetCurrentTab();
            tab.tileLayerActive[layer] = !tab.tileLayerActive[layer];
            SelectNextActiveGroup();
        }
    }

    TEINAPI void ToggleLayerAction (LevelLayer layer)
    {
        if (CurrentTabIsLevel())
        {
            if (IsWindowFocused("WINMAIN"))
            {
                bool allLayersWereInactive = AreAllLayersInactive();
                Internal::ToggleLayer(layer);
                // If we're coming from all layers being inactive we need to find an entity
                // we can select now that there are entities that can be selected again.
                if (allLayersWereInactive && !AreAllLayersInactive())
                {
                    ResetSelectedGroup();
                }
            }
        }
    }
}

TEINAPI void InitLayerPanel ()
{
    gLayerPanelScrollOffset = 0;
    gLayerPanelBounds = {};
    gLayerPanelContentHeight = (gLayerPanelButtonHeight * LEVEL_LAYER_TOTAL) - 1; // -1 because we don't care about the last separator!
    gLayerPanelHeight = 0;
}

TEINAPI void DoLayerPanel (bool scrollbar)
{
    if (!IsLayerPanelPresent()) return;

    gLayerPanelBounds.x = 0;
    gLayerPanelBounds.y = GetTilePanelHeight();
    gLayerPanelBounds.w = GetPanelWidth();
    gLayerPanelBounds.h = GetPanelHeight() - GetTilePanelHeight();

    Vec2 cursor(0,0);

    SetUiTexture(&gResourceIcons);
    SetUiFont(&GetEditorRegularFont());

    constexpr float Pad = gLayerPanelInnerPad;

    BeginPanel(gLayerPanelBounds, UI_NONE, gUiColorMedium);
    BeginPanel(Pad, Pad, gControlPanelWidth-(Pad*2), gLayerPanelBounds.h-(Pad*2), UI_NONE, gUiColorMedDark);

    // NOTE: We do this to add a 1px border around the actual layer buttons in
    // the case that the panel is too small and needs a scrollbar it looks
    // nicer. Its a bit hacky and at some point we should probs have a feature
    // to add a padding border around a panel but for now we just do this...
    BeginPanel(1, 1, GetPanelWidth()-2, GetPanelHeight()-2, UI_NONE, gUiColorMedDark);
    gLayerPanelHeight = GetPanelHeight();

    SetPanelCursor(&cursor);
    SetPanelCursorDir(UI_DIR_DOWN);

    if (scrollbar)
    {
        float sx =  1 + GetViewport().w + gControlPanelInnerPad;
        float sy = -1;
        float sw =  gControlPanelScrollbarWidth - gControlPanelInnerPad;
        float sh =  2 + GetViewport().h;

        DoScrollbar(sx,sy,sw,sh, gLayerPanelContentHeight, gLayerPanelScrollOffset);
    }

    bool allLayersWereInactive = AreAllLayersInactive();
    Tab& tab = GetCurrentTab();

    for (int i=LEVEL_LAYER_TAG; i<LEVEL_LAYER_TOTAL; ++i)
    {
        const char* layerName = NULL;
        switch (i)
        {
            case (LEVEL_LAYER_TAG    ): layerName = "Tag";     break;
            case (LEVEL_LAYER_OVERLAY): layerName = "Overlay"; break;
            case (LEVEL_LAYER_ACTIVE ): layerName = "Active";  break;
            case (LEVEL_LAYER_BACK1  ): layerName = "Back 1";  break;
            case (LEVEL_LAYER_BACK2  ): layerName = "Back 2";  break;
        }

        UiFlag tileFlag = ((tab.tileLayerActive[i]) ? UI_NONE : UI_INACTIVE);
        if (Internal::DoLayerButton(tileFlag, static_cast<LevelLayer>(i), layerName, gTileLayerInfo))
        {
            Internal::ToggleLayer(static_cast<LevelLayer>(i));
        }
    }

    // If we're coming from all layers being inactive we need to find an entity
    // we can select now that there are entities that can be selected again.
    if (allLayersWereInactive && !AreAllLayersInactive())
    {
        ResetSelectedGroup();
    }

    EndPanel();

    EndPanel();
    EndPanel();
}

TEINAPI float GetLayerPanelHeight ()
{
    return ceilf((gLayerPanelContentHeight + 1 + 2 + (gLayerPanelInnerPad * 2)));
}

TEINAPI bool LayerPanelNeedsScrollbar ()
{
    return ((CurrentTabIsLevel()) ? (gLayerPanelContentHeight > gLayerPanelHeight) : false);
}

TEINAPI bool IsLayerPanelPresent ()
{
    return CurrentTabIsLevel();
}

TEINAPI void ToggleLayerActive ()
{
    Internal::ToggleLayerAction(LEVEL_LAYER_ACTIVE);
}
TEINAPI void ToggleLayerTag ()
{
    Internal::ToggleLayerAction(LEVEL_LAYER_TAG);
}
TEINAPI void ToggleLayerOverlay ()
{
    Internal::ToggleLayerAction(LEVEL_LAYER_OVERLAY);
}
TEINAPI void ToggleLayerBack1 ()
{
    Internal::ToggleLayerAction(LEVEL_LAYER_BACK1);
}
TEINAPI void ToggleLayerBack2 ()
{
    Internal::ToggleLayerAction(LEVEL_LAYER_BACK2);
}
