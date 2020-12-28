/*******************************************************************************
 * Editor GUI widget containing level/map editor specific actions and tools.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr const char* TB_INFO_BRUSH    = "Brush Tool: Allows for placing and erasing of single tiles.";
static constexpr const char* TB_INFO_FILL     = "Fill Tool: Fills or erases a given region with tiles.";
static constexpr const char* TB_INFO_SELECT   = "Select Tool: Click and drag to select tiles for copy, cut, delete, grouping, etc.";
static constexpr const char* TB_INFO_GRID     = "Grid: Toggle the tile grid on and off.";
static constexpr const char* TB_INFO_BOUNDS   = "Bounds: Toggle the shaded out-of-bounds region on and off.";
static constexpr const char* TB_INFO_LAYERS   = "Layer Transparency: Toggles the transparency applied to layers above the active layer.";
static constexpr const char* TB_INFO_CAMERA   = "Reset Camera: Return to the editor's default position and zoom.";
static constexpr const char* TB_INFO_RESIZE   = "Resize: Resize the boundaries of the level.";
static constexpr const char* TB_INFO_FLIP_H   = "Flip Horizontal: Flip the level's content horizontally.";
static constexpr const char* TB_INFO_FLIP_V   = "Flip Vertical: Flip the level's content vertically.";
static constexpr const char* TB_INFO_MIRROR_H = "Mirror Horizontal: Toggle horizontal symmetry on and off.";
static constexpr const char* TB_INFO_MIRROR_V = "Mirror Vertical: Toggle vertical symmetry on and off.";
static constexpr const char* TB_INFO_CUT      = "Cut: Copy and delete the currently selected region.";
static constexpr const char* TB_INFO_COPY     = "Copy: Copy the currently selected region.";
static constexpr const char* TB_INFO_DESELECT = "Deselect: Removes the current box selection.";
static constexpr const char* TB_INFO_CLEAR    = "Clear Selection: Deletes everything inside the current selection.";
static constexpr const char* TB_INFO_ENTITY   = "Large Entities: Toggles entity graphics to represent their in-game size.";
static constexpr const char* TB_INFO_GUIDES   = "Entity Guides: Toggles guides to showcase entity placement and bounds.";

/* -------------------------------------------------------------------------- */

static constexpr float TOTAL_TOOLBAR_LVL_BUTTONS = 18;
static constexpr float TOTAL_TOOLBAR_MAP_BUTTONS =  1;
static           float toolbar_width = TOOLBAR_WIDTH;

/* -------------------------------------------------------------------------- */

TEINAPI void internal__do_level_toolbar ()
{
    Vec2 cursor(TOOLBAR_INNER_PAD, TOOLBAR_INNER_PAD);

    // Dimensions for the toolbar buttons.
    float bw = TOOLBAR_WIDTH - (TOOLBAR_INNER_PAD * 2);
    float bh = 25;

    set_ui_texture(&resource_icons);

    UI_Flag brush_flags    = UI_NONE;
    UI_Flag fill_flags     = UI_NONE;
    UI_Flag select_flags   = UI_NONE;
    UI_Flag grid_flags     = UI_NONE;
    UI_Flag bounds_flags   = UI_NONE;
    UI_Flag layer_flags    = UI_NONE;
    UI_Flag entity_flags   = UI_NONE;
    UI_Flag guide_flags    = UI_NONE;
    UI_Flag flip_h_flags   = UI_NONE;
    UI_Flag flip_v_flags   = UI_NONE;
    UI_Flag mirror_h_flags = UI_NONE;
    UI_Flag mirror_v_flags = UI_NONE;
    UI_Flag cut_flags      = UI_NONE;
    UI_Flag copy_flags     = UI_NONE;
    UI_Flag deselect_flags = UI_NONE;
    UI_Flag clear_flags    = UI_NONE;

    brush_flags    = (level_editor.tool_type == Tool_Type::BRUSH)  ? UI_NONE : UI_INACTIVE;
    fill_flags     = (level_editor.tool_type == Tool_Type::FILL)   ? UI_NONE : UI_INACTIVE;
    select_flags   = (level_editor.tool_type == Tool_Type::SELECT) ? UI_NONE : UI_INACTIVE;
    grid_flags     = (editor.grid_visible)                         ? UI_NONE : UI_INACTIVE;
    bounds_flags   = (level_editor.bounds_visible)                 ? UI_NONE : UI_INACTIVE;
    layer_flags    = (level_editor.layer_transparency)             ? UI_NONE : UI_INACTIVE;
    entity_flags   = (level_editor.large_tiles)                    ? UI_NONE : UI_INACTIVE;
    guide_flags    = (level_editor.entity_guides)                  ? UI_NONE : UI_INACTIVE;
    flip_h_flags   =                                                 UI_NONE;
    flip_v_flags   =                                                 UI_NONE;
    mirror_h_flags = (level_editor.mirror_h)                       ? UI_NONE : UI_INACTIVE;
    mirror_v_flags = (level_editor.mirror_v)                       ? UI_NONE : UI_INACTIVE;

    guide_flags |= (level_editor.large_tiles) ? UI_NONE : UI_LOCKED;

    cut_flags      = (are_any_select_boxes_visible())              ? UI_NONE : UI_LOCKED;
    copy_flags     = (are_any_select_boxes_visible())              ? UI_NONE : UI_LOCKED;
    deselect_flags = (are_any_select_boxes_visible())              ? UI_NONE : UI_LOCKED;
    clear_flags    = (are_any_select_boxes_visible())              ? UI_NONE : UI_LOCKED;

    // If not all buttons will fit on the screen we will double the width of the
    // toolbar and then place the buttons side-to-side instead of straight down.
    float content_height = (bh*TOTAL_TOOLBAR_LVL_BUTTONS)+(TOOLBAR_INNER_PAD*2);
    toolbar_width = TOOLBAR_WIDTH;
    if (get_viewport().h < content_height)
    {
        toolbar_width *= 2;
        toolbar_width -= 1;
    }

    // The toolbar is a vertical list of available tools.
    begin_panel(0, 0, toolbar_width, get_viewport().h, UI_NONE, ui_color_medium);

    set_panel_cursor(&cursor);
    set_panel_cursor_dir(UI_DIR_DOWN);

    do_button_img(tb_set_tool_to_brush,  bw,bh, brush_flags,     &CLIP_BRUSH,           TB_INFO_BRUSH,           gKbToolBrush,     "Brush");
    do_button_img(tb_set_tool_to_fill,   bw,bh, fill_flags,      &CLIP_FILL,            TB_INFO_FILL,            gKbToolFill,      "Fill");
    do_button_img(tb_set_tool_to_select, bw,bh, select_flags,    &CLIP_SELECT,          TB_INFO_SELECT,          gKbToolSelect,    "Select");
    do_button_img(tb_toggle_grid,        bw,bh, grid_flags,      &CLIP_GRID,            TB_INFO_GRID,            gKbGridToggle,    "Grid");
    do_button_img(tb_toggle_bounds,      bw,bh, bounds_flags,    &CLIP_BOUNDS,          TB_INFO_BOUNDS,          gKbBoundsToggle,  "Bounds");
    do_button_img(tb_toggle_layer_trans, bw,bh, layer_flags,     &CLIP_LAYERS,          TB_INFO_LAYERS,          gKbLayersToggle,  "Layer Transparency");
    do_button_img(tb_reset_camera,       bw,bh, UI_NONE,         &CLIP_CAMERA,          TB_INFO_CAMERA,          gKbCameraReset,   "Reset Camera");
    do_button_img(tb_toggle_entity,      bw,bh, entity_flags,    &CLIP_ENTITY,          TB_INFO_ENTITY,          gKbToggleEntity,  "Toggle Large Entities");
    do_button_img(tb_toggle_guides,      bw,bh, guide_flags,     &CLIP_GUIDES,          TB_INFO_GUIDES,          gKbToggleGuides,  "Toggle Entity Guides");

    // If not all buttons will fit on the screen we will double the width of the
    // toolbar and then place the buttons side-to-side instead of straight down.
    if (get_viewport().h < content_height)
    {
        cursor.x += bw + 1;
        cursor.y = TOOLBAR_INNER_PAD;

        set_panel_cursor_dir(UI_DIR_RIGHT);
        do_separator((bh*ceilf(TOTAL_TOOLBAR_LVL_BUTTONS/2)));
        set_panel_cursor_dir(UI_DIR_DOWN);

        cursor.x -= 1;
        cursor.y = TOOLBAR_INNER_PAD;
    }

    do_button_img(tb_resize,             bw,bh, UI_NONE,         &CLIP_RESIZE,          TB_INFO_RESIZE,          gKbLevelResize,   "Resize");
    do_button_img(tb_flip_level_h,       bw,bh, flip_h_flags,    &CLIP_FLIP_H,          TB_INFO_FLIP_H,          gKbFlipH,         "Flip Horizontal");
    do_button_img(tb_flip_level_v,       bw,bh, flip_v_flags,    &CLIP_FLIP_V,          TB_INFO_FLIP_V,          gKbFlipV,         "Flip Vertical");
    do_button_img(tb_toggle_mirror_h,    bw,bh, mirror_h_flags,  &CLIP_MIRROR_H,        TB_INFO_MIRROR_H,        gKbMirrorHToggle, "Mirror Horizontal");
    do_button_img(tb_toggle_mirror_v,    bw,bh, mirror_v_flags,  &CLIP_MIRROR_V,        TB_INFO_MIRROR_V,        gKbMirrorVToggle, "Mirror Vertical");
    do_button_img(tb_cut,                bw,bh, cut_flags,       &CLIP_CUT,             TB_INFO_CUT,             gKbCut,           "Cut");
    do_button_img(tb_copy,               bw,bh, copy_flags,      &CLIP_COPY,            TB_INFO_COPY,            gKbCopy,          "Copy");
    do_button_img(tb_deselect,           bw,bh, deselect_flags,  &CLIP_DESELECT,        TB_INFO_DESELECT,        gKbDeselect,      "Deselect");
    do_button_img(tb_clear_select,       bw,bh, clear_flags,     &CLIP_CLEAR,           TB_INFO_CLEAR,           gKbClearSelect,   "Clear Selection");

    end_panel();
}

/* -------------------------------------------------------------------------- */

TEINAPI void internal__do_map_toolbar ()
{
    Vec2 cursor(TOOLBAR_INNER_PAD, TOOLBAR_INNER_PAD);

    // Dimensions for the toolbar buttons.
    float bw = TOOLBAR_WIDTH - (TOOLBAR_INNER_PAD * 2);
    float bh = 25;

    set_ui_texture(&resource_icons);

    Tab& tab = get_current_tab();

    // @Incomplete: Want to check if the active node's LVL text is a properly formatted as tileset-level!!!
    // @Incomplete: Want to check if the active node's LVL text is a properly formatted as tileset-level!!!
    // @Incomplete: Want to check if the active node's LVL text is a properly formatted as tileset-level!!!
    // @Incomplete: Want to check if the active node's LVL text is a properly formatted as tileset-level!!!
    UI_Flag add_left       = (tab.map_node_info.active && !tab.map_node_info.active->lvl.empty()) ? UI_NONE : UI_LOCKED;
    UI_Flag add_right      = (tab.map_node_info.active && !tab.map_node_info.active->lvl.empty()) ? UI_NONE : UI_LOCKED;
    UI_Flag add_up         = (tab.map_node_info.active && !tab.map_node_info.active->lvl.empty()) ? UI_NONE : UI_LOCKED;
    UI_Flag add_down       = (tab.map_node_info.active && !tab.map_node_info.active->lvl.empty()) ? UI_NONE : UI_LOCKED;
    UI_Flag cut_flags      = (map_select_box_present()) ? UI_NONE : UI_LOCKED;
    UI_Flag copy_flags     = (map_select_box_present()) ? UI_NONE : UI_LOCKED;
    UI_Flag deselect_flags = (map_select_box_present()) ? UI_NONE : UI_LOCKED;
    UI_Flag clear_flags    = (map_select_box_present()) ? UI_NONE : UI_LOCKED;

    toolbar_width = TOOLBAR_WIDTH;
    begin_panel(0, 0, toolbar_width, get_viewport().h, UI_NONE, ui_color_medium);

    set_panel_cursor(&cursor);
    set_panel_cursor_dir(UI_DIR_DOWN);

    do_button_img(tb_reset_camera, bw,bh, UI_NONE,        &CLIP_CAMERA,   TB_INFO_CAMERA,   gKbCameraReset, "Reset Camera"   );
    do_button_img(tb_cut,          bw,bh, cut_flags,      &CLIP_CUT,      TB_INFO_CUT,      gKbCut,         "Cut"            );
    do_button_img(tb_copy,         bw,bh, copy_flags,     &CLIP_COPY,     TB_INFO_COPY,     gKbCopy,        "Copy"           );
    do_button_img(tb_deselect,     bw,bh, deselect_flags, &CLIP_DESELECT, TB_INFO_DESELECT, gKbDeselect,    "Deselect"       );
    do_button_img(tb_clear_select, bw,bh, clear_flags,    &CLIP_CLEAR,    TB_INFO_CLEAR,    gKbClearSelect, "Clear Selection");

    end_panel();
}

/* -------------------------------------------------------------------------- */

TEINAPI void do_toolbar ()
{
    if (are_there_any_tabs())
    {
        if (current_tab_is_level()) internal__do_level_toolbar();
        else                        internal__do_map_toolbar();
    }
}

TEINAPI float get_toolbar_w ()
{
    return ((are_there_any_tabs()) ? toolbar_width : 0);
}

/* -------------------------------------------------------------------------- */

TEINAPI void tb_set_tool_to_brush ()
{
    if (!current_tab_is_level()) return;

    if (level_editor.tool_type == Tool_Type::SELECT && level_editor.tool_state == Tool_State::PLACE)
    {
        Tab& tab = get_current_tab();
        if (!tab.tool_info.select.bounds.empty())
        {
            new_level_history_state(Level_History_Action::SELECT_STATE);
        }
    }

    if (level_editor.tool_type != Tool_Type::BRUSH && level_editor.tool_state != Tool_State::IDLE)
    {
        new_level_history_state(Level_History_Action::NORMAL);
    }

    level_editor.tool_type = Tool_Type::BRUSH;
}

TEINAPI void tb_set_tool_to_fill ()
{
    if (!current_tab_is_level()) return;

    if (level_editor.tool_type == Tool_Type::SELECT && level_editor.tool_state == Tool_State::PLACE)
    {
        Tab& tab = get_current_tab();
        if (!tab.tool_info.select.bounds.empty())
        {
            new_level_history_state(Level_History_Action::SELECT_STATE);
        }
    }

    if (level_editor.tool_type != Tool_Type::FILL && level_editor.tool_state != Tool_State::IDLE)
    {
        new_level_history_state(Level_History_Action::NORMAL);
    }

    level_editor.tool_type = Tool_Type::FILL;
}

TEINAPI void tb_set_tool_to_select ()
{
    if (!current_tab_is_level()) return;

    if (level_editor.tool_type != Tool_Type::SELECT && level_editor.tool_state != Tool_State::IDLE)
    {
        new_level_history_state(Level_History_Action::NORMAL);
    }

    if (level_editor.tool_state == Tool_State::PLACE)
    {
        Tab& tab = get_current_tab();
        tab.tool_info.select.start = true;
    }

    level_editor.tool_type = Tool_Type::SELECT;
}

/* -------------------------------------------------------------------------- */

TEINAPI void tb_toggle_grid ()
{
    if (current_tab_is_level()) editor.grid_visible = !editor.grid_visible;
}

TEINAPI void tb_toggle_bounds ()
{
    if (current_tab_is_level()) level_editor.bounds_visible = !level_editor.bounds_visible;
}

TEINAPI void tb_toggle_layer_trans ()
{
    if (current_tab_is_level()) level_editor.layer_transparency = !level_editor.layer_transparency;
}

/* -------------------------------------------------------------------------- */

TEINAPI void tb_reset_camera ()
{
    if (!are_there_any_tabs()) return;

    Tab& tab = get_current_tab();
    tab.camera.zoom = 1;

    if (tab.type == Tab_Type::MAP)
    {
        tab.camera.x = -get_min_map_bounds_x();
        tab.camera.y = -get_min_map_bounds_y();
    }
    else
    {
        tab.camera.x = 0;
        tab.camera.y = 0;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void tb_flip_level_h ()
{
    if (current_tab_is_level()) flip_level_h();
}

TEINAPI void tb_flip_level_v ()
{
    if (current_tab_is_level()) flip_level_v();
}

/* -------------------------------------------------------------------------- */

TEINAPI void tb_toggle_mirror_h ()
{
    if (current_tab_is_level()) level_editor.mirror_h = !level_editor.mirror_h;
}

TEINAPI void tb_toggle_mirror_v ()
{
    if (current_tab_is_level()) level_editor.mirror_v = !level_editor.mirror_v;
}

/* -------------------------------------------------------------------------- */

TEINAPI void tb_cut ()
{
    if (!are_there_any_tabs()) return;

    Tab& tab = get_current_tab();
    switch (tab.type)
    {
        case (Tab_Type::LEVEL): le_cut(); break;
        case (Tab_Type::MAP  ): me_cut(); break;
    }
}

TEINAPI void tb_copy ()
{
    if (!are_there_any_tabs()) return;

    Tab& tab = get_current_tab();
    switch (tab.type)
    {
        case (Tab_Type::LEVEL): le_copy(); break;
        case (Tab_Type::MAP  ): me_copy(); break;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void tb_deselect ()
{
    if (!are_there_any_tabs()) return;

    Tab& tab = get_current_tab();
    switch (tab.type)
    {
        case (Tab_Type::LEVEL): le_deselect(); break;
        case (Tab_Type::MAP  ): me_deselect(); break;
    }
}

TEINAPI void tb_clear_select ()
{
    if (!are_there_any_tabs()) return;

    Tab& tab = get_current_tab();
    switch (tab.type)
    {
        case (Tab_Type::LEVEL): le_clear_select(); break;
        case (Tab_Type::MAP  ): me_clear_select(); break;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void tb_resize ()
{
    if (current_tab_is_level()) le_resize();
}

/* -------------------------------------------------------------------------- */

TEINAPI void tb_toggle_entity ()
{
    if (current_tab_is_level()) level_editor.large_tiles = !level_editor.large_tiles;
}

TEINAPI void tb_toggle_guides ()
{
    if (current_tab_is_level()) level_editor.entity_guides = !level_editor.entity_guides;
}

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

/*******************************************************************************
 *
 * Copyright (c) 2020 Joshua Robertson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
*******************************************************************************/
