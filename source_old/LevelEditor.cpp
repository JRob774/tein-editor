static constexpr float gGhostedCursorAlpha = .5f;
static constexpr TileID gCameraID = 20000;

namespace Internal
{
    TEINAPI Quad& GetTileGraphicClip (TextureAtlas& atlas, TileID id)
    {
        if (gLevelEditor.largeTiles && atlas.clips.count(id + gAltOffset)) id += gAltOffset;
        return GetAtlasClip(atlas, id);
    }

    TEINAPI bool AreActiveLayersInBoundsEmpty (int x, int y, int w, int h)
    {
        const Tab& tab = GetCurrentTab();
        for (size_t i=0; i<tab.level.data.size(); ++i) {
            if (tab.tileLayerActive[i]) {
                const auto& tileLayer = tab.level.data.at(i);
                for (int iy=y; iy<(y+h); ++iy) {
                    for (int ix=x; ix<(x+w); ++ix) {
                        TileID id = tileLayer.at(iy*tab.level.header.width+ix);
                        if (id != 0) return false;
                    }
                }
            }
        }
        return true;
    }

    TEINAPI LevelHistoryState& GetCurrentHistoryState ()
    {
        Tab& tab = GetCurrentTab();
        return tab.levelHistory.state[tab.levelHistory.currentPosition];
    }

    TEINAPI bool TileInBounds (int x, int y)
    {
        const Tab& tab = GetCurrentTab();

        int w = tab.level.header.width;
        int h = tab.level.header.height;

        return (x >= 0 && x < w && y >= 0 && y < h);
    }

    TEINAPI void PlaceTileClear (int x, int y, TileID id, LevelLayer tileLayer)
    {
        Tab& tab = GetCurrentTab();

        if (!tab.tileLayerActive[static_cast<int>(tileLayer)]) return;
        if (!TileInBounds(x,y)) return;

        auto& layer = tab.level.data[static_cast<int>(tileLayer)];

        LevelHistoryInfo info = {};
        info.x = x;
        info.y = y;
        info.oldID = layer[y * tab.level.header.width + x];
        info.newID = id;
        info.tileLayer = tileLayer;
        AddToHistoryClearState(info);

        layer[y * tab.level.header.width + x] = id;

        GetCurrentTab().unsavedChanges = true;
    }

    TEINAPI void PlaceTile (int x, int y, TileID id, LevelLayer tileLayer)
    {
        Tab& tab = GetCurrentTab();

        if (!tab.tileLayerActive[static_cast<int>(tileLayer)]) return;
        if (!TileInBounds(x,y)) return;

        auto& layer = tab.level.data[static_cast<int>(tileLayer)];

        LevelHistoryInfo info = {};
        info.x = x;
        info.y = y;
        info.oldID = layer[y * tab.level.header.width + x];
        info.newID = id;
        info.tileLayer = tileLayer;
        AddToHistoryNormalState(info);

        layer[y * tab.level.header.width + x] = id;

        GetCurrentTab().unsavedChanges = true;
    }

    TEINAPI void PlaceMirroredTileClear (int x, int y, TileID id, LevelLayer tileLayer)
    {
        bool both = (gLevelEditor.mirrorH && gLevelEditor.mirrorV);

        const Tab& tab = GetCurrentTab();

        int lw = tab.level.header.width-1;
        int lh = tab.level.header.height-1;

                                  PlaceTileClear(   x,    y,                                           id  , tileLayer);
        if (gLevelEditor.mirrorH) PlaceTileClear(lw-x,    y, GetTileHorizontalFlip                    (id) , tileLayer);
        if (gLevelEditor.mirrorV) PlaceTileClear(   x, lh-y,                       GetTileVerticalFlip(id) , tileLayer);
        if (both)                 PlaceTileClear(lw-x, lh-y, GetTileHorizontalFlip(GetTileVerticalFlip(id)), tileLayer);
    }

    TEINAPI void PlaceMirroredTile (int x, int y, TileID id, LevelLayer tileLayer)
    {
        bool both = (gLevelEditor.mirrorH && gLevelEditor.mirrorV);

        const Tab& tab = GetCurrentTab();

        int lw = tab.level.header.width-1;
        int lh = tab.level.header.height-1;

                                  PlaceTile(   x,    y,                                           id  , tileLayer);
        if (gLevelEditor.mirrorH) PlaceTile(lw-x,    y, GetTileHorizontalFlip                    (id) , tileLayer);
        if (gLevelEditor.mirrorV) PlaceTile(   x, lh-y,                       GetTileVerticalFlip(id) , tileLayer);
        if (both)                 PlaceTile(lw-x, lh-y, GetTileHorizontalFlip(GetTileVerticalFlip(id)), tileLayer);
    }

    TEINAPI bool ClipboardEmpty ()
    {
        for (auto& clipboard: gLevelEditor.clipboard) {
            for (auto& layer: clipboard.data) {
                if (!layer.empty()) return false;
            }
        }
        return true;
    }

    TEINAPI void Copy ()
    {
        Tab& tab = GetCurrentTab();

        if (AreAnySelectBoxesVisible()) {
            // Clear the old clipboard content now we know we can actually copy.
            gLevelEditor.clipboard.clear();

            int sl = 0;
            int st = 0;
            int sr = 0;
            int sb = 0;

            GetTotalSelectBoundary(&sl,&st,&sr,&sb);

            for (auto& bounds: tab.toolInfo.select.bounds) {
                if (bounds.visible) {
                    gLevelEditor.clipboard.push_back(LevelClipboard());
                    LevelClipboard& clipboard = gLevelEditor.clipboard.back();

                    int l,t,r,b;
                    GetOrderedSelectBounds(bounds, &l,&t,&r,&b);

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
                    for (size_t i=0; i<clipboard.data.size(); ++i) {
                        if (!tab.tileLayerActive[i]) continue;

                        const auto& srcLayer = tab.level.data[i];
                        auto& dstLayer = clipboard.data[i];

                        for (int y=b; y<=t; ++y) {
                            for (int x=l; x<=r; ++x) {
                                dstLayer[(y-b)*w+(x-l)] = srcLayer[y*tab.level.header.width+x];
                            }
                        }
                    }
                }
            }
        }
    }

    TEINAPI Vec2 MouseToTilePosition ()
    {
        // Only continue calculating the tile position if the mouse is in bounds.
        if (!MouseInsideLevelEditorViewport()) return Vec2(-1,-1);
        Vec2 m = gLevelEditor.mouseWorld;
        m.x = floorf((m.x - gLevelEditor.bounds.x) / gDefaultTileSize);
        m.y = floorf((m.y - gLevelEditor.bounds.y) / gDefaultTileSize);
        return m;
    }

    TEINAPI void HandleBrush ()
    {
        Vec2 tilePos = MouseToTilePosition();

        int x = static_cast<int>(tilePos.x);
        int y = static_cast<int>(tilePos.y);

        bool place = (gLevelEditor.toolState == ToolState::Place);
        TileID id = ((place) ? GetSelectedTile() : 0);
        PlaceMirroredTile(x, y, id, GetSelectedLayer());
    }

    TEINAPI TileID GetFillFindID (int x, int y, LevelLayer layer)
    {
        if (!TileInBounds(x,y)) return 0;
        const auto& tab = GetCurrentTab();
        return tab.level.data[static_cast<int>(layer)][y*tab.level.header.width+x];
    }

    TEINAPI bool InsideSelectBounds (int x, int y)
    {
        if (!AreThereAnyTabs()) return false;
        const Tab& tab = GetCurrentTab();
        for (auto& bounds: tab.toolInfo.select.bounds) {
            if (bounds.visible) {
                int sl,st,sr,sb;
                GetOrderedSelectBounds(bounds, &sl,&st,&sr,&sb);
                if (x >= sl && x <= sr && y >= sb && y <= st) {
                    return true;
                }
            }
        }
        return false;
    }

    TEINAPI void CheckFillNeighbor (int x, int y)
    {
        Tab& tab = GetCurrentTab();

        // If the select box is visible then check if we should be filling inside
        // or outside of the select box bounds. Based on that case we discard any
        // tiles/spawns that do not fit in the bounds we are to be filling within.
        if (AreAnySelectBoxesVisible()) {
            if (tab.toolInfo.fill.insideSelect) {
                if (!InsideSelectBounds(x,y)) return;
            } else {
                if (InsideSelectBounds(x,y)) return;
            }
        }

        // If we have already visited this space then we don't need to again.
        size_t index = y * tab.level.header.width + x;
        if (tab.toolInfo.fill.searched.at(index)) return;

        if (GetFillFindID(x, y, tab.toolInfo.fill.layer) == tab.toolInfo.fill.findID) {
            PlaceMirroredTile(x, y, tab.toolInfo.fill.replaceID, tab.toolInfo.fill.layer);
            tab.toolInfo.fill.frontier.push_back({ x,y });
        }

        tab.toolInfo.fill.searched.at(index) = true; // Mark as searched!
    }

    TEINAPI void Fill ()
    {
        Tab& tab = GetCurrentTab();

        int w = tab.level.header.width;
        int h = tab.level.header.height;

        tab.toolInfo.fill.searched.resize(w*h, false);

        int startX = static_cast<int>(tab.toolInfo.fill.start.x);
        int startY = static_cast<int>(tab.toolInfo.fill.start.y);

        // Start tile marked searched as we can just replace it now.
        PlaceMirroredTile(startX, startY, tab.toolInfo.fill.replaceID, tab.toolInfo.fill.layer);

        tab.toolInfo.fill.searched.at(startY * w + startX) = true;
        tab.toolInfo.fill.frontier.push_back(tab.toolInfo.fill.start);

        // Once the frontier is empty this means all tiles within the potentially
        // enclosed space have been filled and there is no where else to expand
        // to within the level. Meaning that the fill is complete and we can leave.
        while (tab.toolInfo.fill.frontier.size() > 0) {
            Vec2 temp = tab.toolInfo.fill.frontier.at(0);

            int cx = static_cast<int>(temp.x);
            int cy = static_cast<int>(temp.y);

            tab.toolInfo.fill.frontier.erase(tab.toolInfo.fill.frontier.begin());

            // Check the neighbors, but don't try to access outside level bounds.
            if (cy > 0)     CheckFillNeighbor(cx,   cy-1);
            if (cx < (w-1)) CheckFillNeighbor(cx+1, cy  );
            if (cy < (h-1)) CheckFillNeighbor(cx,   cy+1);
            if (cx > 0)     CheckFillNeighbor(cx-1, cy  );
        }

        tab.toolInfo.fill.searched.clear();
        tab.toolInfo.fill.frontier.clear();
    }

    TEINAPI void Replace ()
    {
        Tab& tab = GetCurrentTab();

        auto& layer = tab.level.data[static_cast<int>(tab.toolInfo.fill.layer)];
        for (int y=0; y<tab.level.header.height; ++y) {
            for (int x=0; x<tab.level.header.width; ++x) {
                // If the select box is visible then check if we should be replacing inside
                // or outside of the select box bounds. Based on that case we discard any
                // tiles/spawns that do not fit in the bounds we are to be replacing within.
                if (AreAnySelectBoxesVisible()) {
                    if (tab.toolInfo.fill.insideSelect) {
                        if (!InsideSelectBounds(x,y)) continue;
                    } else {
                        if (InsideSelectBounds(x,y)) continue;
                    }
                }

                if (layer[y*tab.level.header.width+x] == tab.toolInfo.fill.findID) {
                    PlaceTile(x, y, tab.toolInfo.fill.replaceID, tab.toolInfo.fill.layer);
                }
            }
        }
    }

    TEINAPI void HandleFill ()
    {
        Vec2 tilePos = MouseToTilePosition();

        int x = static_cast<int>(tilePos.x);
        int y = static_cast<int>(tilePos.y);

        // Do not bother starting a fill if out of bounds!
        if (!TileInBounds(x,y)) return;

        Tab& tab = GetCurrentTab();

        bool place = (gLevelEditor.toolState == ToolState::Place);
        TileID id = ((place) ? GetSelectedTile() : 0);

        tab.toolInfo.fill.start = { tilePos.x, tilePos.y };
        tab.toolInfo.fill.layer = GetSelectedLayer();
        tab.toolInfo.fill.findID = GetFillFindID(x, y, tab.toolInfo.fill.layer);
        tab.toolInfo.fill.replaceID = id;

        // Determine if the origin of the fill is inside a selection box or not.
        // This part does not matter if a selection box is not currently present.
        tab.toolInfo.fill.insideSelect = false;
        for (auto& bounds: tab.toolInfo.select.bounds) {
            if (bounds.visible) {
                int sl,st,sr,sb;
                GetOrderedSelectBounds(bounds, &sl,&st,&sr,&sb);
                if (x >= sl && x <= sr && y >= sb && y <= st) {
                    tab.toolInfo.fill.insideSelect = true;
                }
            }
        }

        // If the IDs are the same there is no need to fill.
        if (tab.toolInfo.fill.findID == tab.toolInfo.fill.replaceID) return;

        // Determine if we are doing a fill or find/replace.
        if (IsKeyModStateActive(KMOD_ALT)) Replace(); else Fill();
    }

    TEINAPI void RestoreSelectState (const std::vector<SelectBounds>& selectState)
    {
        if (AreThereAnyTabs()) {
            Tab& tab = GetCurrentTab();
            tab.toolInfo.select.bounds = selectState;
        }
    }

    TEINAPI void Deselect ()
    {
        if (!AreThereAnyTabs()) return;
        Tab& tab = GetCurrentTab();
        tab.toolInfo.select.bounds.clear();
    }

    TEINAPI void HandleSelect ()
    {
        // Right clicking whilst using the select tool remove the current selection.
        if (gLevelEditor.toolState == ToolState::Erase) {
            Deselect();
            return;
        }

        // We do this because otherwise if we have a selection box up and we click
        // outside of the level editor viewport region then it will try calculate
        // a new selection box -- instead we want it to do absolutely nothing.
        if (!MouseInsideLevelEditorViewport()) return;

        Tab& tab = GetCurrentTab();

        // If it is the start of a new selection then we do some extra stuff.
        if (tab.toolInfo.select.start) {
            // Cache the old state so we can add it to the history for undo/redo.
            tab.oldSelectState = tab.toolInfo.select.bounds;

            // Clear the old selection box(es) if we are not adding a new one.
            if (!tab.toolInfo.select.add) {
                tab.toolInfo.select.bounds.clear();
            }

            tab.toolInfo.select.bounds.push_back(SelectBounds());
            SelectBounds& selectBounds = tab.toolInfo.select.bounds.back();

            Vec2 ta = MouseToTilePosition();
            // Set the starting anchor point of the selection (clamp in bounds).
            selectBounds.top = std::clamp(static_cast<int>(ta.y), 0, tab.level.header.height-1);
            selectBounds.left = std::clamp(static_cast<int>(ta.x), 0, tab.level.header.width-1);

            tab.toolInfo.select.start = false;
            selectBounds.visible = false;
        }

        SelectBounds& selectBounds = tab.toolInfo.select.bounds.back();

        // We do this initial check so that if the user started by dragging
        // their selection box from outside the editor bounds then it will
        // not start the actual selection until their mouse enters the bounds.
        if (!selectBounds.visible) {
            Vec2 a = MouseToTilePosition();
            if (TileInBounds(static_cast<int>(a.x), static_cast<int>(a.y))) {
                selectBounds.visible = true;
            }
        }

        if (selectBounds.visible) {
            // Set the second point of the selection box bounds.
            Vec2 tb = MouseToTilePosition();
            selectBounds.bottom = std::clamp(static_cast<int>(tb.y), 0, tab.level.header.height-1);
            selectBounds.right = std::clamp(static_cast<int>(tb.x), 0, tab.level.header.width-1);
        }
    }

    TEINAPI void HandleCurrentTool ()
    {
        // Don't need to do anything, the user isn't using the tool right now!
        if (gLevelEditor.toolState == ToolState::Idle) return;

        switch (gLevelEditor.toolType) {
            case (ToolType::Brush): HandleBrush(); break;
            case (ToolType::Fill): HandleFill(); break;
            case (ToolType::Select): HandleSelect(); break;
        }
    }

    TEINAPI void FlipLevelH (const bool tileLayerActive[LevelLayer::Total])
    {
        Tab& tab = GetCurrentTab();

        int lw = tab.level.header.width;
        int lh = tab.level.header.height;

        int x = 0;
        int y = 0;
        int w = lw;
        int h = lh;

        // Flip all of the level's tiles.
        for (int i=0; i<static_cast<int>(LevelLayer::Total); ++i) {
            if (!tileLayerActive[i]) continue;

            auto& layer = tab.level.data[i];
            // Swap the tile columns from left-to-right for each row.
            for (int j=y; j<(y+h); ++j) {
                int r = x + (j*lw) + (w-1);
                int l = x + (j*lw);

                // Stop after we hit the middle point (no need to flip).
                while (l < r) {
                    TileID rt = layer[r];
                    TileID lt = layer[l];

                    layer[r--] = GetTileHorizontalFlip(lt);
                    layer[l++] = GetTileHorizontalFlip(rt);
                }
            }
        }

        GetCurrentTab().unsavedChanges = true;
    }

    TEINAPI void FlipLevelV (const bool tileLayerActive[LevelLayer::Total])
    {
        Tab& tab = GetCurrentTab();

        int lw = tab.level.header.width;
        int lh = tab.level.header.height;

        int x = 0;
        int y = 0;
        int w = lw;
        int h = lh;

        // Flip all of the level's tiles.
        std::vector<TileID> tempRow;
        tempRow.resize(w);

        for (int i=0; i<static_cast<int>(LevelLayer::Total); ++i) {
            if (!tileLayerActive[i]) continue;

            auto& layer = tab.level.data[i];
            size_t pitch = w * sizeof(TileID);

            int b = (y * lw) + x;
            int t = ((y+h-1) * lw) + x;

            // Stop after we hit the middle point (no need to flip).
            while (b < t) {
                memcpy(&tempRow[0], &layer  [b], pitch);
                memcpy(&layer  [b], &layer  [t], pitch);
                memcpy(&layer  [t], &tempRow[0], pitch);

                for (int j=0; j<lw; ++j) {
                    layer[t+j] = GetTileVerticalFlip(layer[t+j]);
                    layer[b+j] = GetTileVerticalFlip(layer[b+j]);
                }

                b += lw;
                t -= lw;
            }
        }

        GetCurrentTab().unsavedChanges = true;
    }

    TEINAPI void DrawCursor (int x, int y, TileID id)
    {
        // Do not try to draw the cursor if it is out of bounds!
        if (!TileInBounds(x,y)) return;

        // We don't want to do this if select is the current tool.
        if (gLevelEditor.toolType == ToolType::Brush || gLevelEditor.toolType == ToolType::Fill) {
            float gx = gLevelEditor.bounds.x + (x * gDefaultTileSize);
            float gy = gLevelEditor.bounds.y + (y * gDefaultTileSize);

            StencilModeDraw();
            SetDrawColor(gEditorSettings.cursorColor);
            FillQuad(gx, gy, gx+gDefaultTileSize, gy+gDefaultTileSize);

            TextureAtlas& atlas = GetEditorAtlasLarge();

            atlas.texture.color.a = gGhostedCursorAlpha;
            DrawTexture(atlas.texture, gx+gDefaultTileSizeHalf, gy+gDefaultTileSizeHalf, &GetTileGraphicClip(atlas, id));
            atlas.texture.color.a = 1; // Important!

            StencilModeErase();
            SetDrawColor(1,1,1,1);
            FillQuad(gx, gy, gx+gDefaultTileSize, gy+gDefaultTileSize);

            atlas.texture.color = Vec4(1,1,1,1);
            DrawTexture(atlas.texture, gx+gDefaultTileSizeHalf, gy+gDefaultTileSizeHalf, &GetTileGraphicClip(atlas, id));
        }
    }

    TEINAPI void DrawMirroredCursor ()
    {
        // No need to draw if we do not have focus.
        if (!IsWindowFocused("WINMAIN")) return;

        bool both = (gLevelEditor.mirrorH && gLevelEditor.mirrorV);

        const Tab& tab = GetCurrentTab();

        int lw = tab.level.header.width-1;
        int lh = tab.level.header.height-1;

        Vec2 t = MouseToTilePosition();

        int tx = static_cast<int>(t.x);
        int ty = static_cast<int>(t.y);

        TileID id = GetSelectedTile();

        BeginStencil();

                                  DrawCursor(   tx,    ty,                                           id  );
        if (gLevelEditor.mirrorH) DrawCursor(lw-tx,    ty, GetTileHorizontalFlip                    (id) );
        if (gLevelEditor.mirrorV) DrawCursor(   tx, lh-ty,                       GetTileVerticalFlip(id) );
        if (both)                 DrawCursor(lw-tx, lh-ty, GetTileHorizontalFlip(GetTileVerticalFlip(id)));

        EndStencil();
    }

    TEINAPI void DrawClipboardHighlight (UiDir xDir, UiDir yDir)
    {
        BeginStencil();
        for (auto& clipboard: gLevelEditor.clipboard) {
            const Tab& tab = GetCurrentTab();

            int lw = tab.level.header.width-1;
            int lh = tab.level.header.height-1;

            int sw = clipboard.w-1;
            int sh = clipboard.h-1;

            Vec2 t = MouseToTilePosition();

            int tx = static_cast<int>(t.x) + clipboard.x;
            int ty = static_cast<int>(t.y) + clipboard.y;

            if (xDir == UiDir::Left) tx = lw-sw-tx;
            if (yDir == UiDir::Down) ty = lh-sh-ty;

            float gx = gLevelEditor.bounds.x + (tx * gDefaultTileSize);
            float gy = gLevelEditor.bounds.y + (ty * gDefaultTileSize);
            float gw = clipboard.w * gDefaultTileSize;
            float gh = clipboard.h * gDefaultTileSize;

            StencilModeDraw();
            SetDrawColor(gEditorSettings.cursorColor);
            FillQuad(gx, gy, gx+gw, gy+gh);

            StencilModeErase();
            SetDrawColor(1,1,1,1);
            FillQuad(gx, gy, gx+gw, gy+gh);
        }
        EndStencil();
    }

    TEINAPI void DrawClipboard (UiDir xDir, UiDir yDir)
    {
        TextureAtlas& atlas = GetEditorAtlasLarge();

        SetTileBatchTexture(atlas.texture);
        SetTileBatchColor(Vec4(1,1,1,gGhostedCursorAlpha));

        // Stops us from drawing multiple copies of a tile where clipboards overlap.
        std::array<std::map<size_t,bool>, static_cast<size_t>(LevelLayer::Total)> tileSpaceOccupied;
        for (auto& clipboard: gLevelEditor.clipboard) {
            const Tab& tab = GetCurrentTab();

            int lw = tab.level.header.width-1;
            int lh = tab.level.header.height-1;

            int sw = clipboard.w-1;
            int sh = clipboard.h-1;

            Vec2 t = MouseToTilePosition();

            int x = static_cast<int>(t.x) + clipboard.x;
            int y = static_cast<int>(t.y) + clipboard.y;

            if (xDir == UiDir::Left) x = lw-sw-x;
            if (yDir == UiDir::Down) y = lh-sh-y;

            float gx = gLevelEditor.bounds.x + (x * gDefaultTileSize);
            float gy = gLevelEditor.bounds.y + (y * gDefaultTileSize);
            float gw = clipboard.w * gDefaultTileSize;
            float gh = clipboard.h * gDefaultTileSize;

            // Draw all of the select buffer tiles.
            for (int i=0; i<clipboard.data.size(); ++i) {
                // If the layer is not active then we do not bother drawing its clipboard content.
                if (!tab.tileLayerActive[i]) continue;

                // We start the Y axis from the bottom because the game stores data
                // with coordinate (0,0) being the bottom left of the current level.
                float ty = 0;
                float tx = 0;

                if      (xDir == UiDir::Right) tx = gx;
                else if (xDir == UiDir::Left ) tx = gx+gw-gDefaultTileSize;
                if      (yDir == UiDir::Up   ) ty = gy;
                else if (yDir == UiDir::Down ) ty = gy+gh-gDefaultTileSize;

                const auto& layer = clipboard.data[i];
                auto& layerSpaceOccupied = tileSpaceOccupied[i];

                for (size_t j=0; j<layer.size(); ++j) {
                    TileID id = layer[j];
                    if (id) { // No point drawing empty tiles...
                        if (!layerSpaceOccupied.count(j)) {
                            if (xDir == UiDir::Left) id = GetTileHorizontalFlip(id);
                            if (yDir == UiDir::Down) id = GetTileVerticalFlip(id);

                            DrawBatchedTile(tx+gDefaultTileSizeHalf, ty+gDefaultTileSizeHalf, &GetTileGraphicClip(atlas, id));
                            layerSpaceOccupied.insert(std::pair<size_t,bool>(j, true));
                        }
                    }

                    // Move to the next tile based on our direction.
                    if (xDir == UiDir::Right && yDir == UiDir::Up) {
                        tx += gDefaultTileSize;
                        if ((tx+gDefaultTileSize) > (gx+gw)) {
                            ty += gDefaultTileSize;
                            tx = gx;
                        }
                    } else if (xDir == UiDir::Left && yDir == UiDir::Up) {
                        tx -= gDefaultTileSize;
                        if ((tx) < gx) {
                            ty += gDefaultTileSize;
                            tx = gx+gw-gDefaultTileSize;
                        }
                    } else if (xDir == UiDir::Right && yDir == UiDir::Down) {
                        tx += gDefaultTileSize;
                        if ((tx+gDefaultTileSize) > (gx+gw)) {
                            ty -= gDefaultTileSize;
                            tx = gx;
                        }
                    } else if (xDir == UiDir::Left && yDir == UiDir::Down) {
                        tx -= gDefaultTileSize;
                        if ((tx) < gx) {
                            ty -= gDefaultTileSize;
                            tx = gx+gw-gDefaultTileSize;
                        }
                    }
                }
            }
        }

        FlushBatchedTiles();
    }

    TEINAPI void DrawMirroredClipboard ()
    {
        bool both = (gLevelEditor.mirrorH && gLevelEditor.mirrorV);

        if (!IsWindowFocused("WINMAIN")) return;

                                  DrawClipboardHighlight(UiDir::Right, UiDir::Up  );
        if (gLevelEditor.mirrorH) DrawClipboardHighlight(UiDir::Left,  UiDir::Up  );
        if (gLevelEditor.mirrorV) DrawClipboardHighlight(UiDir::Right, UiDir::Down);
        if (both)                 DrawClipboardHighlight(UiDir::Left,  UiDir::Down);
                                  DrawClipboard         (UiDir::Right, UiDir::Up  );
        if (gLevelEditor.mirrorH) DrawClipboard         (UiDir::Left,  UiDir::Up  );
        if (gLevelEditor.mirrorV) DrawClipboard         (UiDir::Right, UiDir::Down);
        if (both)                 DrawClipboard         (UiDir::Left,  UiDir::Down);
    }

    TEINAPI void DumpLevelHistory ()
    {
        const Tab& tab = GetCurrentTab();

        BeginDebugSection("History Stack Dump:");
        for (int i=0; i<static_cast<int>(tab.levelHistory.state.size()); ++i) {
            const LevelHistoryState& s = tab.levelHistory.state.at(i);
            std::string historyState = (tab.levelHistory.currentPosition==i) ? ">" : " ";

            switch (s.action) {
                case (LevelHistoryAction::Normal     ): historyState += "| NORMAL | "; break;
                case (LevelHistoryAction::FlipLevelH ): historyState += "| FLIP H | "; break;
                case (LevelHistoryAction::FlipLevelV ): historyState += "| FLIP V | "; break;
                case (LevelHistoryAction::SelectState): historyState += "| SELECT | "; break;
                case (LevelHistoryAction::Clear      ): historyState += "| CLEAR  | "; break;
                case (LevelHistoryAction::Resize     ): historyState += "| RESIZE | "; break;
            }

            historyState += FormatString("%5zd | ", s.info.size());

            if (s.action == LevelHistoryAction::FlipLevelH || s.action == LevelHistoryAction::FlipLevelV) {
                for (const auto& tileLayer: s.tileLayerActive) {
                    historyState += (tileLayer) ? "X" : ".";
                }
            }
            else
            {
                historyState += ". ";
                for (const auto& tileLayer: tab.tileLayerActive)
                {
                    historyState += ".";
                }
            }

            historyState += " |";
            LogDebug("%s", historyState.c_str());
        }
        EndDebugSection();
    }

    TEINAPI void Resize (ResizeDir dir, int newWidth, int newHeight)
    {
        Tab& tab = GetCurrentTab();

        int lw = tab.level.header.width;
        int lh = tab.level.header.height;

        int dx = newWidth - lw;
        int dy = newHeight - lh;

        if (dx == 0 && dy == 0) return;

        LevelData oldData = tab.level.data;
        for (auto& layer: tab.level.data) {
            layer.clear();
            layer.resize(newWidth*newHeight, 0);
        }

        int lvlW = lw;
        int lvlH = lh;
        int lvlX = (newWidth-lvlW) / 2;
        int lvlY = (newHeight-lvlH) / 2;

        int offX = 0;
        int offY = 0;

        // Determine the content offset needed if shrinking the level down.
        if (dx < 0) {
            if (ResizeDirIsWest (dir)) lvlW -= abs(dx);
            else if (ResizeDirIsEast (dir)) lvlW -= abs(dx), offX += abs(dx);
            else lvlW -= abs(dx), offX += abs(dx) / 2;
        }
        if (dy < 0) {
            if (ResizeDirIsNorth(dir)) lvlH -= abs(dy);
            else if (ResizeDirIsSouth(dir)) lvlH -= abs(dy), offY += abs(dy);
            else lvlH -= abs(dy), offY += abs(dy) / 2;
        }

        // Determine the horizontal position of the level content.
        if (ResizeDirIsWest(dir)) lvlX = 0;
        else if (ResizeDirIsEast(dir)) lvlX = newWidth - lvlW;
        // Determine the vertical position of the level content.
        if (ResizeDirIsNorth(dir)) lvlY = 0;
        else if (ResizeDirIsSouth(dir)) lvlY = newHeight - lvlH;

        // Make sure not out of bounds!
        if (lvlX < 0) lvlX = 0;
        if (lvlY < 0) lvlY = 0;

        for (int i=0; i<static_cast<int>(LevelLayer::Total); ++i) {
            auto& newLayer = tab.level.data.at(i);
            auto& oldLayer = oldData.at(i);

            for (int iy=0; iy<lvlH; ++iy) {
                for (int ix=0; ix<lvlW; ++ix) {
                    int nPos = (iy+lvlY) * newWidth + (ix+lvlX);
                    int oPos = (iy+offY) * lw + (ix+offX);

                    newLayer.at(nPos) = oldLayer.at(oPos);
                }
            }
        }

        tab.level.header.width = newWidth;
        tab.level.header.height = newHeight;

        LevelHasUnsavedChanges();
    }
}

TEINAPI void InitLevelEditor ()
{
    gLevelEditor.toolState = ToolState::Idle;
    gLevelEditor.toolType = ToolType::Brush;

    gLevelEditor.mouseWorld = Vec2(0,0);
    gLevelEditor.mouse = Vec2(0,0);
    gLevelEditor.mouseTile = Vec2(0,0);

    gLevelEditor.boundsVisible = true;
    gLevelEditor.layerTransparency = true;

    gLevelEditor.mirrorH = false;
    gLevelEditor.mirrorV = false;

    gLevelEditor.bounds = { 0,0,0,0 };
    gLevelEditor.viewport = { 0,0,0,0 };
}

TEINAPI void DoLevelEditor ()
{
    Quad p1;

    p1.x = GetToolbarWidth() + 1;
    p1.y = gTabBarHeight  + 1;
    p1.w = GetViewport().w - GetToolbarWidth() - (GetControlPanelWidth()) - 2;
    p1.h = GetViewport().h - gStatusBarHeight - gTabBarHeight - 2;

    BeginPanel(p1.x,p1.y,p1.w,p1.h, UiFlag::None);

    // We cache the mouse position so that systems such as paste which can
    // potentially happen outside of this section of code (where the needed
    // transforms will be applied) can use the mouse position reliably as
    // prior to doing this there were bugs with the cursor's position being
    // slightly off during those operations + it's probably a bit faster.
    PushEditorCameraTransform();
    gLevelEditor.mouseWorld = ScreenToWorld(GetMousePos());
    gLevelEditor.mouse = GetMousePos();
    gLevelEditor.mouseTile = Internal::MouseToTilePosition();
    PopEditorCameraTransform();

    // We cache this just in case anyone else wants to use it (status bar).
    gLevelEditor.viewport = GetViewport();

    const Tab& tab = GetCurrentTab();

    // If we're in the level editor viewport then the cursor can be one of
    // the custom tool cursors based on what our current tool currently is.
    if (MouseInsideLevelEditorViewport() && IsWindowFocused("WINMAIN")) {
        switch (gLevelEditor.toolType) {
            case (ToolType::Brush): SetCursorType(Cursor::Brush); break;
            case (ToolType::Fill): SetCursorType(Cursor::Fill); break;
            case (ToolType::Select): SetCursorType(Cursor::Select); break;
        }
    } else {
        // We do this check so it doesn't mess with text box and hyperlink UI cursors!
        if (GetCursorType() != Cursor::Beam && GetCursorType() != Cursor::Pointer) {
            SetCursorType(Cursor::Arrow);
        }
    }

    PushEditorCameraTransform();

    // The boundaries of the actual level content (tiles/spawns).
    gLevelEditor.bounds.w = tab.level.header.width  * gDefaultTileSize;
    gLevelEditor.bounds.h = tab.level.header.height * gDefaultTileSize;
    gLevelEditor.bounds.x = (GetViewport().w - gLevelEditor.bounds.w) / 2;
    gLevelEditor.bounds.y = (GetViewport().h - gLevelEditor.bounds.h) / 2;

    float x = gLevelEditor.bounds.x;
    float y = gLevelEditor.bounds.y;
    float w = gLevelEditor.bounds.w;
    float h = gLevelEditor.bounds.h;

    float tileScale = gDefaultTileSize / gTileImageSize;
    SetTextureDrawScale(tileScale, tileScale);

    // We cache the transformed level editor bounds in screen coordinates so
    // that we can later scissor the area to avoid any tile/spawn overspill.
    Vec2 levelEditorBoundsA = WorldToScreen(Vec2(x,y));
    Vec2 levelEditorBoundsB = WorldToScreen(Vec2(x+w,y+h));

    // Because we mess with the orthographic projection matrix a pixel is no
    // longer 1.0f so we need to adjust by the current zoom to get a pixel.
    //
    // We do this because otherwise the outer border gets scaled incorrectly
    // and looks quite ugly. This method ensures it always remains 1px thick.
    float px = (1 / tab.camera.zoom);

    SetDrawColor(gUiColorBlack);
    FillQuad(x-px,y-px,x+w+px,y+h+px);
    SetDrawColor(gEditorSettings.backgroundColor);
    FillQuad(x,y,x+w,y+h);

    // Determine the currently selected layer so that we can make all of the
    // layers above semi-transparent. If we're the spawn layer (top) then it
    // we don't really have a layer so we just assign to minus one to mark.
    LevelLayer selectedLayer = GetSelectedLayer();

    constexpr float SemiTrans = .6f;

    // Scissor the content of the level editor region to avoid any overspill.
    float scx = floorf(levelEditorBoundsA.x);
    float scy = floorf(levelEditorBoundsA.y);
    float scw = ceilf (levelEditorBoundsB.x - scx);
    float sch = ceilf (levelEditorBoundsB.y - scy);

    BeginScissor(scx,scy,scw,sch);

    TextureAtlas& atlas = GetEditorAtlasLarge();
    SetTileBatchTexture(atlas.texture);

    // Draw all of the tiles for the level, layer-by-layer.
    for (int i=static_cast<int>(LevelLayer::Back2); (i<=static_cast<int>(LevelLayer::Back2))&&(i>=static_cast<int>(LevelLayer::Tag)); --i) {
        // If the layer is not active then we do not bother drawing its content.
        if (!tab.tileLayerActive[i]) continue;

        if (gLevelEditor.layerTransparency && ((selectedLayer != LevelLayer::Tag) && (static_cast<int>(selectedLayer) > i))) {
            SetTileBatchColor(Vec4(1,1,1,SemiTrans));
        } else {
            SetTileBatchColor(Vec4(1,1,1,1));
        }

        // We draw from the top-to-bottom, left-to-right, so that elements
        // in the level editor stack up nicely on top of each other.
        float ty = y+gDefaultTileSizeHalf;
        float tx = x+gDefaultTileSizeHalf;

        const auto& layer = tab.level.data[i];
        for (int j=0; j<static_cast<int>(layer.size()); ++j) {
            if (layer[j] != 0) { // No point drawing empty tiles...
                DrawBatchedTile(tx, ty, &Internal::GetTileGraphicClip(atlas, layer[j]));
            }

            // Move to the next tile in the row, move down if needed.
            tx += gDefaultTileSize;
            if (tx >= (x + w)) {
                ty += gDefaultTileSize;
                tx = x+gDefaultTileSizeHalf;
            }
        }
    }

    FlushBatchedTiles();

    // Draw either a ghosted version of the currently selected tile or what is
    // currently in the clipboard. What we draw depends on if the key modifier
    // for pasting is currently being pressed or not (by default this is CTRL).
    if (!AreAllLayersInactive()) {
        if (!IsAWindowResizing() && MouseInsideLevelEditorViewport()) {
            if (!Internal::ClipboardEmpty() && IsKeyModStateActive(GetKeyBinding(gKbPaste).mod)) {
                Internal::DrawMirroredClipboard();
            } else {
                Internal::DrawMirroredCursor();
            }
        }
    }

    EndScissor();

    // Draw the greyed out area outside of the level's camera bounds.
    if (gLevelEditor.boundsVisible) {
        // It seems, from testing, that the game uses the camera tiles to calculate the visible
        // area of a level by taking the most extreme camera tile positions and uses those to
        // create a bounding box of the top, left, right, and down-most camera tile placements.
        //
        // So in order to get the most accurate camera bounding box for the editor we too must
        // obtain these values by searching through the level data, and then rendering this.

        int lw = tab.level.header.width;
        int lh = tab.level.header.height;

        auto& tagLayer = tab.level.data[static_cast<int>(LevelLayer::Tag)];

        int cl = lw-1;
        int ct = lh-1;
        int cr = 0;
        int cb = 0;

        int cameraTileCount = 0;

        for (int iy=0; iy<lh; ++iy) {
            for (int ix=0; ix<lw; ++ix) {
                TileID id = tagLayer[iy * tab.level.header.width + ix];
                if (id == gCameraID) {
                    ++cameraTileCount;

                    cl = std::min(cl, ix);
                    ct = std::min(ct, iy);
                    cr = std::max(cr, ix);
                    cb = std::max(cb, iy);
                }
            }
        }

        // If we have a camera tile selected we can also use that to showcase how it will impact the bounds.
        if (gLevelEditor.toolType != ToolType::Select) {
            if (GetSelectedTile() == gCameraID) {
                if (MouseInsideLevelEditorViewport()) {
                    Vec2 tile = Internal::MouseToTilePosition();
                    ++cameraTileCount;

                    cl = std::min(cl, static_cast<int>(tile.x));
                    ct = std::min(ct, static_cast<int>(tile.y));
                    cr = std::max(cr, static_cast<int>(tile.x));
                    cb = std::max(cb, static_cast<int>(tile.y));
                }
            }
        }

        // If there is just one then there is no bounds.
        if (cameraTileCount == 1) {
            cl = lw-1;
            ct = lh-1;
            cr = 0;
            cb = 0;
        }

        float cx1 = x + (static_cast<float>(std::min(cl, cr)    ) * gDefaultTileSize);
        float cy1 = y + (static_cast<float>(std::min(ct, cb)    ) * gDefaultTileSize);
        float cx2 = x + (static_cast<float>(std::max(cl, cr) + 1) * gDefaultTileSize);
        float cy2 = y + (static_cast<float>(std::max(ct, cb) + 1) * gDefaultTileSize);

        BeginStencil();

        StencilModeErase();
        SetDrawColor(Vec4(1,1,1,1));
        FillQuad(cx1,cy1,cx2,cy2);

        StencilModeDraw();
        SetDrawColor(gEditorSettings.outOfBoundsColor);
        FillQuad(x,y,x+w,y+h);

        EndStencil();
    }

    // Draw the selection box(es) if visible.
    BeginStencil();
    for (auto& bounds: tab.toolInfo.select.bounds) {
        if (bounds.visible) {
            int il,it,ir,ib;
            GetOrderedSelectBounds(bounds, &il,&it,&ir,&ib);

            float l =       static_cast<float>(il);
            float r = ceilf(static_cast<float>(ir)+.5f);
            float b =       static_cast<float>(ib);
            float t = ceilf(static_cast<float>(it)+.5f);

            float sx1 = x   + (l     * gDefaultTileSize);
            float sy1 = y   + (b     * gDefaultTileSize);
            float sx2 = sx1 + ((r-l) * gDefaultTileSize);
            float sy2 = sy1 + ((t-b) * gDefaultTileSize);

            StencilModeDraw();
            SetDrawColor(gEditorSettings.selectColor);
            FillQuad(sx1,sy1,sx2,sy2);

            StencilModeErase();
            SetDrawColor(1,1,1,1);
            FillQuad(sx1,sy1,sx2,sy2);
        }
    }
    EndStencil();

    // Draw the tile/spawn grid for the level editor.
    if (gEditor.gridVisible) {
        BeginDraw(BufferMode::Lines);
        for (float ix=x+gDefaultTileSize; ix<(x+w); ix+=gDefaultTileSize) {
            PutVertex(ix,y, Vec4(gEditorSettings.tileGridColor));
            PutVertex(ix,y+h, Vec4(gEditorSettings.tileGridColor));
        }
        for (float iy=y+gDefaultTileSize; iy<(y+h); iy+=gDefaultTileSize) {
            PutVertex(x,iy, Vec4(gEditorSettings.tileGridColor));
            PutVertex(x+w,iy, Vec4(gEditorSettings.tileGridColor));
        }
        EndDraw();
    }

    // Draw the large entity guides if they are enabled.
    if (gLevelEditor.largeTiles && gLevelEditor.entityGuides) {
        if (tab.tileLayerActive[static_cast<int>(LevelLayer::Active)]) {
            BeginScissor(scx,scy,scw,sch);

            Vec4 color = gEditorSettings.cursorColor;
            SetLineWidth(2);

            const float LineWidth = (gDefaultTileSize / 3) * 2; // 2/3
            const float Offset = roundf(LineWidth / 2);

            float ty = y+gDefaultTileSizeHalf;
            float tx = x+gDefaultTileSizeHalf;

            auto& layer = tab.level.data[static_cast<int>(LevelLayer::Active)];
            for (int i=0; i<static_cast<int>(layer.size()); ++i) {
                // Ensures that the tile is an Entity and not a Basic.
                TileID id = layer[i];
                if ((id != 0) && ((id-40000) >= 0))
                {
                    Quad& b = Internal::GetTileGraphicClip(atlas, id);

                    float hw = (b.w * tileScale) / 2;
                    float hh = (b.h * tileScale) / 2;

                    color.a = .20f;
                    SetDrawColor(color);

                    FillQuad(tx-hw,ty-hh,tx+hw,ty+hh);

                    color.a = .85f;
                    SetDrawColor(color);

                    DrawLine(tx-Offset,ty,tx+Offset,ty);
                    DrawLine(tx,ty-Offset,tx,ty+Offset);

                    DrawQuad(tx-hw,ty-hh,tx+hw,ty+hh);
                }

                // Move to the next tile in the row, move down if needed.
                tx += gDefaultTileSize;
                if (tx >= (x + w)) {
                    ty += gDefaultTileSize;
                    tx = x+gDefaultTileSizeHalf;
                }
            }

            EndScissor();
        }
    }

    // Draw the mirroring lines for the level editor.
    SetDrawColor(gEditorSettings.mirrorLineColor);
    SetLineWidth(std::max(1.0f, 3.0f/px));
    if (gLevelEditor.mirrorH) {
        float hw = w/2;
        DrawLine(x+hw,y,x+hw,y+h);
    }
    if (gLevelEditor.mirrorV) {
        float hh = h/2;
        DrawLine(x,y+hh,x+w,y+hh);
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
    FillQuad(x,y,x+w,y+h);

    StencilModeDraw();
    SetDrawColor(gUiColorBlack);
    FillQuad(x-px,y-px,x+w+px,y+h+px);

    EndStencil();

    PopEditorCameraTransform();

    EndPanel();

    SetTextureDrawScale(1,1);
}

TEINAPI void HandleLevelEditorEvents ()
{
    Tab* tab = &GetCurrentTab();

    // We don't want to handle any of these events if we don't have focus.
    // We set the tool state to idle here so that if the user was doing
    // an action and then opened another window with a hotkey, when they
    // return the action will not continue due to the tool being active.
    if (!IsWindowFocused("WINMAIN")) {
        gLevelEditor.toolState = ToolState::Idle;
        return;
    }

    switch (gMainEvent.type) {
        case (SDL_MOUSEBUTTONDOWN):
        case (SDL_MOUSEBUTTONUP): {
            // Do not handle these events whilst we are cooling down!
            if (gEditor.dialogBox) return;

            bool pressed = (gMainEvent.button.state == SDL_PRESSED);
            if (pressed && IsThereAHitUiElement()) return;

            switch (gMainEvent.button.button) {
                case (SDL_BUTTON_LEFT): {
                    if (pressed) {
                        gLevelEditor.toolState = ToolState::Place;

                        // This will be the start of a new selection!
                        if (gLevelEditor.toolType == ToolType::Select) {
                            tab->toolInfo.select.start = true;
                            tab->toolInfo.select.cachedSize = tab->toolInfo.select.bounds.size();
                        }
                        if (gLevelEditor.toolType == ToolType::Brush || gLevelEditor.toolType == ToolType::Fill) {
                            NewLevelHistoryState(LevelHistoryAction::Normal);
                        }

                        // Handle the current tool immediately so that placing/erasing
                        // doesn't require the user to move the mouse for it to work.
                        Internal::HandleCurrentTool();
                    } else {
                        gLevelEditor.toolState = ToolState::Idle;
                        if (gLevelEditor.toolType == ToolType::Select) {
                            if (tab->toolInfo.select.bounds.size() > tab->toolInfo.select.cachedSize) {
                                NewLevelHistoryState(LevelHistoryAction::SelectState);
                                tab->toolInfo.select.cachedSize = tab->toolInfo.select.bounds.size();
                            }
                        }
                    }
                } break;
                case (SDL_BUTTON_RIGHT): {
                    if (pressed) {
                        gLevelEditor.toolState = ToolState::Erase;

                        if (gLevelEditor.toolType == ToolType::Brush || gLevelEditor.toolType == ToolType::Fill) {
                            NewLevelHistoryState(LevelHistoryAction::Normal);
                        }

                        // Handle the current tool immediately so that placing/erasing
                        // doesn't require the user to move the mouse for it to work.
                        Internal::HandleCurrentTool();
                    } else {
                        gLevelEditor.toolState = ToolState::Idle;
                    }
                } break;
            }
        } break;
        case (SDL_MOUSEMOTION): {
            // We only want drag painting to happen if the entity allows it.
            // However, this is ignored if the tool is the selection box.
            if (gLevelEditor.toolState != ToolState::Idle) Internal::HandleCurrentTool();
        } break;
        case (SDL_KEYDOWN):
        case (SDL_KEYUP): {
            bool pressed = gMainEvent.key.state;
            switch (gMainEvent.key.keysym.sym) {
                // Handle toggling the select box addition mode using the CTRL key.
                case (SDLK_RCTRL): case (SDLK_LCTRL): tab->toolInfo.select.add = pressed; break;
            }
        } break;
    }

    // We can dump the history on command in debug mode.
    #if defined(BUILD_DEBUG)
    if (gMainEvent.type == SDL_KEYDOWN) {
        if (gMainEvent.key.keysym.sym == SDLK_F12) {
            Internal::DumpLevelHistory();
        }
    }
    #endif // BUILD_DEBUG
}

TEINAPI bool MouseInsideLevelEditorViewport ()
{
    Vec2 m = gLevelEditor.mouse;
    Quad v = gLevelEditor.viewport;

    // We do this check for the disabling of cursor drawing during a resize.
    // As once the resize is done normally this function would end up being
    // true and would then draw the cursor at the wrong place after resize
    // so this check prevents that from happening and looks visually better.
    if (!SDL_GetMouseFocus()) return false;

    return ((m.x>=v.x) && (m.y>=v.y) && (m.x<=(v.x+v.w)) && (m.y<=(v.y+v.h)));
}

TEINAPI void NewLevelHistoryState (LevelHistoryAction action)
{
    if (action == LevelHistoryAction::Normal && !MouseInsideLevelEditorViewport()) return;

    Tab& tab = GetCurrentTab();

    // Don't bother creating a new state if the current erase/place action is
    // empty otherwise we will end up with a bunch of empty states in the list.
    if (tab.levelHistory.currentPosition > -1)
    {
        LevelHistoryState& current = Internal::GetCurrentHistoryState();
        if (current.info.empty() && current.action == action && action == LevelHistoryAction::Normal) {
            return;
        }
    }

    // Clear all the history after the current position, if there is any, as it
    // will no longer apply to the timeline of level editor actions anymore.
    int deletePosition = tab.levelHistory.currentPosition+1;
    if (deletePosition < static_cast<int>(tab.levelHistory.state.size())) {
        auto begin = tab.levelHistory.state.begin();
        auto end = tab.levelHistory.state.end();

        tab.levelHistory.state.erase(begin+deletePosition, end);
    }

    // If it's a selection action then we don't need to modify this.
    if (action != LevelHistoryAction::SelectState) {
        GetCurrentTab().unsavedChanges = true;
    }

    tab.levelHistory.state.push_back(LevelHistoryState());
    tab.levelHistory.state.back().action = action;

    // Also deal with the layer states for flip actions.
    if (action == LevelHistoryAction::FlipLevelH || action == LevelHistoryAction::FlipLevelV) {
        for (int i=static_cast<int>(LevelLayer::Tag); i<static_cast<int>(LevelLayer::Total); ++i) {
            tab.levelHistory.state.back().tileLayerActive[i] = tab.tileLayerActive[i];
        }
    }

    // Also deal with the select bounds for selection actions.
    if (action == LevelHistoryAction::SelectState) {
        tab.levelHistory.state.back().oldSelectState = tab.oldSelectState;
        tab.levelHistory.state.back().newSelectState = tab.toolInfo.select.bounds;
    }

    // Also deal with width and height for resizing.
    if (action == LevelHistoryAction::Resize) {
        tab.levelHistory.state.back().resizeDir = GetResizeDir();
        tab.levelHistory.state.back().oldWidth = tab.level.header.width;
        tab.levelHistory.state.back().oldHeight = tab.level.header.height;
        tab.levelHistory.state.back().newWidth = GetResizeWidth();
        tab.levelHistory.state.back().newHeight = GetResizeHeight();
    }

    ++tab.levelHistory.currentPosition;
}

TEINAPI void AddToHistoryNormalState (LevelHistoryInfo info)
{
    if (!MouseInsideLevelEditorViewport()) return;

    Tab& tab = GetCurrentTab();

    // If there is no current action then we create one. This resolved some
    // potential bugs that can occur when undoing/redoing mid stroke, etc.
    if (tab.levelHistory.currentPosition <= -1) {
        NewLevelHistoryState(LevelHistoryAction::Normal);
    }

    // We also check if the current state is not of type normal because if
    // it is then we need to add a new normal state (because flip states
    // do not ever need to call this function). This resolves the issues of
    // the history getting messed up if the user flips the level mid-stroke.
    if (Internal::GetCurrentHistoryState().action != LevelHistoryAction::Normal) {
        NewLevelHistoryState(LevelHistoryAction::Normal);
    }

    // Don't add the same spawns/tiles repeatedly, otherwise add the spawn/tile.
    LevelHistoryState& state = Internal::GetCurrentHistoryState();
    if (!state.info.empty()) {
        const LevelHistoryInfo& n = info;
        for (auto& o: state.info) {
            if (o.x == n.x && o.y == n.y && o.tileLayer == n.tileLayer && o.newID != n.newID) o.newID = n.newID;
            if (o.x == n.x && o.y == n.y && o.tileLayer == n.tileLayer && o.newID == n.newID) return;
        }
    }

    state.info.push_back(info);
}

TEINAPI void AddToHistoryClearState (LevelHistoryInfo info)
{
    assert(Internal::GetCurrentHistoryState().action == LevelHistoryAction::Clear);
    Internal::GetCurrentHistoryState().info.push_back(info);
}

TEINAPI bool AreAllLayersInactive ()
{
    const Tab& tab = GetCurrentTab();
    for (auto tileLayerActive: tab.tileLayerActive) if (tileLayerActive) return false;
    return true;
}

TEINAPI bool AreAnySelectBoxesVisible ()
{
    if (!AreThereAnyTabs()) return false;
    const Tab& tab = GetCurrentTab();
    for (auto& bounds: tab.toolInfo.select.bounds) if (bounds.visible) return true;
    return false;
}

TEINAPI void GetOrderedSelectBounds (const SelectBounds& bounds, int* l, int* t, int* r, int* b)
{
    // We do this here rather than ordering it in the actual handle
    // select function because otherwise it would cause some issues.
    if (l) *l = std::min(bounds.left, bounds.right);
    if (r) *r = std::max(bounds.left, bounds.right);
    if (b) *b = std::min(bounds.top, bounds.bottom);
    if (t) *t = std::max(bounds.top, bounds.bottom);
}

TEINAPI void GetTotalSelectBoundary (int* l, int* t, int* r, int* b)
{
    if (l) *l = 0;
    if (t) *t = 0;
    if (r) *r = 0;
    if (b) *b = 0;

    if (!AreThereAnyTabs() || !AreAnySelectBoxesVisible()) return;

    const Tab& tab = GetCurrentTab();

    int minL = INT_MAX;
    int maxT = 0;
    int maxR = 0;
    int minB = INT_MAX;

    for (auto& bounds: tab.toolInfo.select.bounds) {
        if (bounds.visible) {
            int sl,st,sr,sb;
            GetOrderedSelectBounds(bounds, &sl,&st,&sr,&sb);

            minL = std::min(minL, sl);
            maxT = std::max(maxT, st);
            maxR = std::max(maxR, sr);
            minB = std::min(minB, sb);
        }
    }

    if (l) *l = minL;
    if (t) *t = maxT;
    if (r) *r = maxR;
    if (b) *b = minB;
}

TEINAPI void LoadLevelTab (std::string fileName)
{
    // If there is just one tab and it is completely empty with no changes
    // then we close this tab before opening the new level(s) in editor.
    if (gEditor.tabs.size() == 1) {
        if (IsCurrentTabEmpty() && !GetCurrentTab().unsavedChanges && GetCurrentTab().name.empty()) {
            CloseCurrentTab();
        }
    }

    size_t tabIndex = GetTabIndexWithThisFileName(fileName);
    if (tabIndex != gInvalidTab) { // This file is already open so just focus on it.
        SetCurrentTab(tabIndex);
    } else {
        CreateNewLevelTabAndFocus();
        Tab& tab = GetCurrentTab();
        tab.name = fileName;
        SetMainWindowSubtitleForTab(tab.name);
        if (!LoadLevel(tab.level, tab.name)) {
            CloseCurrentTab();
        }
    }

    NeedToScrollNextUpdate();
}

TEINAPI bool LevelEditorSave (Tab& tab)
{
    // If the current file already has a name (has been saved before) then we
    // just do a normal Save to that file. Otherwise, we perform a Save As.
    if (tab.name.empty()) {
        std::string fileName = SaveDialog(DialogType::Lvl);
        if (fileName.empty()) return false;
        tab.name = fileName;
    }

    SaveLevel(tab.level, tab.name);
    BackupLevelTab(tab.level, tab.name);

    tab.unsavedChanges = false;
    SetMainWindowSubtitleForTab(tab.name);

    return true;
}

TEINAPI bool LevelEditorSaveAs ()
{
    std::string fileName = SaveDialog(DialogType::Lvl);
    if (fileName.empty()) return false;

    Tab& tab = GetCurrentTab();

    tab.name = fileName;
    SaveLevel(tab.level, tab.name);
    BackupLevelTab(tab.level, tab.name);

    tab.unsavedChanges = false;
    SetMainWindowSubtitleForTab(tab.name);

    return true;
}

TEINAPI void LevelEditorClearSelect ()
{
    if (!CurrentTabIsLevel() || !AreAnySelectBoxesVisible()) return;

    Tab& tab = GetCurrentTab();

    NewLevelHistoryState(LevelHistoryAction::Clear);
    for (auto& bounds: tab.toolInfo.select.bounds) {
        if (bounds.visible) {
            int l,t,r,b;
            GetOrderedSelectBounds(bounds, &l,&t,&r,&b);

            // Clear all of the tiles within the selection.
            for (int i=static_cast<int>(LevelLayer::Tag); i<static_cast<int>(LevelLayer::Total); ++i) {
                for (int y=b; y<=t; ++y) {
                    for (int x=l; x<=r; ++x) {
                        Internal::PlaceMirroredTileClear(x, y, 0, static_cast<LevelLayer>(i));
                    }
                }
            }
        }
    }

    // We also deselect the select box(es) afterwards -- feels right.
    tab.levelHistory.state.back().oldSelectState = tab.toolInfo.select.bounds;
    Internal::Deselect();
    tab.levelHistory.state.back().newSelectState = tab.toolInfo.select.bounds;

    GetCurrentTab().unsavedChanges = true;
}

TEINAPI void LevelEditorDeselect ()
{
    if (!CurrentTabIsLevel()) return;
    Tab& tab = GetCurrentTab();
    tab.oldSelectState = tab.toolInfo.select.bounds;
    Internal::Deselect();
    NewLevelHistoryState(LevelHistoryAction::SelectState);
}

TEINAPI void LevelEditorSelectAll ()
{
    if (!CurrentTabIsLevel()) return;

    Tab& tab = GetCurrentTab();

    tab.oldSelectState = tab.toolInfo.select.bounds;

    tab.toolInfo.select.bounds.clear();
    tab.toolInfo.select.bounds.push_back(SelectBounds());
    tab.toolInfo.select.bounds.back().left = 0;
    tab.toolInfo.select.bounds.back().top = 0;
    tab.toolInfo.select.bounds.back().right = tab.level.header.width-1;
    tab.toolInfo.select.bounds.back().bottom = tab.level.header.height-1;
    tab.toolInfo.select.bounds.back().visible = true;

    NewLevelHistoryState(LevelHistoryAction::SelectState);
}

TEINAPI void LevelEditorCopy ()
{
    if (!CurrentTabIsLevel() || !AreAnySelectBoxesVisible()) return;
    Internal::Copy();
    LevelEditorDeselect(); // We also deselect the region afterwards, feels right.
}

TEINAPI void LevelEditorCut ()
{
    if (!CurrentTabIsLevel() || !AreAnySelectBoxesVisible()) return;
    Internal::Copy();
    LevelEditorClearSelect(); // Does the deselect for us.
    GetCurrentTab().unsavedChanges = true;
}

TEINAPI void LevelEditorPaste ()
{
    if (!CurrentTabIsLevel() || Internal::ClipboardEmpty()) return;

    Vec2 tilePos = gLevelEditor.mouseTile;
    NewLevelHistoryState(LevelHistoryAction::Normal);

    for (auto& clipboard: gLevelEditor.clipboard) {
        int x = static_cast<int>(tilePos.x) + clipboard.x;
        int y = static_cast<int>(tilePos.y) + clipboard.y;
        int w = clipboard.w;
        int h = clipboard.h;

        // Paste all of the clipboard tiles.
        for (size_t i=0; i<clipboard.data.size(); ++i) {
            const auto& srcLayer = clipboard.data[i];
            for (int iy=y; iy<(y+h); ++iy) {
                for (int ix=x; ix<(x+w); ++ix) {
                    Internal::PlaceMirroredTile(ix, iy, srcLayer[(iy-y)*w+(ix-x)], static_cast<LevelLayer>(i));
                }
            }
        }
    }

    GetCurrentTab().unsavedChanges = true;
}

TEINAPI void FlipLevelH ()
{
    // If all layers are inactive then there is no point in doing the flip.
    if (AreAllLayersInactive()) return;

    const Tab& tab = GetCurrentTab();

    int lw = tab.level.header.width;
    int lh = tab.level.header.height;

    // If the active layers in flip bounds are empty there is no point.
    if (Internal::AreActiveLayersInBoundsEmpty(0,0,lw,lh)) return;

    // We deselect as this feels correct.
    Internal::Deselect();

    NewLevelHistoryState(LevelHistoryAction::FlipLevelH);
    Internal::FlipLevelH(tab.tileLayerActive);
}
TEINAPI void FlipLevelV ()
{
    // If all layers are inactive then there is no point in doing the flip.
    if (AreAllLayersInactive()) return;

    const Tab& tab = GetCurrentTab();

    int lw = tab.level.header.width;
    int lh = tab.level.header.height;

    // If the active layers in flip bounds are empty there is no point.
    if (Internal::AreActiveLayersInBoundsEmpty(0,0,lw,lh)) return;

    // We deselect as this feels correct.
    Internal::Deselect();

    NewLevelHistoryState(LevelHistoryAction::FlipLevelV);
    Internal::FlipLevelV(tab.tileLayerActive);
}

TEINAPI void LevelHasUnsavedChanges ()
{
    GetCurrentTab().unsavedChanges = true;
}

TEINAPI void LevelEditorUndo ()
{
    Tab& tab = GetCurrentTab();

    // There is no history or we are already at the beginning.
    if (tab.levelHistory.currentPosition <= -1) return;

    bool normalStateEmpty = false;

    LevelHistoryState& state = Internal::GetCurrentHistoryState();
    switch (state.action) {
        case (LevelHistoryAction::Resize): {
            Internal::Resize(state.resizeDir, state.oldWidth, state.oldHeight);
            tab.level.data = state.oldData;
        } break;
        case (LevelHistoryAction::SelectState): {
            Internal::RestoreSelectState(state.oldSelectState);
        } break;
        case (LevelHistoryAction::FlipLevelH): {
            Internal::FlipLevelH(state.tileLayerActive);
        } break;
        case (LevelHistoryAction::FlipLevelV): {
            Internal::FlipLevelV(state.tileLayerActive);
        } break;
        case (LevelHistoryAction::Normal):
        case (LevelHistoryAction::Clear):
        {
            // We check if the normal state we're undoing is empty or not. If it is
            // then we mark it as such and then if there is another state before it
            // we undo that one as well. This just feels a nicer than not doing it.
            if (state.info.empty()) normalStateEmpty = true;
            for (auto& i: state.info) {
                int pos = i.y * tab.level.header.width + i.x;
                tab.level.data[static_cast<int>(i.tileLayer)][pos] = i.oldID;
            }
            if (state.action == LevelHistoryAction::Clear) {
                Internal::RestoreSelectState(state.oldSelectState);
            }
        } break;
    }

    if (tab.levelHistory.currentPosition > -1) {
        --tab.levelHistory.currentPosition;
        // We only want to do this part if there is another state to undo.
        if (state.action == LevelHistoryAction::Normal && normalStateEmpty) {
            LevelEditorUndo();
        }
    }

    if (state.action != LevelHistoryAction::SelectState) {
        tab.unsavedChanges = true;
    }
}

TEINAPI void LevelEditorRedo ()
{
    Tab& tab = GetCurrentTab();

    // There is no history or we are already at the end.
    if (tab.levelHistory.currentPosition >= static_cast<int>(tab.levelHistory.state.size())-1) return;

    ++tab.levelHistory.currentPosition;

    LevelHistoryState& state = Internal::GetCurrentHistoryState();
    switch (state.action) {
        case (LevelHistoryAction::Resize): {
            Internal::Resize(state.resizeDir, state.newWidth, state.newHeight);
            tab.level.data = state.newData;
        } break;
        case (LevelHistoryAction::SelectState): {
            Internal::RestoreSelectState(state.newSelectState);
        } break;
        case (LevelHistoryAction::FlipLevelH): {
            Internal::FlipLevelH(state.tileLayerActive);
        } break;
        case (LevelHistoryAction::FlipLevelV): {
            Internal::FlipLevelV(state.tileLayerActive);
        } break;
        case (LevelHistoryAction::Normal):
        case (LevelHistoryAction::Clear): {
            for (auto& i: state.info) {
                int pos = i.y * tab.level.header.width + i.x;
                tab.level.data[static_cast<int>(i.tileLayer)][pos] = i.newID;
            }
            if (state.action == LevelHistoryAction::Clear) {
                Internal::RestoreSelectState(state.newSelectState);
            }
        } break;
    }

    // If we end on an empty normal state and we are not already at the end of
    // the redo history then we redo again as it feels nicer. This action is
    // the inverse of what we do when we do an undo with blank normal actions.
    if (tab.levelHistory.currentPosition+1 < tab.levelHistory.state.size()) {
        // Jump forward to see if it is empty, if it's not then revert back.
        ++tab.levelHistory.currentPosition;
        LevelHistoryState& nextState = Internal::GetCurrentHistoryState();

        if (nextState.action != LevelHistoryAction::Normal || !nextState.info.empty()) {
            --tab.levelHistory.currentPosition;
        }
    }

    if (state.action != LevelHistoryAction::SelectState) {
        tab.unsavedChanges = true;
    }
}

TEINAPI void LevelEditorHistoryBegin ()
{
    Tab& tab = GetCurrentTab();
    while (tab.levelHistory.currentPosition > -1) LevelEditorUndo();
    tab.unsavedChanges = true;
}
TEINAPI void LevelEditorHistoryEnd ()
{
    Tab& tab = GetCurrentTab();
    int maximum = static_cast<int>(tab.levelHistory.state.size()-1);
    while (tab.levelHistory.currentPosition < maximum) LevelEditorRedo();
    tab.unsavedChanges = true;
}

TEINAPI void LevelEditorResize ()
{
    if (!CurrentTabIsLevel()) return;
    const Tab& tab = GetCurrentTab();
    OpenResize(tab.level.header.width, tab.level.header.height);
}
TEINAPI void LevelEditorResizeOkay ()
{
    Tab& tab = GetCurrentTab();

    int lw = tab.level.header.width;
    int lh = tab.level.header.height;

    int nw = GetResizeWidth();
    int nh = GetResizeHeight();

    int dx = nw - lw;
    int dy = nh - lh;

    // Return early to avoid making a history state for no reason.
    if (dx == 0 && dy == 0) return;

    NewLevelHistoryState(LevelHistoryAction::Resize);
    Internal::GetCurrentHistoryState().oldData = tab.level.data;
    Internal::Resize(GetResizeDir(), nw, nh);
    Internal::GetCurrentHistoryState().newData = tab.level.data;
}

TEINAPI void LevelEditorLoadPrevLevel ()
{
    if (!CurrentTabIsLevel()) return;

    Tab& tab = GetCurrentTab();

    std::string path(StripFileName(tab.name));

    std::vector<std::string> files;
    ListPathFiles(path, files);
    if (files.size() <= 1) return;

    files.erase(std::remove_if(files.begin(), files.end(),
    [](const std::string& s) {
        return s.substr(s.find_last_of(".")) != ".lvl";
    }), files.end());

    // We strip the extensions then sort the files as they can interfere with
    // getting a good alphabetical sort on the strings. We add them back later.
    for (auto& f: files) f = StripFileExt(f);
    std::sort(files.begin(), files.end());

    // Find our current location and move on to the prev file.
    std::string current(StripFileExt(tab.name));
    auto iter = std::find(files.begin(), files.end(), current);
    if (iter == files.end()) { assert(false); return; } // Shouldn't happen...

    std::string prev;
    if (iter == files.begin()) prev = *(files.end()-1);
    else prev = *(iter-1);

    prev += ".lvl";

    if (SaveChangesPrompt(tab) == AlertResult::Cancel) return;
    tab.unsavedChanges = false;

    // Finally, we can load the prev level as the current tab.
    tab.name = prev;
    SetMainWindowSubtitleForTab(tab.name);

    if (!LoadLevel(tab.level, tab.name)) CloseCurrentTab();
}

TEINAPI void LevelEditorLoadNextLevel ()
{
    if (!CurrentTabIsLevel()) return;

    Tab& tab = GetCurrentTab();

    std::string path(StripFileName(tab.name));

    std::vector<std::string> files;
    ListPathFiles(path, files);
    if (files.size() <= 1) return;

    files.erase(std::remove_if(files.begin(), files.end(),
    [](const std::string& s) {
        return s.substr(s.find_last_of(".")) != ".lvl";
    }), files.end());

    // We strip the extensions then sort the files as they can interfere with
    // getting a good alphabetical sort on the strings. We add them back later.
    for (auto& f: files) {
        std::string ext(f.substr(f.find_last_of(".")));
        f = StripFileExt(f);
    }
    std::sort(files.begin(), files.end());

    // Find our current location and move on to the next file.
    std::string current(StripFileExt(tab.name));
    auto iter = std::find(files.begin(), files.end(), current);
    if (iter == files.end()) { assert(false); return; } // Shouldn't happen...

    std::string next;
    if (iter+1 != files.end()) next = *(iter+1);
    else next = *files.begin();

    next += ".lvl";

    if (SaveChangesPrompt(tab) == AlertResult::Cancel) return;
    tab.unsavedChanges = false;

    // Finally, we can load the next level as the current tab.
    tab.name = next;
    SetMainWindowSubtitleForTab(tab.name);

    if (!LoadLevel(tab.level, tab.name)) CloseCurrentTab();
}

TEINAPI void LevelDropFile (Tab* tab, std::string fileName)
{
    fileName = FixPathSlashes(fileName);

    // If there is just one tab and it is completely empty with no changes
    // then we close this tab before opening the new level(s) in editor.
    if (gEditor.tabs.size() == 1) {
        if (IsCurrentTabEmpty() && !GetCurrentTab().unsavedChanges && GetCurrentTab().name.empty()) {
            CloseCurrentTab();
        }
    }

    size_t tabIndex = GetTabIndexWithThisFileName(fileName);
    if (tabIndex != gInvalidTab) { // This file is already open so just focus on it.
        SetCurrentTab(tabIndex);
    } else {
        CreateNewLevelTabAndFocus();
        tab = &GetCurrentTab();
        tab->name = fileName;
        SetMainWindowSubtitleForTab(tab->name);
        if (!LoadLevel(tab->level, tab->name)) {
            CloseCurrentTab();
        }
    }

    NeedToScrollNextUpdate();
}

TEINAPI void BackupLevelTab (const Level& level, const std::string& fileName)
{
    // Determine how many backups the user wants saved for a given level.
    int backupCount = gEditorSettings.backupCount;
    if (backupCount <= 0) return; // No backups are wanted!

    std::string levelName((fileName.empty()) ? "untitled" : StripFilePathAndExt(fileName));

    // Create a folder for this particular level's backups if it does not exist.
    // We make separate sub-folders in the backup directory for each level as
    // there was an issue in older versions with the editor freezing when backing
    // up levels to a backups folder with loads of saves. This was because the
    // editor was searching the folder for old backups (leading to a freeze).
    std::string backupPath(MakePathAbsolute("backups/" + levelName + "/"));
    if (!DoesPathExist(backupPath)) {
        if (!CreatePath(backupPath)) {
            LogError(ErrorLevel::Min, "Failed to create backup for level \"%s\"!", levelName.c_str());
            return;
        }
    }

    // Determine how many backups are already saved of this level.
    std::vector<std::string> backups;
    ListPathContent(backupPath, backups);

    int levelCount = 0;
    for (auto& file: backups) {
        if (IsFile(file)) {
            // We strip extension twice because there are two extension parts to backups the .bak and the .lvl.
            std::string compareName(StripFileExt(StripFilePathAndExt(file)));
            if (InsensitiveCompare(levelName, compareName)) ++levelCount;
        }
    }

    // If there is still room to create a new backup then that is what
    // we do. Otherwise, we overwrite the oldest backup of the level.
    std::string backupName = backupPath + levelName + ".bak";
    if (gEditorSettings.unlimitedBackups || (levelCount < backupCount)) {
        backupName += std::to_string(levelCount) + ".lvl";
        SaveLevel(level, backupName);
    } else {
        U64 oldest = UINT64_MAX;
        int oldestIndex = 0;

        for (int i=0; i<levelCount; ++i) {
            std::string name(backupName + std::to_string(i) + ".lvl");
            U64 current = LastFileWriteTime(name);
            if (CompareFileWriteTimes(current, oldest) == -1) {
                oldest = current;
                oldestIndex = i;
            }
        }

        backupName += std::to_string(oldestIndex) + ".lvl";
        SaveLevel(level, backupName);
    }
}

TEINAPI bool IsCurrentLevelEmpty ()
{
    if (AreThereAnyLevelTabs()) {
        const Tab& tab = GetCurrentTab();
        if (tab.type == TabType::Level) {
            for (const auto& layer: tab.level.data) {
                for (auto id: layer) if (id != 0) return false;
            }
            return true;
        }
    }
    return false;
}
