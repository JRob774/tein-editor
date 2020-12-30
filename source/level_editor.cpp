/*******************************************************************************
 * THe systems and functionality for the level editing portion of the editor.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float GHOSTED_CURSOR_ALPHA = .5f;
static constexpr TileID CAMERA_ID = 20000;

/* -------------------------------------------------------------------------- */

TEINAPI Quad& internal__get_tile_graphic_clip (TextureAtlas& atlas, TileID id)
{
    if (level_editor.large_tiles && atlas.clips.count(id + gAltOffset))
    {
        id += gAltOffset;
    }
    return GetAtlasClip(atlas, id);
}

TEINAPI bool internal__are_active_layers_in_bounds_empty (int x, int y, int w, int h)
{
    const Tab& tab = get_current_tab();
    for (size_t i=0; i<tab.level.data.size(); ++i)
    {
        if (tab.tile_layer_active[i])
        {
            const auto& tile_layer = tab.level.data.at(i);
            for (int iy=y; iy<(y+h); ++iy)
            {
                for (int ix=x; ix<(x+w); ++ix)
                {
                    TileID id = tile_layer.at(iy*tab.level.header.width+ix);
                    if (id != 0) return false;
                }
            }
        }
    }
    return true;
}

TEINAPI Level_History_State& internal__get_current_history_state ()
{
    Tab& tab = get_current_tab();
    return tab.level_history.state[tab.level_history.current_position];
}

TEINAPI bool internal__tile_in_bounds (int x, int y)
{
    const Tab& tab = get_current_tab();

    int w = tab.level.header.width;
    int h = tab.level.header.height;

    return (x >= 0 && x < w && y >= 0 && y < h);
}

TEINAPI void internal__place_tile_clear (int x, int y, TileID id, LevelLayer tile_layer)
{
    Tab& tab = get_current_tab();

    if (!tab.tile_layer_active[tile_layer]) return;
    if (!internal__tile_in_bounds(x, y))    return;

    auto& layer = tab.level.data[tile_layer];

    Level_History_Info info = {};
    info.x                  = x;
    info.y                  = y;
    info.old_id             = layer[y * tab.level.header.width + x];
    info.new_id             = id;
    info.tile_layer         = tile_layer;
    add_to_history_clear_state(info);

    layer[y * tab.level.header.width + x] = id;

    get_current_tab().unsaved_changes = true;
}

TEINAPI void internal__place_tile (int x, int y, TileID id, LevelLayer tile_layer)
{
    Tab& tab = get_current_tab();

    if (!tab.tile_layer_active[tile_layer]) return;
    if (!internal__tile_in_bounds(x, y))    return;

    auto& layer = tab.level.data[tile_layer];

    Level_History_Info info = {};
    info.x                  = x;
    info.y                  = y;
    info.old_id             = layer[y * tab.level.header.width + x];
    info.new_id             = id;
    info.tile_layer         = tile_layer;
    add_to_history_normal_state(info);

    layer[y * tab.level.header.width + x] = id;

    get_current_tab().unsaved_changes = true;
}

TEINAPI void internal__place_mirrored_tile_clear (int x, int y, TileID id, LevelLayer tile_layer)
{
    bool both = (level_editor.mirror_h && level_editor.mirror_v);

    const Tab& tab = get_current_tab();

    int lw = tab.level.header.width-1;
    int lh = tab.level.header.height-1;

                               internal__place_tile_clear(   x,    y,                                                 id  , tile_layer);
    if (level_editor.mirror_h) internal__place_tile_clear(lw-x,    y, GetTileHorizontalFlip                       (id) , tile_layer);
    if (level_editor.mirror_v) internal__place_tile_clear(   x, lh-y,                          GetTileVerticalFlip(id) , tile_layer);
    if (both)                  internal__place_tile_clear(lw-x, lh-y, GetTileHorizontalFlip(GetTileVerticalFlip(id)), tile_layer);
}

TEINAPI void internal__place_mirrored_tile (int x, int y, TileID id, LevelLayer tile_layer)
{
    bool both = (level_editor.mirror_h && level_editor.mirror_v);

    const Tab& tab = get_current_tab();

    int lw = tab.level.header.width-1;
    int lh = tab.level.header.height-1;

                               internal__place_tile(   x,    y,                                                 id  , tile_layer);
    if (level_editor.mirror_h) internal__place_tile(lw-x,    y, GetTileHorizontalFlip                       (id) , tile_layer);
    if (level_editor.mirror_v) internal__place_tile(   x, lh-y,                          GetTileVerticalFlip(id) , tile_layer);
    if (both)                  internal__place_tile(lw-x, lh-y, GetTileHorizontalFlip(GetTileVerticalFlip(id)), tile_layer);
}

TEINAPI bool internal__clipboard_empty ()
{
    for (auto& clipboard: level_editor.clipboard)
    {
        for (auto& layer: clipboard.data)
        {
            if (!layer.empty()) return false;
        }
    }
    return true;
}

TEINAPI void internal__copy ()
{
    Tab& tab = get_current_tab();

    if (are_any_select_boxes_visible())
    {
        // Clear the old clipboard content now we know we can actually copy.
        level_editor.clipboard.clear();

        int sl = 0;
        int st = 0;
        int sr = 0;
        int sb = 0;

        get_total_select_boundary(&sl,&st,&sr,&sb);

        for (auto& bounds: tab.tool_info.select.bounds)
        {
            if (bounds.visible)
            {
                level_editor.clipboard.push_back(Level_Clipboard());
                Level_Clipboard& clipboard = level_editor.clipboard.back();

                int l, t, r, b;
                get_ordered_select_bounds(bounds, &l, &t, &r, &b);

                int w = (r-l)+1;
                int h = (t-b)+1;

                // Resize the clipboard tile buffer to be the new selection box size.
                for (auto& layer: clipboard.data) layer.assign(w*h, 0);

                // Important to cache so we can use during paste.
                clipboard.x = l - sl;
                clipboard.y = t - st;
                clipboard.w = w;
                clipboard.h = h;

                // Copy the selected tiles into the buffer.
                for (size_t i=0; i<clipboard.data.size(); ++i)
                {
                    if (!tab.tile_layer_active[i]) continue;

                    const auto& src_layer = tab.level.data[i];
                    auto& dst_layer = clipboard.data[i];
                    for (int y=b; y<=t; ++y)
                    {
                        for (int x=l; x<=r; ++x)
                        {
                            dst_layer[(y-b) * w + (x-l)] = src_layer[y * tab.level.header.width + x];
                        }
                    }
                }
            }
        }
    }
}

TEINAPI Vec2 internal__mouse_to_tile_position ()
{
    // Only continue calculating the tile position if the mouse is in bounds.
    if (!mouse_inside_level_editor_viewport()) return Vec2(-1,-1);

    Vec2 m = level_editor.mouse_world;

    m.x = floorf((m.x - level_editor.bounds.x) / DEFAULT_TILE_SIZE);
    m.y = floorf((m.y - level_editor.bounds.y) / DEFAULT_TILE_SIZE);

    return m;
}

TEINAPI void internal__handle_brush ()
{
    Vec2 tile_pos = internal__mouse_to_tile_position();

    int x = static_cast<int>(tile_pos.x);
    int y = static_cast<int>(tile_pos.y);

    bool place = (level_editor.tool_state == Tool_State::PLACE);
    TileID id = (place) ? GetSelectedTile() : 0;
    internal__place_mirrored_tile(x, y, id, GetSelectedLayer());
}

TEINAPI TileID internal__get_fill_find_id (int x, int y, LevelLayer layer)
{
    if (!internal__tile_in_bounds(x, y)) return 0;
    const auto& tab = get_current_tab();
    return tab.level.data[layer][y * tab.level.header.width + x];
}

TEINAPI bool internal__inside_select_bounds (int x, int y)
{
    if (!are_there_any_tabs()) return false;

    const Tab& tab = get_current_tab();
    for (auto& bounds: tab.tool_info.select.bounds)
    {
        if (bounds.visible)
        {
            int sl,st,sr,sb; get_ordered_select_bounds(bounds, &sl,&st,&sr,&sb);
            if (x >= sl && x <= sr && y >= sb && y <= st) return true;
        }
    }
    return false;
}

TEINAPI void internal__check_fill_neighbour (int x, int y)
{
    Tab& tab = get_current_tab();

    // If the select box is visible then check if we should be filling inside
    // or outside of the select box bounds. Based on that case we discard any
    // tiles/spawns that do not fit in the bounds we are to be filling within.
    if (are_any_select_boxes_visible())
    {
        if (tab.tool_info.fill.inside_select)
        {
            if (!internal__inside_select_bounds(x, y)) return;
        }
        else
        {
            if (internal__inside_select_bounds(x, y)) return;
        }
    }

    // If we have already visited this space then we don't need to again.
    size_t index = y * tab.level.header.width + x;
    if (tab.tool_info.fill.searched.at(index)) return;

    if (internal__get_fill_find_id(x, y, tab.tool_info.fill.layer) == tab.tool_info.fill.find_id)
    {
        internal__place_mirrored_tile(x, y, tab.tool_info.fill.replace_id, tab.tool_info.fill.layer);
        tab.tool_info.fill.frontier.push_back({ x, y });
    }

    tab.tool_info.fill.searched.at(index) = true; // Mark as searched!
}

TEINAPI void internal__fill ()
{
    Tab& tab = get_current_tab();

    int w = tab.level.header.width;
    int h = tab.level.header.height;

    tab.tool_info.fill.searched.resize(w*h, false);

    int start_x = static_cast<int>(tab.tool_info.fill.start.x);
    int start_y = static_cast<int>(tab.tool_info.fill.start.y);

    // Start tile marked searched as we can just replace it now.
    internal__place_mirrored_tile(start_x, start_y, tab.tool_info.fill.replace_id, tab.tool_info.fill.layer);

    tab.tool_info.fill.searched.at(start_y * w + start_x) = true;
    tab.tool_info.fill.frontier.push_back(tab.tool_info.fill.start);

    // Once the frontier is empty this means all tiles within the potentially
    // enclosed space have been filled and there is no where else to expand
    // to within the level. Meaning that the fill is complete and we can leave.
    while (tab.tool_info.fill.frontier.size() > 0)
    {
        Vec2 temp = tab.tool_info.fill.frontier.at(0);

        int cx = static_cast<int>(temp.x);
        int cy = static_cast<int>(temp.y);

        tab.tool_info.fill.frontier.erase(tab.tool_info.fill.frontier.begin());

        // Check the neighbors, but don't try to access outside level bounds.
        if (cy > 0)     internal__check_fill_neighbour(cx,   cy-1);
        if (cx < (w-1)) internal__check_fill_neighbour(cx+1, cy  );
        if (cy < (h-1)) internal__check_fill_neighbour(cx,   cy+1);
        if (cx > 0)     internal__check_fill_neighbour(cx-1, cy  );
    }

    tab.tool_info.fill.searched.clear();
    tab.tool_info.fill.frontier.clear();
}

TEINAPI void internal__replace ()
{
    Tab& tab = get_current_tab();

    auto& layer = tab.level.data[tab.tool_info.fill.layer];
    for (int y=0; y<tab.level.header.height; ++y)
    {
        for (int x=0; x<tab.level.header.width; ++x)
        {
            // If the select box is visible then check if we should be replacing inside
            // or outside of the select box bounds. Based on that case we discard any
            // tiles/spawns that do not fit in the bounds we are to be replacing within.
            if (are_any_select_boxes_visible())
            {
                if (tab.tool_info.fill.inside_select)
                {
                    if (!internal__inside_select_bounds(x, y)) continue;
                }
                else
                {
                    if (internal__inside_select_bounds(x, y)) continue;
                }
            }

            if (layer[y * tab.level.header.width + x] == tab.tool_info.fill.find_id)
            {
                internal__place_tile(x, y, tab.tool_info.fill.replace_id, tab.tool_info.fill.layer);
            }
        }
    }
}

TEINAPI void internal__handle_fill ()
{
    Vec2 tile_pos = internal__mouse_to_tile_position();

    int x = static_cast<int>(tile_pos.x);
    int y = static_cast<int>(tile_pos.y);

    // Do not bother starting a fill if out of bounds!
    if (!internal__tile_in_bounds(x, y)) return;

    Tab& tab = get_current_tab();

    bool place = (level_editor.tool_state == Tool_State::PLACE);
    TileID id = (place) ? GetSelectedTile() : 0;

    tab.tool_info.fill.start      = { tile_pos.x, tile_pos.y };
    tab.tool_info.fill.layer      = GetSelectedLayer();
    tab.tool_info.fill.find_id    = internal__get_fill_find_id(x, y, tab.tool_info.fill.layer);
    tab.tool_info.fill.replace_id = id;

    // Determine if the origin of the fill is inside a selection box or not.
    // This part does not matter if a selection box is not currently present.
    tab.tool_info.fill.inside_select = false;
    for (auto& bounds: tab.tool_info.select.bounds)
    {
        if (bounds.visible)
        {
            int sl,st,sr,sb; get_ordered_select_bounds(bounds, &sl,&st,&sr,&sb);
            if (x >= sl && x <= sr && y >= sb && y <= st) tab.tool_info.fill.inside_select = true;
        }
    }

    // If the IDs are the same there is no need to fill.
    if (tab.tool_info.fill.find_id == tab.tool_info.fill.replace_id) return;

    // Determine if we are doing a fill or find/replace.
    if (IsKeyModStateActive(KMOD_ALT)) internal__replace();
    else                               internal__fill();
}

TEINAPI void internal__restore_select_state (const std::vector<Select_Bounds>& select_state)
{
    if (are_there_any_tabs())
    {
        Tab& tab = get_current_tab();
        tab.tool_info.select.bounds = select_state;
    }
}

TEINAPI void internal__deselect ()
{
    if (!are_there_any_tabs()) return;
    Tab& tab = get_current_tab();
    tab.tool_info.select.bounds.clear();
}

TEINAPI void internal__handle_select ()
{
    // Right clicking whilst using the select tool remove the current selection.
    if (level_editor.tool_state == Tool_State::ERASE)
    {
        internal__deselect();
        return;
    }

    // We do this because otherwise if we have a selection box up and we click
    // outside of the level editor viewport region then it will try calculate
    // a new selection box -- instead we want it to do absolutely nothing.
    if (!mouse_inside_level_editor_viewport()) return;

    Tab& tab = get_current_tab();

    // If it is the start of a new selection then we do some extra stuff.
    if (tab.tool_info.select.start)
    {
        // Cache the old state so we can add it to the history for undo/redo.
        tab.old_select_state = tab.tool_info.select.bounds;

        // Clear the old selection box(es) if we are not adding a new one.
        if (!tab.tool_info.select.add)
        {
            tab.tool_info.select.bounds.clear();
        }
        tab.tool_info.select.bounds.push_back(Select_Bounds());

        Select_Bounds& select_bounds = tab.tool_info.select.bounds.back();

        Vec2 ta = internal__mouse_to_tile_position();
        // Set the starting anchor point of the selection (clamp in bounds).
        select_bounds.top  = std::clamp(static_cast<int>(ta.y), 0, tab.level.header.height-1);
        select_bounds.left = std::clamp(static_cast<int>(ta.x), 0, tab.level.header.width-1);

        tab.tool_info.select.start = false;
        select_bounds.visible = false;
    }

    Select_Bounds& select_bounds = tab.tool_info.select.bounds.back();

    // We do this initial check so that if the user started by dragging
    // their selection box from outside the editor bounds then it will
    // not start the actual selection until their mouse enters the bounds.
    if (!select_bounds.visible)
    {
        Vec2 a = internal__mouse_to_tile_position();
        if (internal__tile_in_bounds(static_cast<int>(a.x), static_cast<int>(a.y)))
        {
            select_bounds.visible = true;
        }
    }

    if (select_bounds.visible)
    {
        // Set the second point of the selection box bounds.
        Vec2 tb = internal__mouse_to_tile_position();
        select_bounds.bottom = std::clamp(static_cast<int>(tb.y), 0, tab.level.header.height-1);
        select_bounds.right  = std::clamp(static_cast<int>(tb.x), 0, tab.level.header.width-1);
    }
}

TEINAPI void internal__handle_current_tool ()
{
    // Don't need to do anything, the user isn't using the tool right now!
    if (level_editor.tool_state == Tool_State::IDLE) return;

    switch (level_editor.tool_type)
    {
        case (Tool_Type::BRUSH ): internal__handle_brush();  break;
        case (Tool_Type::FILL  ): internal__handle_fill();   break;
        case (Tool_Type::SELECT): internal__handle_select(); break;
    }
}

TEINAPI void internal__flip_level_h (const bool tile_layer_active[LEVEL_LAYER_TOTAL])
{
    Tab& tab = get_current_tab();

    int lw = tab.level.header.width;
    int lh = tab.level.header.height;

    int x = 0;
    int y = 0;
    int w = lw;
    int h = lh;

    // Flip all of the level's tiles.
    for (int i=0; i<LEVEL_LAYER_TOTAL; ++i)
    {
        if (!tile_layer_active[i]) continue;

        auto& layer = tab.level.data[i];
        // Swap the tile columns from left-to-right for each row.
        for (int j=y; j<(y+h); ++j)
        {
            int r = x + (j*lw) + (w-1);
            int l = x + (j*lw);

            // Stop after we hit the middle point (no need to flip).
            while (l < r)
            {
                TileID rt = layer[r];
                TileID lt = layer[l];

                layer[r--] = GetTileHorizontalFlip(lt);
                layer[l++] = GetTileHorizontalFlip(rt);
            }
        }
    }

    get_current_tab().unsaved_changes = true;
}

TEINAPI void internal__flip_level_v (const bool tile_layer_active[LEVEL_LAYER_TOTAL])
{
    Tab& tab = get_current_tab();

    int lw = tab.level.header.width;
    int lh = tab.level.header.height;

    int x = 0;
    int y = 0;
    int w = lw;
    int h = lh;

    // Flip all of the level's tiles.
    std::vector<TileID> temp_row;
    temp_row.resize(w);

    for (int i=0; i<LEVEL_LAYER_TOTAL; ++i)
    {
        if (!tile_layer_active[i]) continue;

        auto& layer = tab.level.data[i];
        size_t pitch = w * sizeof(TileID);

        int b = (y * lw) + x;
        int t = ((y+h-1) * lw) + x;

        // Stop after we hit the middle point (no need to flip).
        while (b < t)
        {
            memcpy(&temp_row[0], &layer   [b], pitch);
            memcpy(&layer   [b], &layer   [t], pitch);
            memcpy(&layer   [t], &temp_row[0], pitch);

            for (int j=0; j<lw; ++j)
            {
                layer[t+j] = GetTileVerticalFlip(layer[t+j]);
                layer[b+j] = GetTileVerticalFlip(layer[b+j]);
            }

            b += lw;
            t -= lw;
        }
    }

    get_current_tab().unsaved_changes = true;
}

TEINAPI void internal__draw_cursor (int x, int y, TileID id)
{
    // Do not try to draw the cursor if it is out of bounds!
    if (!internal__tile_in_bounds(x, y)) return;

    // We don't want to do this if select is the current tool.
    if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
    {
        float gx = level_editor.bounds.x + (x * DEFAULT_TILE_SIZE);
        float gy = level_editor.bounds.y + (y * DEFAULT_TILE_SIZE);

        StencilModeDraw();
        SetDrawColor(gEditorSettings.cursorColor);
        FillQuad(gx, gy, gx+DEFAULT_TILE_SIZE, gy+DEFAULT_TILE_SIZE);

        TextureAtlas& atlas = GetEditorAtlasLarge();

        atlas.texture.color.a = GHOSTED_CURSOR_ALPHA;
        DrawTexture(atlas.texture, gx+DEFAULT_TILE_SIZE_HALF, gy+DEFAULT_TILE_SIZE_HALF, &internal__get_tile_graphic_clip(atlas, id));
        atlas.texture.color.a = 1; // Important!

        StencilModeErase();
        SetDrawColor(1,1,1,1);
        FillQuad(gx, gy, gx+DEFAULT_TILE_SIZE, gy+DEFAULT_TILE_SIZE);

        atlas.texture.color = Vec4(1,1,1,1);
        DrawTexture(atlas.texture, gx+DEFAULT_TILE_SIZE_HALF, gy+DEFAULT_TILE_SIZE_HALF, &internal__get_tile_graphic_clip(atlas, id));
    }
}

TEINAPI void internal__draw_mirrored_cursor ()
{
    // No need to draw if we do not have focus.
    if (!IsWindowFocused("WINMAIN")) return;

    bool both = (level_editor.mirror_h && level_editor.mirror_v);

    const Tab& tab = get_current_tab();

    int lw = tab.level.header.width-1;
    int lh = tab.level.header.height-1;

    Vec2 t = internal__mouse_to_tile_position();

    int tx = static_cast<int>(t.x);
    int ty = static_cast<int>(t.y);

    TileID id = GetSelectedTile();

    BeginStencil();

                               internal__draw_cursor(   tx,    ty,                                                 id  );
    if (level_editor.mirror_h) internal__draw_cursor(lw-tx,    ty, GetTileHorizontalFlip                       (id) );
    if (level_editor.mirror_v) internal__draw_cursor(   tx, lh-ty,                          GetTileVerticalFlip(id) );
    if (both)                  internal__draw_cursor(lw-tx, lh-ty, GetTileHorizontalFlip(GetTileVerticalFlip(id)));

    EndStencil();
}

TEINAPI void internal__draw_clipboard_highlight (UiDir xdir, UiDir ydir)
{
    BeginStencil();
    for (auto& clipboard: level_editor.clipboard)
    {
        const Tab& tab = get_current_tab();

        int lw = tab.level.header.width-1;
        int lh = tab.level.header.height-1;

        int sw = clipboard.w-1;
        int sh = clipboard.h-1;

        Vec2 t = internal__mouse_to_tile_position();

        int tx = static_cast<int>(t.x) + clipboard.x;
        int ty = static_cast<int>(t.y) + clipboard.y;

        if (xdir == UI_DIR_LEFT) tx = lw-sw-tx;
        if (ydir == UI_DIR_DOWN) ty = lh-sh-ty;

        float gx = level_editor.bounds.x + (tx * DEFAULT_TILE_SIZE);
        float gy = level_editor.bounds.y + (ty * DEFAULT_TILE_SIZE);
        float gw = clipboard.w * DEFAULT_TILE_SIZE;
        float gh = clipboard.h * DEFAULT_TILE_SIZE;

        StencilModeDraw();
        SetDrawColor(gEditorSettings.cursorColor);
        FillQuad(gx, gy, gx+gw, gy+gh);

        StencilModeErase();
        SetDrawColor(1,1,1,1);
        FillQuad(gx, gy, gx+gw, gy+gh);
    }
    EndStencil();
}

TEINAPI void internal__draw_clipboard (UiDir xdir, UiDir ydir)
{
    TextureAtlas& atlas = GetEditorAtlasLarge();

    SetTileBatchTexture(atlas.texture);
    SetTileBatchColor(Vec4(1,1,1,GHOSTED_CURSOR_ALPHA));

    // Stops us from drawing multiple copies of a tile where clipboards overlap.
    std::array<std::map<size_t, bool>, LEVEL_LAYER_TOTAL> tile_space_occupied;
    for (auto& clipboard: level_editor.clipboard)
    {
        const Tab& tab = get_current_tab();

        int lw = tab.level.header.width-1;
        int lh = tab.level.header.height-1;

        int sw = clipboard.w-1;
        int sh = clipboard.h-1;

        Vec2 t = internal__mouse_to_tile_position();

        int x = static_cast<int>(t.x) + clipboard.x;
        int y = static_cast<int>(t.y) + clipboard.y;

        if (xdir == UI_DIR_LEFT) x = lw-sw-x;
        if (ydir == UI_DIR_DOWN) y = lh-sh-y;

        float gx = level_editor.bounds.x + (x * DEFAULT_TILE_SIZE);
        float gy = level_editor.bounds.y + (y * DEFAULT_TILE_SIZE);
        float gw = clipboard.w * DEFAULT_TILE_SIZE;
        float gh = clipboard.h * DEFAULT_TILE_SIZE;

        // Draw all of the select buffer tiles.
        for (int i=0; i<clipboard.data.size(); ++i)
        {
            // If the layer is not active then we do not bother drawing its clipboard content.
            if (!tab.tile_layer_active[i]) continue;

            // We start the Y axis from the bottom because the game stores data
            // with coordinate (0,0) being the bottom left of the current level.
            float ty = 0;
            float tx = 0;

            if      (xdir == UI_DIR_RIGHT) tx = gx;
            else if (xdir == UI_DIR_LEFT ) tx = gx+gw-DEFAULT_TILE_SIZE;
            if      (ydir == UI_DIR_UP   ) ty = gy;
            else if (ydir == UI_DIR_DOWN ) ty = gy+gh-DEFAULT_TILE_SIZE;

            const auto& layer = clipboard.data[i];
            auto& layer_space_occupied = tile_space_occupied[i];

            for (size_t j=0; j<layer.size(); ++j)
            {
                TileID id = layer[j];
                if (id) // No point drawing empty tiles...
                {
                    if (!layer_space_occupied.count(j))
                    {
                        if (xdir == UI_DIR_LEFT) id = GetTileHorizontalFlip(id);
                        if (ydir == UI_DIR_DOWN) id = GetTileVerticalFlip(id);

                        DrawBatchedTile(tx+DEFAULT_TILE_SIZE_HALF, ty+DEFAULT_TILE_SIZE_HALF, &internal__get_tile_graphic_clip(atlas, id));
                        layer_space_occupied.insert(std::pair<size_t, bool>(j, true));
                    }
                }

                // Move to the next tile based on our direction.
                if (xdir == UI_DIR_RIGHT && ydir == UI_DIR_UP)
                {
                    tx += DEFAULT_TILE_SIZE;
                    if ((tx+DEFAULT_TILE_SIZE) > (gx+gw))
                    {
                        ty += DEFAULT_TILE_SIZE;
                        tx = gx;
                    }
                }
                else if (xdir == UI_DIR_LEFT && ydir == UI_DIR_UP)
                {
                    tx -= DEFAULT_TILE_SIZE;
                    if ((tx) < gx)
                    {
                        ty += DEFAULT_TILE_SIZE;
                        tx = gx+gw-DEFAULT_TILE_SIZE;
                    }
                }
                else if (xdir == UI_DIR_RIGHT && ydir == UI_DIR_DOWN)
                {
                    tx += DEFAULT_TILE_SIZE;
                    if ((tx+DEFAULT_TILE_SIZE) > (gx+gw))
                    {
                        ty -= DEFAULT_TILE_SIZE;
                        tx = gx;
                    }
                }
                else if (xdir == UI_DIR_LEFT && ydir == UI_DIR_DOWN)
                {
                    tx -= DEFAULT_TILE_SIZE;
                    if ((tx) < gx)
                    {
                        ty -= DEFAULT_TILE_SIZE;
                        tx = gx+gw-DEFAULT_TILE_SIZE;
                    }
                }
            }
        }
    }

    FlushBatchedTiles();
}

TEINAPI void internal__draw_mirrored_clipboard ()
{
    bool both = (level_editor.mirror_h && level_editor.mirror_v);

    if (!IsWindowFocused("WINMAIN")) return;

                               internal__draw_clipboard_highlight(UI_DIR_RIGHT, UI_DIR_UP  );
    if (level_editor.mirror_h) internal__draw_clipboard_highlight(UI_DIR_LEFT,  UI_DIR_UP  );
    if (level_editor.mirror_v) internal__draw_clipboard_highlight(UI_DIR_RIGHT, UI_DIR_DOWN);
    if (both)                  internal__draw_clipboard_highlight(UI_DIR_LEFT,  UI_DIR_DOWN);
                               internal__draw_clipboard          (UI_DIR_RIGHT, UI_DIR_UP  );
    if (level_editor.mirror_h) internal__draw_clipboard          (UI_DIR_LEFT,  UI_DIR_UP  );
    if (level_editor.mirror_v) internal__draw_clipboard          (UI_DIR_RIGHT, UI_DIR_DOWN);
    if (both)                  internal__draw_clipboard          (UI_DIR_LEFT,  UI_DIR_DOWN);
}

TEINAPI void internal__dump_level_history ()
{
    const Tab& tab = get_current_tab();

    BeginDebugSection("History Stack Dump:");
    for (int i=0; i<static_cast<int>(tab.level_history.state.size()); ++i)
    {
        const Level_History_State& s = tab.level_history.state.at(i);
        std::string history_state = (tab.level_history.current_position==i) ? ">" : " ";

        switch (s.action)
        {
            case (Level_History_Action::NORMAL       ): history_state += "| NORMAL | "; break;
            case (Level_History_Action::FLIP_LEVEL_H ): history_state += "| FLIP H | "; break;
            case (Level_History_Action::FLIP_LEVEL_V ): history_state += "| FLIP V | "; break;
            case (Level_History_Action::SELECT_STATE ): history_state += "| SELECT | "; break;
            case (Level_History_Action::CLEAR        ): history_state += "| CLEAR  | "; break;
            case (Level_History_Action::RESIZE       ): history_state += "| RESIZE | "; break;
        }

        history_state += FormatString("%5zd | ", s.info.size());

        if (s.action == Level_History_Action::FLIP_LEVEL_H || s.action == Level_History_Action::FLIP_LEVEL_V)
        {
            for (const auto& tile_layer: s.tile_layer_active)
            {
                history_state += (tile_layer) ? "X" : ".";
            }
        }
        else
        {
            history_state += ". ";
            for (const auto& tile_layer: tab.tile_layer_active)
            {
                history_state += ".";
            }
        }

        history_state += " |";
        LogDebug("%s", history_state.c_str());
    }
    EndDebugSection();
}

TEINAPI void internal__resize (ResizeDir dir, int nw, int nh)
{
    Tab& tab = get_current_tab();

    int lw = tab.level.header.width;
    int lh = tab.level.header.height;

    int dx = nw - lw;
    int dy = nh - lh;

    if (dx == 0 && dy == 0) return;

    LevelData old_data = tab.level.data;
    for (auto& layer: tab.level.data)
    {
        layer.clear();
        layer.resize(nw*nh, 0);
    }

    int lvlw = lw;
    int lvlh = lh;
    int lvlx = (nw-lvlw) / 2;
    int lvly = (nh-lvlh) / 2;

    int offx = 0;
    int offy = 0;

    // Determine the content offset needed if shrinking the level down.
    if (dx < 0)
    {
        if      (ResizeDirIsWest (dir)) lvlw -= abs(dx);
        else if (ResizeDirIsEast (dir)) lvlw -= abs(dx), offx += abs(dx);
        else                               lvlw -= abs(dx), offx += abs(dx) / 2;
    }
    if (dy < 0)
    {
        if      (ResizeDirIsNorth(dir)) lvlh -= abs(dy);
        else if (ResizeDirIsSouth(dir)) lvlh -= abs(dy), offy += abs(dy);
        else                               lvlh -= abs(dy), offy += abs(dy) / 2;
    }

    // Determine the horizontal position of the level content.
    if      (ResizeDirIsWest (dir)) lvlx = 0;
    else if (ResizeDirIsEast (dir)) lvlx = nw - lvlw;
    // Determine the vertical position of the level content.
    if      (ResizeDirIsNorth(dir)) lvly = 0;
    else if (ResizeDirIsSouth(dir)) lvly = nh - lvlh;

    // Make sure not out of bounds!
    if (lvlx < 0) lvlx = 0;
    if (lvly < 0) lvly = 0;

    for (int i=0; i<LEVEL_LAYER_TOTAL; ++i)
    {
        auto& new_layer = tab.level.data.at(i);
        auto& old_layer = old_data.at(i);

        for (int iy=0; iy<lvlh; ++iy)
        {
            for (int ix=0; ix<lvlw; ++ix)
            {
                int npos = (iy+lvly) * nw + (ix+lvlx);
                int opos = (iy+offy) * lw + (ix+offx);

                new_layer.at(npos) = old_layer.at(opos);
            }
        }
    }

    tab.level.header.width  = nw;
    tab.level.header.height = nh;

    level_has_unsaved_changes();
}

/* -------------------------------------------------------------------------- */

TEINAPI void init_level_editor ()
{
    level_editor.tool_state = Tool_State::IDLE;
    level_editor.tool_type  = Tool_Type::BRUSH;

    level_editor.mouse_world = Vec2(0,0);
    level_editor.mouse       = Vec2(0,0);
    level_editor.mouse_tile  = Vec2(0,0);

    level_editor.bounds_visible     = true;
    level_editor.layer_transparency = true;

    level_editor.mirror_h = false;
    level_editor.mirror_v = false;

    level_editor.bounds   = { 0, 0, 0, 0 };
    level_editor.viewport = { 0, 0, 0, 0 };
}

TEINAPI void do_level_editor ()
{
    Quad p1;

    p1.x = GetToolbarWidth() + 1;
    p1.y = gTabBarHeight  + 1;
    p1.w = GetViewport().w - GetToolbarWidth() - (GetControlPanelWidth()) - 2;
    p1.h = GetViewport().h - STATUS_BAR_HEIGHT - gTabBarHeight - 2;

    BeginPanel(p1.x, p1.y, p1.w, p1.h, UI_NONE);

    // We cache the mouse position so that systems such as paste which can
    // potentially happen outside of this section of code (where the needed
    // transforms will be applied) can use the mouse position reliably as
    // prior to doing this there were bugs with the cursor's position being
    // slightly off during those operations + it's probably a bit faster.
    push_editor_camera_transform();
    level_editor.mouse_world = ScreenToWorld(GetMousePos());
    level_editor.mouse = GetMousePos();
    level_editor.mouse_tile = internal__mouse_to_tile_position();
    pop_editor_camera_transform();

    // We cache this just in case anyone else wants to use it (status bar).
    level_editor.viewport = GetViewport();

    const Tab& tab = get_current_tab();

    // If we're in the level editor viewport then the cursor can be one of
    // the custom tool cursors based on what our current tool currently is.
    if (mouse_inside_level_editor_viewport() && IsWindowFocused("WINMAIN"))
    {
        switch (level_editor.tool_type)
        {
            case (Tool_Type::BRUSH ): SetCursorType(Cursor::BRUSH);  break;
            case (Tool_Type::FILL  ): SetCursorType(Cursor::FILL);   break;
            case (Tool_Type::SELECT): SetCursorType(Cursor::SELECT); break;
        }
    }
    else
    {
        // We do this check so it doesn't mess with text box and hyperlink UI cursors!
        if (GetCursorType() != Cursor::BEAM && GetCursorType() != Cursor::POINTER)
        {
            SetCursorType(Cursor::ARROW);
        }
    }

    push_editor_camera_transform();

    // The boundaries of the actual level content (tiles/spawns).
    level_editor.bounds.w = tab.level.header.width  * DEFAULT_TILE_SIZE;
    level_editor.bounds.h = tab.level.header.height * DEFAULT_TILE_SIZE;
    level_editor.bounds.x = (GetViewport().w - level_editor.bounds.w) / 2;
    level_editor.bounds.y = (GetViewport().h - level_editor.bounds.h) / 2;

    float x = level_editor.bounds.x;
    float y = level_editor.bounds.y;
    float w = level_editor.bounds.w;
    float h = level_editor.bounds.h;

    float tile_scale = DEFAULT_TILE_SIZE / gTileImageSize;
    SetTextureDrawScale(tile_scale, tile_scale);

    // We cache the transformed level editor bounds in screen coordinates so
    // that we can later scissor the area to avoid any tile/spawn overspill.
    Vec2 le_bounds_a = WorldToScreen(Vec2(x  , y  ));
    Vec2 le_bounds_b = WorldToScreen(Vec2(x+w, y+h));

    // Because we mess with the orthographic projection matrix a pixel is no
    // longer 1.0f so we need to adjust by the current zoom to get a pixel.
    //
    // We do this because otherwise the outer border gets scaled incorrectly
    // and looks quite ugly. This method ensures it always remains 1px thick.
    float px = (1 / tab.camera.zoom);

    SetDrawColor(gUiColorBlack);
    FillQuad(x-px, y-px, x+w+px, y+h+px);
    SetDrawColor(gEditorSettings.backgroundColor);
    FillQuad(x, y, x+w, y+h);

    // Determine the currently selected layer so that we can make all of the
    // layers above semi-transparent. If we're the spawn layer (top) then it
    // we don't really have a layer so we just assign to minus one to mark.
    LevelLayer selected_layer = GetSelectedLayer();

    constexpr float SEMI_TRANS = .6f;

    // Scissor the content of the level editor region to avoid any overspill.
    float scx = floorf(le_bounds_a.x);
    float scy = floorf(le_bounds_a.y);
    float scw = ceilf (le_bounds_b.x - scx);
    float sch = ceilf (le_bounds_b.y - scy);

    BeginScissor(scx, scy, scw, sch);

    TextureAtlas& atlas = GetEditorAtlasLarge();
    SetTileBatchTexture(atlas.texture);

    // Draw all of the tiles for the level, layer-by-layer.
    for (int i=LEVEL_LAYER_BACK2; (i<=LEVEL_LAYER_BACK2)&&(i>=LEVEL_LAYER_TAG); --i)
    {
        // If the layer is not active then we do not bother drawing its content.
        if (!tab.tile_layer_active[i]) continue;

        if (level_editor.layer_transparency && ((selected_layer != LEVEL_LAYER_TAG) && (static_cast<int>(selected_layer) > i)))
        {
            SetTileBatchColor(Vec4(1,1,1,SEMI_TRANS));
        }
        else
        {
            SetTileBatchColor(Vec4(1,1,1,1));
        }

        // We draw from the top-to-bottom, left-to-right, so that elements
        // in the level editor stack up nicely on top of each other.
        float ty = y+DEFAULT_TILE_SIZE_HALF;
        float tx = x+DEFAULT_TILE_SIZE_HALF;

        const auto& layer = tab.level.data[i];
        for (int j=0; j<static_cast<int>(layer.size()); ++j)
        {
            if (layer[j] != 0) // No point drawing empty tiles...
            {
                DrawBatchedTile(tx, ty, &internal__get_tile_graphic_clip(atlas, layer[j]));
            }

            // Move to the next tile in the row, move down if needed.
            tx += DEFAULT_TILE_SIZE;
            if (tx >= (x + w))
            {
                ty += DEFAULT_TILE_SIZE;
                tx = x+DEFAULT_TILE_SIZE_HALF;
            }
        }
    }

    FlushBatchedTiles();

    // Draw either a ghosted version of the currently selected tile or what is
    // currently in the clipboard. What we draw depends on if the key modifier
    // for pasting is currently being pressed or not (by default this is CTRL).
    if (!are_all_layers_inactive())
    {
        if (!IsAWindowResizing() && mouse_inside_level_editor_viewport())
        {
            if (!internal__clipboard_empty() && IsKeyModStateActive(GetKeyBinding(gKbPaste).mod))
            {
                internal__draw_mirrored_clipboard();
            }
            else
            {
                internal__draw_mirrored_cursor();
            }
        }
    }

    EndScissor();

    // Draw the greyed out area outside of the level's camera bounds.
    if (level_editor.bounds_visible)
    {
        // It seems, from testing, that the game uses the camera tiles to calculate the visible
        // area of a level by taking the most extreme camera tile positions and uses those to
        // create a bounding box of the top, left, right, and down-most camera tile placements.
        //
        // So in order to get the most accurate camera bounding box for the editor we too must
        // obtain these values by searching through the level data, and then rendering this.

        int lw = tab.level.header.width;
        int lh = tab.level.header.height;

        auto& tag_layer = tab.level.data[LEVEL_LAYER_TAG];

        int cl = lw-1;
        int ct = lh-1;
        int cr = 0;
        int cb = 0;

        int camera_tile_count = 0;

        for (int iy=0; iy<lh; ++iy)
        {
            for (int ix=0; ix<lw; ++ix)
            {
                TileID id = tag_layer[iy * tab.level.header.width + ix];
                if (id == CAMERA_ID)
                {
                    ++camera_tile_count;

                    cl = std::min(cl, ix);
                    ct = std::min(ct, iy);
                    cr = std::max(cr, ix);
                    cb = std::max(cb, iy);
                }
            }
        }

        // If we have a camera tile selected we can also use that to showcase how it will impact the bounds.
        if (level_editor.tool_type != Tool_Type::SELECT)
        {
            if (GetSelectedTile() == CAMERA_ID)
            {
                if (mouse_inside_level_editor_viewport())
                {
                    Vec2 tile = internal__mouse_to_tile_position();
                    ++camera_tile_count;

                    cl = std::min(cl, static_cast<int>(tile.x));
                    ct = std::min(ct, static_cast<int>(tile.y));
                    cr = std::max(cr, static_cast<int>(tile.x));
                    cb = std::max(cb, static_cast<int>(tile.y));
                }
            }
        }

        // If there is just one then there is no bounds.
        if (camera_tile_count == 1)
        {
            cl = lw-1;
            ct = lh-1;
            cr = 0;
            cb = 0;
        }

        float cx1 = x + (static_cast<float>(std::min(cl, cr)    ) * DEFAULT_TILE_SIZE);
        float cy1 = y + (static_cast<float>(std::min(ct, cb)    ) * DEFAULT_TILE_SIZE);
        float cx2 = x + (static_cast<float>(std::max(cl, cr) + 1) * DEFAULT_TILE_SIZE);
        float cy2 = y + (static_cast<float>(std::max(ct, cb) + 1) * DEFAULT_TILE_SIZE);

        BeginStencil();

        StencilModeErase();
        SetDrawColor(Vec4(1,1,1,1));
        FillQuad(cx1, cy1, cx2, cy2);

        StencilModeDraw();
        SetDrawColor(gEditorSettings.outOfBoundsColor);
        FillQuad(x, y, x+w, y+h);

        EndStencil();
    }

    // Draw the selection box(es) if visible.
    BeginStencil();
    for (auto& bounds: tab.tool_info.select.bounds)
    {
        if (bounds.visible)
        {
            int il, it, ir, ib;
            get_ordered_select_bounds(bounds, &il, &it, &ir, &ib);

            float l =       static_cast<float>(il);
            float r = ceilf(static_cast<float>(ir)+.5f);
            float b =       static_cast<float>(ib);
            float t = ceilf(static_cast<float>(it)+.5f);

            float sx1 = x   + (l     * DEFAULT_TILE_SIZE);
            float sy1 = y   + (b     * DEFAULT_TILE_SIZE);
            float sx2 = sx1 + ((r-l) * DEFAULT_TILE_SIZE);
            float sy2 = sy1 + ((t-b) * DEFAULT_TILE_SIZE);

            StencilModeDraw();
            SetDrawColor(gEditorSettings.selectColor);
            FillQuad(sx1, sy1, sx2, sy2);

            StencilModeErase();
            SetDrawColor(1,1,1,1);
            FillQuad(sx1, sy1, sx2, sy2);
        }
    }
    EndStencil();

    // Draw the tile/spawn grid for the level editor.
    if (editor.grid_visible)
    {
        BeginDraw(BufferMode::LINES);
        for (float ix=x+DEFAULT_TILE_SIZE; ix<(x+w); ix+=DEFAULT_TILE_SIZE)
        {
            PutVertex(ix, y,   Vec4(gEditorSettings.tileGridColor));
            PutVertex(ix, y+h, Vec4(gEditorSettings.tileGridColor));
        }
        for (float iy=y+DEFAULT_TILE_SIZE; iy<(y+h); iy+=DEFAULT_TILE_SIZE)
        {
            PutVertex(x,   iy, Vec4(gEditorSettings.tileGridColor));
            PutVertex(x+w, iy, Vec4(gEditorSettings.tileGridColor));
        }
        EndDraw();
    }

    // Draw the large entity guides if they are enabled.
    if (level_editor.large_tiles && level_editor.entity_guides)
    {
        if (tab.tile_layer_active[LEVEL_LAYER_ACTIVE])
        {
            BeginScissor(scx, scy, scw, sch);

            Vec4 color = gEditorSettings.cursorColor;
            SetLineWidth(2);

            const float LINE_WIDTH = (DEFAULT_TILE_SIZE / 3) * 2; // 2/3
            const float OFFSET = roundf(LINE_WIDTH / 2);

            float ty = y+DEFAULT_TILE_SIZE_HALF;
            float tx = x+DEFAULT_TILE_SIZE_HALF;

            auto& layer = tab.level.data[LEVEL_LAYER_ACTIVE];
            for (int i=0; i<static_cast<int>(layer.size()); ++i)
            {
                // Ensures that the tile is an Entity and not a Basic.
                TileID id = layer[i];
                if ((id != 0) && ((id-40000) >= 0))
                {
                    Quad& b = internal__get_tile_graphic_clip(atlas, id);

                    float hw = (b.w * tile_scale) / 2;
                    float hh = (b.h * tile_scale) / 2;

                    color.a = .20f;
                    SetDrawColor(color);

                    FillQuad(tx-hw, ty-hh, tx+hw, ty+hh);

                    color.a = .85f;
                    SetDrawColor(color);

                    DrawLine(tx-OFFSET, ty, tx+OFFSET, ty);
                    DrawLine(tx, ty-OFFSET, tx, ty+OFFSET);

                    DrawQuad(tx-hw, ty-hh, tx+hw, ty+hh);
                }

                // Move to the next tile in the row, move down if needed.
                tx += DEFAULT_TILE_SIZE;
                if (tx >= (x + w))
                {
                    ty += DEFAULT_TILE_SIZE;
                    tx = x+DEFAULT_TILE_SIZE_HALF;
                }
            }

            EndScissor();
        }
    }

    // Draw the mirroring lines for the level editor.
    SetDrawColor(gEditorSettings.mirrorLineColor);
    SetLineWidth(std::max(1.0f, 3.0f/px));
    if (level_editor.mirror_h)
    {
        float hw = w/2;
        DrawLine(x+hw, y, x+hw, y+h);
    }
    if (level_editor.mirror_v)
    {
        float hh = h/2;
        DrawLine(x, y+hh, x+w, y+hh);
    }
    SetLineWidth(1);

    // Draw the black outline surrounding the level editor content.
    //
    // We do it this way because due to some rounding issues, the scissored
    // content of the level editor sometimes bleeds out of the level editor
    // area by a single pixel and would overlap the black border. By using
    // this stencil method and drawing the black border at the end we stop
    // that issue from occurring -- creating a nicer looking editor border.
    BeginStencil();

    StencilModeErase();
    SetDrawColor(Vec4(1,1,1,1));
    FillQuad(x, y, x+w, y+h);

    StencilModeDraw();
    SetDrawColor(gUiColorBlack);
    FillQuad(x-px, y-px, x+w+px, y+h+px);

    EndStencil();

    pop_editor_camera_transform();

    EndPanel();

    SetTextureDrawScale(1,1);
}

/* -------------------------------------------------------------------------- */

TEINAPI void handle_level_editor_events ()
{
    Tab* tab = &get_current_tab();

    // We don't want to handle any of these events if we don't have focus.
    // We set the tool state to idle here so that if the user was doing
    // an action and then opened another window with a hotkey, when they
    // return the action will not continue due to the tool being active.
    if (!IsWindowFocused("WINMAIN"))
    {
        level_editor.tool_state = Tool_State::IDLE;
        return;
    }

    switch (main_event.type)
    {
        case (SDL_MOUSEBUTTONDOWN):
        case (SDL_MOUSEBUTTONUP):
        {
            // Do not handle these events whilst we are cooling down!
            if (editor.dialog_box) return;

            bool pressed = (main_event.button.state == SDL_PRESSED);
            if (pressed && IsThereAHitUiElement()) return;

            switch (main_event.button.button)
            {
                case (SDL_BUTTON_LEFT):
                {
                    if (pressed)
                    {
                        level_editor.tool_state = Tool_State::PLACE;

                        // This will be the start of a new selection!
                        if (level_editor.tool_type == Tool_Type::SELECT)
                        {
                            tab->tool_info.select.start = true;
                            tab->tool_info.select.cached_size = tab->tool_info.select.bounds.size();
                        }
                        if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
                        {
                            new_level_history_state(Level_History_Action::NORMAL);
                        }

                        // Handle the current tool immediately so that placing/erasing
                        // doesn't require the user to move the mouse for it to work.
                        internal__handle_current_tool();
                    }
                    else
                    {
                        level_editor.tool_state = Tool_State::IDLE;
                        if (level_editor.tool_type == Tool_Type::SELECT)
                        {
                            if (tab->tool_info.select.bounds.size() > tab->tool_info.select.cached_size)
                            {
                                new_level_history_state(Level_History_Action::SELECT_STATE);
                                tab->tool_info.select.cached_size = tab->tool_info.select.bounds.size();
                            }
                        }
                    }
                } break;
                case (SDL_BUTTON_RIGHT):
                {
                    if (pressed)
                    {
                        level_editor.tool_state = Tool_State::ERASE;

                        if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
                        {
                            new_level_history_state(Level_History_Action::NORMAL);
                        }

                        // Handle the current tool immediately so that placing/erasing
                        // doesn't require the user to move the mouse for it to work.
                        internal__handle_current_tool();
                    }
                    else
                    {
                        level_editor.tool_state = Tool_State::IDLE;
                    }
                } break;
            }
        } break;
        case (SDL_MOUSEMOTION):
        {
            // We only want drag painting to happen if the entity allows it.
            // However, this is ignored if the tool is the selection box.
            if (level_editor.tool_state != Tool_State::IDLE)
            {
                internal__handle_current_tool();
            }
        } break;
        case (SDL_KEYDOWN):
        case (SDL_KEYUP):
        {
            bool pressed = main_event.key.state;
            switch (main_event.key.keysym.sym)
            {
                // Handle toggling the select box addition mode using the CTRL key.
                case (SDLK_RCTRL):
                case (SDLK_LCTRL):
                {
                    tab->tool_info.select.add = pressed;
                } break;
            }
        } break;
    }

    // We can dump the history on command in debug mode.
    #if defined(BUILD_DEBUG)
    if (main_event.type == SDL_KEYDOWN)
    {
        if (main_event.key.keysym.sym == SDLK_F12)
        {
            internal__dump_level_history();
        }
    }
    #endif // BUILD_DEBUG
}

/* -------------------------------------------------------------------------- */

TEINAPI bool mouse_inside_level_editor_viewport ()
{
    Vec2 m = level_editor.mouse;
    Quad v = level_editor.viewport;

    // We do this check for the disabling of cursor drawing during a resize.
    // As once the resize is done normally this function would end up being
    // true and would then draw the cursor at the wrong place after resize
    // so this check prevents that from happening and looks visually better.
    if (!SDL_GetMouseFocus()) return false;

    return ((m.x>=v.x) && (m.y>=v.y) && (m.x<=(v.x+v.w)) && (m.y<=(v.y+v.h)));
}

/* -------------------------------------------------------------------------- */

TEINAPI void new_level_history_state (Level_History_Action action)
{
    if (action == Level_History_Action::NORMAL && !mouse_inside_level_editor_viewport()) return;

    Tab& tab = get_current_tab();

    // Don't bother creating a new state if the current erase/place action is
    // empty otherwise we will end up with a bunch of empty states in the list.
    if (tab.level_history.current_position > -1)
    {
        Level_History_State& current = internal__get_current_history_state();
        if (current.info.empty() && current.action == action && action == Level_History_Action::NORMAL)
        {
            return;
        }
    }

    // Clear all the history after the current position, if there is any, as it
    // will no longer apply to the timeline of level editor actions anymore.
    int delete_position = tab.level_history.current_position+1;
    if (delete_position < static_cast<int>(tab.level_history.state.size()))
    {
        auto begin = tab.level_history.state.begin();
        auto end = tab.level_history.state.end();

        tab.level_history.state.erase(begin+delete_position, end);
    }

    // If it's a selection action then we don't need to modify this.
    if (action != Level_History_Action::SELECT_STATE)
    {
        get_current_tab().unsaved_changes = true;
    }

    tab.level_history.state.push_back(Level_History_State());
    tab.level_history.state.back().action = action;

    // Also deal with the layer states for flip actions.
    if (action == Level_History_Action::FLIP_LEVEL_H || action == Level_History_Action::FLIP_LEVEL_V)
    {
        for (int i=LEVEL_LAYER_TAG; i<LEVEL_LAYER_TOTAL; ++i)
        {
            tab.level_history.state.back().tile_layer_active[i] = tab.tile_layer_active[i];
        }
    }

    // Also deal with the select bounds for selection actions.
    if (action == Level_History_Action::SELECT_STATE)
    {
        tab.level_history.state.back().old_select_state = tab.old_select_state;
        tab.level_history.state.back().new_select_state = tab.tool_info.select.bounds;
    }

    // Also deal with width and height for resizing.
    if (action == Level_History_Action::RESIZE)
    {
        tab.level_history.state.back().resize_dir = GetResizeDir();
        tab.level_history.state.back().old_width  = tab.level.header.width;
        tab.level_history.state.back().old_height = tab.level.header.height;
        tab.level_history.state.back().new_width  = GetResizeWidth();
        tab.level_history.state.back().new_height = GetResizeHeight();
    }

    ++tab.level_history.current_position;
}

/* -------------------------------------------------------------------------- */

TEINAPI void add_to_history_normal_state (Level_History_Info info)
{
    if (!mouse_inside_level_editor_viewport()) return;

    Tab& tab = get_current_tab();

    // If there is no current action then we create one. This resolved some
    // potential bugs that can occur when undoing/redoing mid stroke, etc.
    if (tab.level_history.current_position <= -1)
    {
        new_level_history_state(Level_History_Action::NORMAL);
    }

    // We also check if the current state is not of type normal because if
    // it is then we need to add a new normal state (because flip states
    // do not ever need to call this function). This resolves the issues of
    // the history getting messed up if the user flips the level mid-stroke.
    if (internal__get_current_history_state().action != Level_History_Action::NORMAL)
    {
        new_level_history_state(Level_History_Action::NORMAL);
    }

    // Don't add the same spawns/tiles repeatedly, otherwise add the spawn/tile.
    Level_History_State& state = internal__get_current_history_state();
    if (!state.info.empty())
    {
        const Level_History_Info& n = info;
        for (auto& o: state.info)
        {
            if (o.x == n.x && o.y == n.y && o.tile_layer == n.tile_layer && o.new_id != n.new_id)
            {
                o.new_id = n.new_id;
            }
            if (o.x == n.x && o.y == n.y && o.tile_layer == n.tile_layer && o.new_id == n.new_id)
            {
                return;
            }
        }
    }

    state.info.push_back(info);
}

TEINAPI void add_to_history_clear_state (Level_History_Info info)
{
    assert(internal__get_current_history_state().action == Level_History_Action::CLEAR);
    internal__get_current_history_state().info.push_back(info);
}

/* -------------------------------------------------------------------------- */

TEINAPI bool are_all_layers_inactive ()
{
    const Tab& tab = get_current_tab();
    for (auto tile_layer_active: tab.tile_layer_active)
    {
        if (tile_layer_active) return false;
    }
    return true;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool are_any_select_boxes_visible ()
{
    if (!are_there_any_tabs()) return false;

    const Tab& tab = get_current_tab();
    for (auto& bounds: tab.tool_info.select.bounds)
    {
        if (bounds.visible) return true;
    }
    return false;
}

TEINAPI void get_ordered_select_bounds (const Select_Bounds& bounds, int* l, int* t, int* r, int* b)
{
    // We do this here rather than ordering it in the actual handle
    // select function because otherwise it would cause some issues.

    if (l) *l = std::min(bounds.left, bounds.right );
    if (r) *r = std::max(bounds.left, bounds.right );
    if (b) *b = std::min(bounds.top,  bounds.bottom);
    if (t) *t = std::max(bounds.top,  bounds.bottom);
}

TEINAPI void get_total_select_boundary (int* l, int* t, int* r, int* b)
{
    if (l) *l = 0;
    if (t) *t = 0;
    if (r) *r = 0;
    if (b) *b = 0;

    if (!are_there_any_tabs() || !are_any_select_boxes_visible()) return;

    const Tab& tab = get_current_tab();

    int min_l = INT_MAX;
    int max_t = 0;
    int max_r = 0;
    int min_b = INT_MAX;

    for (auto& bounds: tab.tool_info.select.bounds)
    {
        if (bounds.visible)
        {
            int sl,st,sr,sb;
            get_ordered_select_bounds(bounds, &sl,&st,&sr,&sb);

            min_l = std::min(min_l, sl);
            max_t = std::max(max_t, st);
            max_r = std::max(max_r, sr);
            min_b = std::min(min_b, sb);
        }
    }

    if (l) *l = min_l;
    if (t) *t = max_t;
    if (r) *r = max_r;
    if (b) *b = min_b;
}

/* -------------------------------------------------------------------------- */

TEINAPI void load_level_tab (std::string file_name)
{
    // If there is just one tab and it is completely empty with no changes
    // then we close this tab before opening the new level(s) in editor.
    if (editor.tabs.size() == 1)
    {
        if (is_current_tab_empty() && !get_current_tab().unsaved_changes && get_current_tab().name.empty())
        {
            close_current_tab();
        }
    }

    size_t tab_index = get_tab_index_with_this_file_name(file_name);
    if (tab_index != INVALID_TAB) // This file is already open so just focus on it.
    {
        set_current_tab(tab_index);
    }
    else
    {
        create_new_level_tab_and_focus();
        Tab& tab = get_current_tab();
        tab.name = file_name;
        set_main_window_subtitle_for_tab(tab.name);

        if (!LoadLevel(tab.level, tab.name))
        {
            close_current_tab();
        }
    }

    NeedToScrollNextUpdate();
}

/* -------------------------------------------------------------------------- */

TEINAPI bool le_save (Tab& tab)
{
    // If the current file already has a name (has been saved before) then we
    // just do a normal Save to that file. Otherwise, we perform a Save As.
    if (tab.name.empty())
    {
        std::string file_name = SaveDialog(DialogType::LVL);
        if (file_name.empty()) return false;
        tab.name = file_name;
    }

    SaveLevel(tab.level, tab.name);
    backup_level_tab(tab.level, tab.name);

    tab.unsaved_changes = false;
    set_main_window_subtitle_for_tab(tab.name);

    return true;
}

TEINAPI bool le_save_as ()
{
    std::string file_name = SaveDialog(DialogType::LVL);
    if (file_name.empty()) return false;

    Tab& tab = get_current_tab();

    tab.name = file_name;
    SaveLevel(tab.level, tab.name);
    backup_level_tab(tab.level, tab.name);

    tab.unsaved_changes = false;
    set_main_window_subtitle_for_tab(tab.name);

    return true;
}

TEINAPI void le_clear_select ()
{
    if (!current_tab_is_level() || !are_any_select_boxes_visible()) return;

    Tab& tab = get_current_tab();

    new_level_history_state(Level_History_Action::CLEAR);
    for (auto& bounds: tab.tool_info.select.bounds)
    {
        if (bounds.visible)
        {
            int l, t, r, b;
            get_ordered_select_bounds(bounds, &l, &t, &r, &b);

            // Clear all of the tiles within the selection.
            for (int i=LEVEL_LAYER_TAG; i<LEVEL_LAYER_TOTAL; ++i)
            {
                for (int y=b; y<=t; ++y)
                {
                    for (int x=l; x<=r; ++x)
                    {
                        internal__place_mirrored_tile_clear(x, y, 0, static_cast<LevelLayer>(i));
                    }
                }
            }
        }
    }

    // We also deselect the select box(es) afterwards -- feels right.
    tab.level_history.state.back().old_select_state = tab.tool_info.select.bounds;
    internal__deselect();
    tab.level_history.state.back().new_select_state = tab.tool_info.select.bounds;

    get_current_tab().unsaved_changes = true;
}

TEINAPI void le_deselect ()
{
    if (!current_tab_is_level()) return;

    Tab& tab = get_current_tab();
    tab.old_select_state = tab.tool_info.select.bounds;

    internal__deselect();

    // Add this deselection to the history.
    new_level_history_state(Level_History_Action::SELECT_STATE);
}

TEINAPI void le_select_all ()
{
    if (!current_tab_is_level()) return;

    Tab& tab = get_current_tab();

    tab.old_select_state = tab.tool_info.select.bounds;

    tab.tool_info.select.bounds.clear();
    tab.tool_info.select.bounds.push_back(Select_Bounds());

    tab.tool_info.select.bounds.back().left    = 0;
    tab.tool_info.select.bounds.back().top     = 0;
    tab.tool_info.select.bounds.back().right   = tab.level.header.width-1;
    tab.tool_info.select.bounds.back().bottom  = tab.level.header.height-1;
    tab.tool_info.select.bounds.back().visible = true;

    // Add this selection to the history.
    new_level_history_state(Level_History_Action::SELECT_STATE);
}

TEINAPI void le_copy ()
{
    if (!current_tab_is_level() || !are_any_select_boxes_visible()) return;
    internal__copy();
    le_deselect(); // We also deselect the region afterwards, feels right.
}

TEINAPI void le_cut ()
{
    if (!current_tab_is_level() || !are_any_select_boxes_visible()) return;
    internal__copy();
    le_clear_select(); // Does deselect for us.
    get_current_tab().unsaved_changes = true;
}

TEINAPI void le_paste ()
{
    if (!current_tab_is_level() || internal__clipboard_empty()) return;

    Vec2 tile_pos = level_editor.mouse_tile;
    new_level_history_state(Level_History_Action::NORMAL);

    for (auto& clipboard: level_editor.clipboard)
    {
        int x = static_cast<int>(tile_pos.x) + clipboard.x;
        int y = static_cast<int>(tile_pos.y) + clipboard.y;
        int w = clipboard.w;
        int h = clipboard.h;

        // Paste all of the clipboard tiles.
        for (size_t i=0; i<clipboard.data.size(); ++i)
        {
            const auto& src_layer = clipboard.data[i];
            for (int iy=y; iy<(y+h); ++iy)
            {
                for (int ix=x; ix<(x+w); ++ix)
                {
                    internal__place_mirrored_tile(ix, iy, src_layer[(iy-y)*w+(ix-x)], static_cast<LevelLayer>(i));
                }
            }
        }
    }

    get_current_tab().unsaved_changes = true;
}

/* -------------------------------------------------------------------------- */

TEINAPI void flip_level_h ()
{
    // If all layers are inactive then there is no point in doing the flip.
    if (are_all_layers_inactive()) return;

    const Tab& tab = get_current_tab();

    int lw = tab.level.header.width;
    int lh = tab.level.header.height;

    // If the active layers in flip bounds are empty there is no point.
    if (internal__are_active_layers_in_bounds_empty(0, 0, lw, lh)) return;

    new_level_history_state(Level_History_Action::FLIP_LEVEL_H);
    internal__flip_level_h(tab.tile_layer_active);
}

TEINAPI void flip_level_v ()
{
    // If all layers are inactive then there is no point in doing the flip.
    if (are_all_layers_inactive()) return;

    const Tab& tab = get_current_tab();

    int lw = tab.level.header.width;
    int lh = tab.level.header.height;

    // If the active layers in flip bounds are empty there is no point.
    if (internal__are_active_layers_in_bounds_empty(0, 0, lw, lh)) return;

    new_level_history_state(Level_History_Action::FLIP_LEVEL_V);
    internal__flip_level_v(tab.tile_layer_active);
}

/* -------------------------------------------------------------------------- */

TEINAPI void level_has_unsaved_changes ()
{
    get_current_tab().unsaved_changes = true;
}

/* -------------------------------------------------------------------------- */

TEINAPI void le_undo ()
{
    Tab& tab = get_current_tab();

    // There is no history or we are already at the beginning.
    if (tab.level_history.current_position <= -1) return;

    bool normal_state_empty = false;

    Level_History_State& state = internal__get_current_history_state();
    switch (state.action)
    {
        case (Level_History_Action::RESIZE):
        {
            internal__resize(state.resize_dir, state.old_width, state.old_height);
            tab.level.data = state.old_data;
        } break;
        case (Level_History_Action::SELECT_STATE):
        {
            internal__restore_select_state(state.old_select_state);
        } break;
        case (Level_History_Action::FLIP_LEVEL_H):
        {
            internal__flip_level_h(state.tile_layer_active);
        } break;
        case (Level_History_Action::FLIP_LEVEL_V):
        {
            internal__flip_level_v(state.tile_layer_active);
        } break;
        case (Level_History_Action::NORMAL):
        case (Level_History_Action::CLEAR):
        {
            // We check if the normal state we're undoing is empty or not. If it is
            // then we mark it as such and then if there is another state before it
            // we undo that one as well. This just feels a nicer than not doing it.
            if (state.info.empty()) normal_state_empty = true;

            for (auto& i: state.info)
            {
                int pos = i.y * tab.level.header.width + i.x;
                tab.level.data[i.tile_layer][pos] = i.old_id;
            }

            if (state.action == Level_History_Action::CLEAR)
            {
                internal__restore_select_state(state.old_select_state);
            }
        } break;
    }

    if (tab.level_history.current_position > -1)
    {
        --tab.level_history.current_position;
        // We only want to do this part if there is another state to undo.
        if (state.action == Level_History_Action::NORMAL && normal_state_empty)
        {
            le_undo();
        }
    }

    if (state.action != Level_History_Action::SELECT_STATE)
    {
        tab.unsaved_changes = true;
    }
}

TEINAPI void le_redo ()
{
    Tab& tab = get_current_tab();

    // There is no history or we are already at the end.
    if (tab.level_history.current_position >= tab.level_history.state.size()-1) return;

    ++tab.level_history.current_position;

    Level_History_State& state = internal__get_current_history_state();
    switch (state.action)
    {
        case (Level_History_Action::RESIZE):
        {
            internal__resize(state.resize_dir, state.new_width, state.new_height);
            tab.level.data = state.new_data;
        } break;
        case (Level_History_Action::SELECT_STATE):
        {
            internal__restore_select_state(state.new_select_state);
        } break;
        case (Level_History_Action::FLIP_LEVEL_H):
        {
            internal__flip_level_h(state.tile_layer_active);
        } break;
        case (Level_History_Action::FLIP_LEVEL_V):
        {
            internal__flip_level_v(state.tile_layer_active);
        } break;
        case (Level_History_Action::NORMAL):
        case (Level_History_Action::CLEAR):
        {
            for (auto& i: state.info)
            {
                int pos = i.y * tab.level.header.width + i.x;
                tab.level.data[i.tile_layer][pos] = i.new_id;
            }

            if (state.action == Level_History_Action::CLEAR)
            {
                internal__restore_select_state(state.new_select_state);
            }
        } break;
    }

    // If we end on an empty normal state and we are not already at the end of
    // the redo history then we redo again as it feels nicer. This action is
    // the inverse of what we do when we do an undo with blank normal actions.
    if (tab.level_history.current_position+1 < tab.level_history.state.size())
    {
        // Jump forward to see if it is empty, if it's not then revert back.
        ++tab.level_history.current_position;
        Level_History_State& next_state = internal__get_current_history_state();

        if (next_state.action != Level_History_Action::NORMAL || !next_state.info.empty())
        {
            --tab.level_history.current_position;
        }
    }

    if (state.action != Level_History_Action::SELECT_STATE)
    {
        tab.unsaved_changes = true;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void le_history_begin ()
{
    Tab& tab = get_current_tab();
    while (tab.level_history.current_position > -1) le_undo();
    tab.unsaved_changes = true;
}

TEINAPI void le_history_end ()
{
    Tab& tab = get_current_tab();
    int maximum = static_cast<int>(tab.level_history.state.size()-1);
    while (tab.level_history.current_position < maximum) le_redo();
    tab.unsaved_changes = true;
}

/* -------------------------------------------------------------------------- */

TEINAPI void le_resize ()
{
    if (!current_tab_is_level()) return;
    const Tab& tab = get_current_tab();
    OpenResize(tab.level.header.width, tab.level.header.height);
}

TEINAPI void le_resize_okay ()
{
    Tab& tab = get_current_tab();

    int lw = tab.level.header.width;
    int lh = tab.level.header.height;

    int nw = GetResizeWidth();
    int nh = GetResizeHeight();

    int dx = nw - lw;
    int dy = nh - lh;

    // Return early to avoid making a history state for no reason.
    if (dx == 0 && dy == 0) return;

    new_level_history_state(Level_History_Action::RESIZE);
    internal__get_current_history_state().old_data = tab.level.data;
    internal__resize(GetResizeDir(), nw, nh);
    internal__get_current_history_state().new_data = tab.level.data;
}

/* -------------------------------------------------------------------------- */

TEINAPI void le_load_prev_level ()
{
    if (!current_tab_is_level()) return;

    Tab& tab = get_current_tab();

    std::string path(StripFileName(tab.name));

    std::vector<std::string> files;
    ListPathFiles(path, files);
    if (files.size() <= 1) return;

    files.erase(std::remove_if(files.begin(), files.end(),
    [](const std::string& s)
    {
        return s.substr(s.find_last_of(".")) != ".lvl";
    }),
    files.end());

    // We strip the extensions then sort the files as they can interfere with
    // getting a good alphabetical sort on the strings. We add them back later.
    for (auto& f: files) f = StripFileExt(f);
    std::sort(files.begin(), files.end());

    // Find our current location and move on to the prev file.
    std::string current(StripFileExt(tab.name));
    auto iter = std::find(files.begin(), files.end(), current);
    if (iter == files.end()) return; // Shouldn't happen...

    std::string prev;
    if (iter == files.begin()) prev = *(files.end()-1);
    else prev = *(iter-1);

    prev += ".lvl";

    if (save_changes_prompt(tab) == ALERT_RESULT_CANCEL)
    {
        return;
    }
    tab.unsaved_changes = false;

    // Finally, we can load the prev level as the current tab.
    tab.name = prev;
    set_main_window_subtitle_for_tab(tab.name);

    if (!LoadLevel(tab.level, tab.name))
    {
        close_current_tab();
    }
}

TEINAPI void le_load_next_level ()
{
    if (!current_tab_is_level()) return;

    Tab& tab = get_current_tab();

    std::string path(StripFileName(tab.name));

    std::vector<std::string> files;
    ListPathFiles(path, files);
    if (files.size() <= 1) return;

    files.erase(std::remove_if(files.begin(), files.end(),
    [](const std::string& s)
    {
        return s.substr(s.find_last_of(".")) != ".lvl";
    }),
    files.end());

    // We strip the extensions then sort the files as they can interfere with
    // getting a good alphabetical sort on the strings. We add them back later.
    for (auto& f: files)
    {
        std::string ext(f.substr(f.find_last_of(".")));
        f = StripFileExt(f);
    }
    std::sort(files.begin(), files.end());

    // Find our current location and move on to the next file.
    std::string current(StripFileExt(tab.name));
    auto iter = std::find(files.begin(), files.end(), current);
    if (iter == files.end()) return; // Shouldn't happen...

    std::string next;
    if (iter+1 != files.end()) next = *(iter+1);
    else next = *files.begin();

    next += ".lvl";

    if (save_changes_prompt(tab) == ALERT_RESULT_CANCEL)
    {
        return;
    }
    tab.unsaved_changes = false;

    // Finally, we can load the next level as the current tab.
    tab.name = next;
    set_main_window_subtitle_for_tab(tab.name);

    if (!LoadLevel(tab.level, tab.name))
    {
        close_current_tab();
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void level_drop_file (Tab* tab, std::string file_name)
{
    file_name = FixPathSlashes(file_name);

    // If there is just one tab and it is completely empty with no changes
    // then we close this tab before opening the new level(s) in editor.
    if (editor.tabs.size() == 1)
    {
        if (is_current_tab_empty() && !get_current_tab().unsaved_changes && get_current_tab().name.empty())
        {
            close_current_tab();
        }
    }

    size_t tab_index = get_tab_index_with_this_file_name(file_name);
    if (tab_index != INVALID_TAB) // This file is already open so just focus on it.
    {
        set_current_tab(tab_index);
    }
    else
    {
        create_new_level_tab_and_focus();
        tab = &get_current_tab();
        tab->name = file_name;
        set_main_window_subtitle_for_tab(tab->name);

        if (!LoadLevel(tab->level, tab->name))
        {
            close_current_tab();
        }
    }

    NeedToScrollNextUpdate();
}

/* -------------------------------------------------------------------------- */

TEINAPI void backup_level_tab (const Level& level, const std::string& file_name)
{
    // Determine how many backups the user wants saved for a given level.
    int backup_count = gEditorSettings.backupCount;
    if (backup_count <= 0) return; // No backups are wanted!

    std::string level_name((file_name.empty()) ? "untitled" : StripFilePathAndExt(file_name));

    // Create a folder for this particular level's backups if it does not exist.
    // We make separate sub-folders in the backup directory for each level as
    // there was an issue in older versions with the editor freezing when backing
    // up levels to a backups folder with loads of saves. This was because the
    // editor was searching the folder for old backups (leading to a freeze).
    std::string backup_path(MakePathAbsolute("backups/" + level_name + "/"));
    if (!DoesPathExist(backup_path))
    {
        if (!CreatePath(backup_path))
        {
            LogError(ERR_MIN, "Failed to create backup for level \"%s\"!", level_name.c_str());
            return;
        }
    }

    // Determine how many backups are already saved of this level.
    std::vector<std::string> backups;
    ListPathContent(backup_path, backups);

    int level_count = 0;
    for (auto& file: backups)
    {
        if (IsFile(file))
        {
            // We strip extension twice because there are two extension parts to backups the .bak and the .lvl.
            std::string compare_name(StripFileExt(StripFilePathAndExt(file)));
            if (InsensitiveCompare(level_name, compare_name)) ++level_count;
        }
    }

    // If there is still room to create a new backup then that is what
    // we do. Otherwise, we overwrite the oldest backup of the level.
    std::string backup_name = backup_path + level_name + ".bak";
    if (gEditorSettings.unlimitedBackups || (level_count < backup_count))
    {
        backup_name += std::to_string(level_count) + ".lvl";
        SaveLevel(level, backup_name);
    }
    else
    {
        U64 oldest = UINT64_MAX;
        int oldest_index = 0;

        for (int i=0; i<level_count; ++i)
        {
            std::string name(backup_name + std::to_string(i) + ".lvl");
            U64 current = LastFileWriteTime(name);
            if (CompareFileWriteTimes(current, oldest) == -1)
            {
                oldest = current;
                oldest_index = i;
            }
        }

        backup_name += std::to_string(oldest_index) + ".lvl";
        SaveLevel(level, backup_name);
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI bool is_current_level_empty ()
{
    if (are_there_any_level_tabs())
    {
        const Tab& tab = get_current_tab();
        if (tab.type == Tab_Type::LEVEL)
        {
            for (const auto& layer: tab.level.data)
            {
                for (auto id: layer) if (id != 0) return false;
            }
            return true;
        }
    }
    return false;
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
