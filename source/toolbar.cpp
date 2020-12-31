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

        UiFlag brushFlags = UI_NONE;
        UiFlag fillFlags = UI_NONE;
        UiFlag selectFlags = UI_NONE;
        UiFlag gridFlags = UI_NONE;
        UiFlag boundsFlags = UI_NONE;
        UiFlag layerFlags = UI_NONE;
        UiFlag entityFlags = UI_NONE;
        UiFlag guideFlags = UI_NONE;
        UiFlag flipHFlags = UI_NONE;
        UiFlag flipVFlags = UI_NONE;
        UiFlag mirrorHFlags = UI_NONE;
        UiFlag mirrorVFlags = UI_NONE;
        UiFlag cutFlags = UI_LOCKED;
        UiFlag copyFlags = UI_LOCKED;
        UiFlag deselectFlags = UI_LOCKED;
        UiFlag clearFlags = UI_LOCKED;

        brushFlags = ((gLevelEditor.toolType == ToolType::BRUSH) ? UI_NONE : UI_INACTIVE);
        fillFlags = ((gLevelEditor.toolType == ToolType::FILL) ? UI_NONE : UI_INACTIVE);
        selectFlags = ((gLevelEditor.toolType == ToolType::SELECT) ? UI_NONE : UI_INACTIVE);
        gridFlags = ((editor.grid_visible) ? UI_NONE : UI_INACTIVE);
        boundsFlags = ((gLevelEditor.boundsVisible) ? UI_NONE : UI_INACTIVE);
        layerFlags = ((gLevelEditor.layerTransparency) ? UI_NONE : UI_INACTIVE);
        entityFlags = ((gLevelEditor.largeTiles) ? UI_NONE : UI_INACTIVE);
        guideFlags = ((gLevelEditor.entityGuides) ? UI_NONE : UI_INACTIVE);
        mirrorHFlags = ((gLevelEditor.mirrorH) ? UI_NONE : UI_INACTIVE);
        mirrorVFlags = ((gLevelEditor.mirrorV) ? UI_NONE : UI_INACTIVE);

        if (AreAnySelectBoxesVisible())
        {
            cutFlags = UI_NONE;
            copyFlags = UI_NONE;
            deselectFlags = UI_NONE;
            clearFlags = UI_NONE;
        }

        guideFlags |= (gLevelEditor.largeTiles) ? UI_NONE : UI_LOCKED;

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
        BeginPanel(0, 0, gToolbarWidth, GetViewport().h, UI_NONE, gUiColorMedium);

        SetPanelCursor(&cursor);
        SetPanelCursorDir(UI_DIR_DOWN);

        DoImageButton(ToolbarSetToolToBrush, bw,bh, brushFlags, &gClipBrush, gToolbarInfoBrush, gKbToolBrush, "Brush");
        DoImageButton(ToolbarSetToolToFill, bw,bh, fillFlags, &gClipFill, gToolbarInfoFill, gKbToolFill, "Fill");
        DoImageButton(ToolbarSetToolToSelect, bw,bh, selectFlags, &gClipSelect, gToolbarInfoSelect, gKbToolSelect, "Select");
        DoImageButton(ToolbarToggleGrid, bw,bh, gridFlags, &gClipGrid, gToolbarInfoGrid, gKbGridToggle, "Grid");
        DoImageButton(ToolbarToggleBounds, bw,bh, boundsFlags, &gClipBounds, gToolbarInfoBounds, gKbBoundsToggle, "Bounds");
        DoImageButton(ToolbarToggleLayerTransparency, bw,bh, layerFlags, &gClipLayers, gToolbarInfoLayers, gKbLayersToggle, "Layer Transparency");
        DoImageButton(ToolbarResetCamera, bw,bh, UI_NONE, &gClipCamera, gToolbarInfoCamera, gKbCameraReset, "Reset Camera");
        DoImageButton(ToolbarToggleEntity, bw,bh, entityFlags, &gClipEntity, gToolbarInfoEntity, gKbToggleEntity, "Toggle Large Entities");
        DoImageButton(ToolbarToggleGuides, bw,bh, guideFlags, &gClipGuides, gToolbarInfoGuides, gKbToggleGuides, "Toggle Entity Guides");

        // If not all buttons will fit on the screen we will double the width of the
        // toolbar and then place the buttons side-to-side instead of straight down.
        if (GetViewport().h < contentHeight)
        {
            cursor.x += bw + 1;
            cursor.y = gToolbarInnerPad;

            SetPanelCursorDir(UI_DIR_RIGHT);
            DoSeparator((bh*ceilf(gTotalToolbarLevelButtons/2)));
            SetPanelCursorDir(UI_DIR_DOWN);

            cursor.x -= 1;
            cursor.y = gToolbarInnerPad;
        }

        DoImageButton(ToolbarResize, bw,bh, UI_NONE, &gClipResize, gToolbarInfoResize, gKbLevelResize, "Resize");
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

        Tab& tab = get_current_tab();

        UiFlag cutFlags = UI_LOCKED;
        UiFlag copyFlags = UI_LOCKED;
        UiFlag deselectFlags = UI_LOCKED;
        UiFlag clearFlags = UI_LOCKED;

        if (MapSelectBoxPresent())
        {
            cutFlags = UI_NONE;
            copyFlags = UI_NONE;
            deselectFlags = UI_NONE;
            clearFlags = UI_NONE;
        }

        gToolbarWidth = gToolbarDefaultWidth;
        BeginPanel(0, 0, gToolbarWidth, GetViewport().h, UI_NONE, gUiColorMedium);

        SetPanelCursor(&cursor);
        SetPanelCursorDir(UI_DIR_DOWN);

        DoImageButton(ToolbarResetCamera, bw,bh, UI_NONE, &gClipCamera, gToolbarInfoCamera, gKbCameraReset, "Reset Camera");
        DoImageButton(ToolbarCut, bw,bh, cutFlags, &gClipCut, gToolbarInfoCut, gKbCut, "Cut");
        DoImageButton(ToolbarCopy, bw,bh, copyFlags, &gClipCopy, gToolbarInfoCopy, gKbCopy, "Copy");
        DoImageButton(ToolbarDeselect, bw,bh, deselectFlags, &gClipDeselect, gToolbarInfoDeselect, gKbDeselect, "Deselect");
        DoImageButton(ToolbarClearSelect, bw,bh, clearFlags, &gClipClear, gToolbarInfoClear, gKbClearSelect, "Clear Selection");

        EndPanel();
    }
}

TEINAPI void DoToolbar ()
{
    if (!are_there_any_tabs()) return;
    if (current_tab_is_level()) Internal::DoLevelToolbar(); else Internal::DoMapToolbar();
}

TEINAPI float GetToolbarWidth ()
{
    return ((are_there_any_tabs()) ? gToolbarWidth : 0);
}

TEINAPI void ToolbarSetToolToBrush ()
{
    if (!current_tab_is_level()) return;

    if (gLevelEditor.toolType == ToolType::SELECT && gLevelEditor.toolState == ToolState::PLACE)
    {
        Tab& tab = get_current_tab();
        if (!tab.tool_info.select.bounds.empty())
        {
            NewLevelHistoryState(LevelHistoryAction::SELECT_STATE);
        }
    }

    if (gLevelEditor.toolType != ToolType::BRUSH && gLevelEditor.toolState != ToolState::IDLE)
    {
        NewLevelHistoryState(LevelHistoryAction::NORMAL);
    }

    gLevelEditor.toolType = ToolType::BRUSH;
}

TEINAPI void ToolbarSetToolToFill ()
{
    if (!current_tab_is_level()) return;

    if (gLevelEditor.toolType == ToolType::SELECT && gLevelEditor.toolState == ToolState::PLACE)
    {
        Tab& tab = get_current_tab();
        if (!tab.tool_info.select.bounds.empty())
        {
            NewLevelHistoryState(LevelHistoryAction::SELECT_STATE);
        }
    }

    if (gLevelEditor.toolType != ToolType::FILL && gLevelEditor.toolState != ToolState::IDLE)
    {
        NewLevelHistoryState(LevelHistoryAction::NORMAL);
    }

    gLevelEditor.toolType = ToolType::FILL;
}

TEINAPI void ToolbarSetToolToSelect ()
{
    if (!current_tab_is_level()) return;

    if (gLevelEditor.toolType != ToolType::SELECT && gLevelEditor.toolState != ToolState::IDLE)
    {
        NewLevelHistoryState(LevelHistoryAction::NORMAL);
    }

    if (gLevelEditor.toolState == ToolState::PLACE)
    {
        Tab& tab = get_current_tab();
        tab.tool_info.select.start = true;
    }

    gLevelEditor.toolType = ToolType::SELECT;
}

TEINAPI void ToolbarToggleGrid ()
{
    if (current_tab_is_level()) editor.grid_visible = !editor.grid_visible;
}

TEINAPI void ToolbarToggleBounds ()
{
    if (current_tab_is_level()) gLevelEditor.boundsVisible = !gLevelEditor.boundsVisible;
}

TEINAPI void ToolbarToggleLayerTransparency ()
{
    if (current_tab_is_level()) gLevelEditor.layerTransparency = !gLevelEditor.layerTransparency;
}

TEINAPI void ToolbarResetCamera ()
{
    if (!are_there_any_tabs()) return;

    Tab& tab = get_current_tab();
    tab.camera.zoom = 1;

    if (tab.type == Tab_Type::MAP)
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
    if (current_tab_is_level()) FlipLevelH();
}
TEINAPI void ToolbarFlipLevelV ()
{
    if (current_tab_is_level()) FlipLevelV();
}

TEINAPI void ToolbarToggleMirrorH ()
{
    if (current_tab_is_level()) gLevelEditor.mirrorH = !gLevelEditor.mirrorH;
}
TEINAPI void ToolbarToggleMirrorV ()
{
    if (current_tab_is_level()) gLevelEditor.mirrorV = !gLevelEditor.mirrorV;
}

TEINAPI void ToolbarCut ()
{
    if (!are_there_any_tabs()) return;
    Tab& tab = get_current_tab();
    switch (tab.type)
    {
        case (Tab_Type::LEVEL): LevelEditorCut(); break;
        case (Tab_Type::MAP): MapEditorCut(); break;
    }
}

TEINAPI void ToolbarCopy ()
{
    if (!are_there_any_tabs()) return;
    Tab& tab = get_current_tab();
    switch (tab.type)
    {
        case (Tab_Type::LEVEL): LevelEditorCopy(); break;
        case (Tab_Type::MAP): MapEditorCopy(); break;
    }
}

TEINAPI void ToolbarDeselect ()
{
    if (!are_there_any_tabs()) return;
    Tab& tab = get_current_tab();
    switch (tab.type)
    {
        case (Tab_Type::LEVEL): LevelEditorDeselect(); break;
        case (Tab_Type::MAP): MapEditorDeselect(); break;
    }
}

TEINAPI void ToolbarClearSelect ()
{
    if (!are_there_any_tabs()) return;
    Tab& tab = get_current_tab();
    switch (tab.type)
    {
        case (Tab_Type::LEVEL): LevelEditorClearSelect(); break;
        case (Tab_Type::MAP): MapEditorClearSelect(); break;
    }
}

TEINAPI void ToolbarResize ()
{
    if (current_tab_is_level()) LevelEditorResize();
}

TEINAPI void ToolbarToggleEntity ()
{
    if (current_tab_is_level()) gLevelEditor.largeTiles = !gLevelEditor.largeTiles;
}

TEINAPI void ToolbarToggleGuides ()
{
    if (current_tab_is_level()) gLevelEditor.entityGuides = !gLevelEditor.entityGuides;
}
