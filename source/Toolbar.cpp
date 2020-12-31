static constexpr const char* gToolbarInfoBrush = "Brush Tool: Allows for placing and erasing of single tiles.";
static constexpr const char* gToolbarInfoFill = "Fill Tool: Fills or erases a given region with tiles.";
static constexpr const char* gToolbarInfoSelect = "Select Tool: Click and drag to select tiles for copy, cut, delete, grouping, etc.";
static constexpr const char* gToolbarInfoGrid = "Grid: Toggle the tile grid on and off.";
static constexpr const char* gToolbarInfoBounds = "Bounds: Toggle the shaded out-of-bounds region on and off.";
static constexpr const char* gToolbarInfoLayers = "Layer Transparency: Toggles the transparency applied to layers above the active layer.";
static constexpr const char* gToolbarInfoCamera = "Reset Camera: Return to the editor's default position and zoom.";
static constexpr const char* gToolbarInfoResize = "Resize: Resize the boundaries of the level.";
static constexpr const char* gToolbarInfoFlipH = "Flip Horizontal: Flip the level's content horizontally.";
static constexpr const char* gToolbarInfoFlipV = "Flip Vertical: Flip the level's content vertically.";
static constexpr const char* gToolbarInfoMirrorH = "Mirror Horizontal: Toggle horizontal symmetry on and off.";
static constexpr const char* gToolbarInfoMirrorV = "Mirror Vertical: Toggle vertical symmetry on and off.";
static constexpr const char* gToolbarInfoCut = "Cut: Copy and delete the currently selected region.";
static constexpr const char* gToolbarInfoCopy = "Copy: Copy the currently selected region.";
static constexpr const char* gToolbarInfoDeselect = "Deselect: Removes the current box selection.";
static constexpr const char* gToolbarInfoClear = "Clear Selection: Deletes everything inside the current selection.";
static constexpr const char* gToolbarInfoEntity = "Large Entities: Toggles entity graphics to represent their in-game size.";
static constexpr const char* gToolbarInfoGuides = "Entity Guides: Toggles guides to showcase entity placement and bounds.";

static constexpr float gTotalToolbarLevelButtons = 18;
static constexpr float gTotalToolbarMapButtons = 1;

static float gToolbarWidth = gToolbarDefaultWidth;

namespace Internal
{
    TEINAPI void DoLevelToolbar ()
    {
        Vec2 cursor(gToolbarInnerPad, gToolbarInnerPad);

        // Dimensions for the toolbar buttons.
        float bw = gToolbarDefaultWidth - (gToolbarInnerPad * 2);
        float bh = 25;

        SetUiTexture(&gResourceIcons);

        UiFlag brushFlags = UiFlag::None;
        UiFlag fillFlags = UiFlag::None;
        UiFlag selectFlags = UiFlag::None;
        UiFlag gridFlags = UiFlag::None;
        UiFlag boundsFlags = UiFlag::None;
        UiFlag layerFlags = UiFlag::None;
        UiFlag entityFlags = UiFlag::None;
        UiFlag guideFlags = UiFlag::None;
        UiFlag flipHFlags = UiFlag::None;
        UiFlag flipVFlags = UiFlag::None;
        UiFlag mirrorHFlags = UiFlag::None;
        UiFlag mirrorVFlags = UiFlag::None;
        UiFlag cutFlags = UiFlag::Locked;
        UiFlag copyFlags = UiFlag::Locked;
        UiFlag deselectFlags = UiFlag::Locked;
        UiFlag clearFlags = UiFlag::Locked;

        brushFlags = ((gLevelEditor.toolType == ToolType::Brush) ? UiFlag::None : UiFlag::Inactive);
        fillFlags = ((gLevelEditor.toolType == ToolType::Fill) ? UiFlag::None : UiFlag::Inactive);
        selectFlags = ((gLevelEditor.toolType == ToolType::Select) ? UiFlag::None : UiFlag::Inactive);
        gridFlags = ((gEditor.gridVisible) ? UiFlag::None : UiFlag::Inactive);
        boundsFlags = ((gLevelEditor.boundsVisible) ? UiFlag::None : UiFlag::Inactive);
        layerFlags = ((gLevelEditor.layerTransparency) ? UiFlag::None : UiFlag::Inactive);
        entityFlags = ((gLevelEditor.largeTiles) ? UiFlag::None : UiFlag::Inactive);
        guideFlags = ((gLevelEditor.entityGuides) ? UiFlag::None : UiFlag::Inactive);
        mirrorHFlags = ((gLevelEditor.mirrorH) ? UiFlag::None : UiFlag::Inactive);
        mirrorVFlags = ((gLevelEditor.mirrorV) ? UiFlag::None : UiFlag::Inactive);

        if (AreAnySelectBoxesVisible())
        {
            cutFlags = UiFlag::None;
            copyFlags = UiFlag::None;
            deselectFlags = UiFlag::None;
            clearFlags = UiFlag::None;
        }

        guideFlags |= (gLevelEditor.largeTiles) ? UiFlag::None : UiFlag::Locked;

        // If not all buttons will fit on the screen we will double the width of the
        // toolbar and then place the buttons side-to-side instead of straight down.
        float contentHeight = (bh*gTotalToolbarLevelButtons)+(gToolbarInnerPad*2);
        gToolbarWidth = gToolbarDefaultWidth;
        if (GetViewport().h < contentHeight)
        {
            gToolbarWidth *= 2;
            gToolbarWidth -= 1;
        }

        // The toolbar is a vertical list of available tools.
        BeginPanel(0, 0, gToolbarWidth, GetViewport().h, UiFlag::None, gUiColorMedium);

        SetPanelCursor(&cursor);
        SetPanelCursorDir(UiDir::Down);

        DoImageButton(ToolbarSetToolToBrush, bw,bh, brushFlags, &gClipBrush, gToolbarInfoBrush, gKbToolBrush, "Brush");
        DoImageButton(ToolbarSetToolToFill, bw,bh, fillFlags, &gClipFill, gToolbarInfoFill, gKbToolFill, "Fill");
        DoImageButton(ToolbarSetToolToSelect, bw,bh, selectFlags, &gClipSelect, gToolbarInfoSelect, gKbToolSelect, "Select");
        DoImageButton(ToolbarToggleGrid, bw,bh, gridFlags, &gClipGrid, gToolbarInfoGrid, gKbGridToggle, "Grid");
        DoImageButton(ToolbarToggleBounds, bw,bh, boundsFlags, &gClipBounds, gToolbarInfoBounds, gKbBoundsToggle, "Bounds");
        DoImageButton(ToolbarToggleLayerTransparency, bw,bh, layerFlags, &gClipLayers, gToolbarInfoLayers, gKbLayersToggle, "Layer Transparency");
        DoImageButton(ToolbarResetCamera, bw,bh, UiFlag::None, &gClipCamera, gToolbarInfoCamera, gKbCameraReset, "Reset Camera");
        DoImageButton(ToolbarToggleEntity, bw,bh, entityFlags, &gClipEntity, gToolbarInfoEntity, gKbToggleEntity, "Toggle Large Entities");
        DoImageButton(ToolbarToggleGuides, bw,bh, guideFlags, &gClipGuides, gToolbarInfoGuides, gKbToggleGuides, "Toggle Entity Guides");

        // If not all buttons will fit on the screen we will double the width of the
        // toolbar and then place the buttons side-to-side instead of straight down.
        if (GetViewport().h < contentHeight)
        {
            cursor.x += bw + 1;
            cursor.y = gToolbarInnerPad;

            SetPanelCursorDir(UiDir::Right);
            DoSeparator((bh*ceilf(gTotalToolbarLevelButtons/2)));
            SetPanelCursorDir(UiDir::Down);

            cursor.x -= 1;
            cursor.y = gToolbarInnerPad;
        }

        DoImageButton(ToolbarResize, bw,bh, UiFlag::None, &gClipResize, gToolbarInfoResize, gKbLevelResize, "Resize");
        DoImageButton(ToolbarFlipLevelH, bw,bh, flipHFlags, &gClipFlipH, gToolbarInfoFlipH, gKbFlipH, "Flip Horizontal");
        DoImageButton(ToolbarFlipLevelV, bw,bh, flipVFlags, &gClipFlipV, gToolbarInfoFlipV, gKbFlipV, "Flip Vertical");
        DoImageButton(ToolbarToggleMirrorH, bw,bh, mirrorHFlags, &gClipMirrorH, gToolbarInfoMirrorH, gKbMirrorHToggle, "Mirror Horizontal");
        DoImageButton(ToolbarToggleMirrorV, bw,bh, mirrorVFlags, &gClipMirrorV, gToolbarInfoMirrorV, gKbMirrorVToggle, "Mirror Vertical");
        DoImageButton(ToolbarCut, bw,bh, cutFlags, &gClipCut, gToolbarInfoCut, gKbCut, "Cut");
        DoImageButton(ToolbarCopy, bw,bh, copyFlags, &gClipCopy, gToolbarInfoCopy, gKbCopy, "Copy");
        DoImageButton(ToolbarDeselect, bw,bh, deselectFlags, &gClipDeselect, gToolbarInfoDeselect, gKbDeselect, "Deselect");
        DoImageButton(ToolbarClearSelect, bw,bh, clearFlags, &gClipClear, gToolbarInfoClear, gKbClearSelect, "Clear Selection");

        EndPanel();

    }
    TEINAPI void DoMapToolbar ()
    {
        Vec2 cursor(gToolbarInnerPad, gToolbarInnerPad);

        // Dimensions for the toolbar buttons.
        float bw = gToolbarDefaultWidth - (gToolbarInnerPad * 2);
        float bh = 25;

        SetUiTexture(&gResourceIcons);

        Tab& tab = GetCurrentTab();

        UiFlag cutFlags = UiFlag::Locked;
        UiFlag copyFlags = UiFlag::Locked;
        UiFlag deselectFlags = UiFlag::Locked;
        UiFlag clearFlags = UiFlag::Locked;

        if (MapSelectBoxPresent())
        {
            cutFlags = UiFlag::None;
            copyFlags = UiFlag::None;
            deselectFlags = UiFlag::None;
            clearFlags = UiFlag::None;
        }

        gToolbarWidth = gToolbarDefaultWidth;
        BeginPanel(0, 0, gToolbarWidth, GetViewport().h, UiFlag::None, gUiColorMedium);

        SetPanelCursor(&cursor);
        SetPanelCursorDir(UiDir::Down);

        DoImageButton(ToolbarResetCamera, bw,bh, UiFlag::None, &gClipCamera, gToolbarInfoCamera, gKbCameraReset, "Reset Camera");
        DoImageButton(ToolbarCut, bw,bh, cutFlags, &gClipCut, gToolbarInfoCut, gKbCut, "Cut");
        DoImageButton(ToolbarCopy, bw,bh, copyFlags, &gClipCopy, gToolbarInfoCopy, gKbCopy, "Copy");
        DoImageButton(ToolbarDeselect, bw,bh, deselectFlags, &gClipDeselect, gToolbarInfoDeselect, gKbDeselect, "Deselect");
        DoImageButton(ToolbarClearSelect, bw,bh, clearFlags, &gClipClear, gToolbarInfoClear, gKbClearSelect, "Clear Selection");

        EndPanel();
    }
}

TEINAPI void DoToolbar ()
{
    if (!AreThereAnyTabs()) return;
    if (CurrentTabIsLevel()) Internal::DoLevelToolbar(); else Internal::DoMapToolbar();
}

TEINAPI float GetToolbarWidth ()
{
    return ((AreThereAnyTabs()) ? gToolbarWidth : 0);
}

TEINAPI void ToolbarSetToolToBrush ()
{
    if (!CurrentTabIsLevel()) return;

    if (gLevelEditor.toolType == ToolType::Select && gLevelEditor.toolState == ToolState::Place)
    {
        Tab& tab = GetCurrentTab();
        if (!tab.toolInfo.select.bounds.empty())
        {
            NewLevelHistoryState(LevelHistoryAction::SelectState);
        }
    }

    if (gLevelEditor.toolType != ToolType::Brush && gLevelEditor.toolState != ToolState::Idle)
    {
        NewLevelHistoryState(LevelHistoryAction::Normal);
    }

    gLevelEditor.toolType = ToolType::Brush;
}

TEINAPI void ToolbarSetToolToFill ()
{
    if (!CurrentTabIsLevel()) return;

    if (gLevelEditor.toolType == ToolType::Select && gLevelEditor.toolState == ToolState::Place)
    {
        Tab& tab = GetCurrentTab();
        if (!tab.toolInfo.select.bounds.empty())
        {
            NewLevelHistoryState(LevelHistoryAction::SelectState);
        }
    }

    if (gLevelEditor.toolType != ToolType::Fill && gLevelEditor.toolState != ToolState::Idle)
    {
        NewLevelHistoryState(LevelHistoryAction::Normal);
    }

    gLevelEditor.toolType = ToolType::Fill;
}

TEINAPI void ToolbarSetToolToSelect ()
{
    if (!CurrentTabIsLevel()) return;

    if (gLevelEditor.toolType != ToolType::Select && gLevelEditor.toolState != ToolState::Idle)
    {
        NewLevelHistoryState(LevelHistoryAction::Normal);
    }

    if (gLevelEditor.toolState == ToolState::Place)
    {
        Tab& tab = GetCurrentTab();
        tab.toolInfo.select.start = true;
    }

    gLevelEditor.toolType = ToolType::Select;
}

TEINAPI void ToolbarToggleGrid ()
{
    if (CurrentTabIsLevel()) gEditor.gridVisible = !gEditor.gridVisible;
}

TEINAPI void ToolbarToggleBounds ()
{
    if (CurrentTabIsLevel()) gLevelEditor.boundsVisible = !gLevelEditor.boundsVisible;
}

TEINAPI void ToolbarToggleLayerTransparency ()
{
    if (CurrentTabIsLevel()) gLevelEditor.layerTransparency = !gLevelEditor.layerTransparency;
}

TEINAPI void ToolbarResetCamera ()
{
    if (!AreThereAnyTabs()) return;

    Tab& tab = GetCurrentTab();
    tab.camera.zoom = 1;

    if (tab.type == TabType::Map)
    {
        tab.camera.x = -GetMinMapBoundsX();
        tab.camera.y = -GetMinMapBoundsY();
    }
    else
    {
        tab.camera.x = 0;
        tab.camera.y = 0;
    }
}

TEINAPI void ToolbarFlipLevelH ()
{
    if (CurrentTabIsLevel()) FlipLevelH();
}
TEINAPI void ToolbarFlipLevelV ()
{
    if (CurrentTabIsLevel()) FlipLevelV();
}

TEINAPI void ToolbarToggleMirrorH ()
{
    if (CurrentTabIsLevel()) gLevelEditor.mirrorH = !gLevelEditor.mirrorH;
}
TEINAPI void ToolbarToggleMirrorV ()
{
    if (CurrentTabIsLevel()) gLevelEditor.mirrorV = !gLevelEditor.mirrorV;
}

TEINAPI void ToolbarCut ()
{
    if (!AreThereAnyTabs()) return;
    Tab& tab = GetCurrentTab();
    switch (tab.type)
    {
        case (TabType::Level): LevelEditorCut(); break;
        case (TabType::Map): MapEditorCut(); break;
    }
}

TEINAPI void ToolbarCopy ()
{
    if (!AreThereAnyTabs()) return;
    Tab& tab = GetCurrentTab();
    switch (tab.type)
    {
        case (TabType::Level): LevelEditorCopy(); break;
        case (TabType::Map): MapEditorCopy(); break;
    }
}

TEINAPI void ToolbarDeselect ()
{
    if (!AreThereAnyTabs()) return;
    Tab& tab = GetCurrentTab();
    switch (tab.type)
    {
        case (TabType::Level): LevelEditorDeselect(); break;
        case (TabType::Map): MapEditorDeselect(); break;
    }
}

TEINAPI void ToolbarClearSelect ()
{
    if (!AreThereAnyTabs()) return;
    Tab& tab = GetCurrentTab();
    switch (tab.type)
    {
        case (TabType::Level): LevelEditorClearSelect(); break;
        case (TabType::Map): MapEditorClearSelect(); break;
    }
}

TEINAPI void ToolbarResize ()
{
    if (CurrentTabIsLevel()) LevelEditorResize();
}

TEINAPI void ToolbarToggleEntity ()
{
    if (CurrentTabIsLevel()) gLevelEditor.largeTiles = !gLevelEditor.largeTiles;
}

TEINAPI void ToolbarToggleGuides ()
{
    if (CurrentTabIsLevel()) gLevelEditor.entityGuides = !gLevelEditor.entityGuides;
}
