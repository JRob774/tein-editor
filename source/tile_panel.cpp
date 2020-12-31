static constexpr const char* gTileDataFile = "data/editor_tiles.txt";
static constexpr const char* gFlipDataFile = "data/editor_flips.txt";

static constexpr float gTilePanelItemPad = 4;
static constexpr float gTilePanelItemSize = 20;
static constexpr float gTilePanelColumns = 11;
static constexpr float gTilePanelInnerPad = 3;
static constexpr float gTilePanelLabelHeight = 20;
static constexpr float gTilePanelInactiveAlpha = .33f;

typedef std::pair<TileID,TileID> TileFlipMap;

struct TileGroup
{
    std::string name;
    std::string desc;
    std::vector<TileID> tile;
    int selectedIndex;
};

struct TilePanel
{
    std::vector<TileFlipMap> flipMapHorz;
    std::vector<TileFlipMap> flipMapVert;

    std::map<TileCategory,std::vector<TileGroup>> category;

    Quad bounds;

    int selectedCategory;
    int selectedGroup;

    bool mouseDown;

    float contentHeight;
    float scrollOffset;
};

static TilePanel gTilePanel;

namespace Internal
{
    TEINAPI bool IsCategoryActive (TileCategory category)
    {
        if (!CurrentTabIsLevel()) return false;
        const Tab& tab = GetCurrentTab();
        return tab.tileLayerActive[CategoryToLayer(category)];
    }

    TEINAPI float DoTilePanelCategory (Vec2& cursor, TileCategory categoryIndex, std::vector<TileGroup>& category)
    {
        // Determine how many rows of entities are present in the category.
        float items = static_cast<float>(category.size());
        float rows = ceilf(items / gTilePanelColumns);

        float totalItemPad = (rows - 1) * gTilePanelItemPad;
        float categoryPad = gTilePanelInnerPad * 2;

        // Do a separate panel for the category's label before the main panel.
        float x = gTilePanelInnerPad;
        float y = cursor.y;
        float w = (gControlPanelWidth - categoryPad);
        float h = gTilePanelLabelHeight;

        bool isActive = IsCategoryActive(categoryIndex);

        Vec2 labelCursor(0,0);
        BeginPanel(x,y,w,h, (isActive) ? UI_NONE : UI_INACTIVE);

        SetPanelCursor(&labelCursor);
        SetPanelCursorDir(UI_DIR_DOWN);

        const char* labelName = NULL;
        switch (categoryIndex)
        {
            case (TILE_CATEGORY_BASIC  ): labelName = "Basic";   break;
            case (TILE_CATEGORY_TAG    ): labelName = "Tag";     break;
            case (TILE_CATEGORY_OVERLAY): labelName = "Overlay"; break;
            case (TILE_CATEGORY_ENTITY ): labelName = "Entity";  break;
            case (TILE_CATEGORY_BACK1  ): labelName = "Back 1";  break;
            case (TILE_CATEGORY_BACK2  ): labelName = "Back 2";  break;
        }

        DoLabel(UI_ALIGN_LEFT,UI_ALIGN_CENTER, w, gTilePanelLabelHeight, labelName);
        EndPanel();

        cursor.y += gTilePanelLabelHeight;

        // Bounding box for the particular category in the panel.
        x = gTilePanelInnerPad;
        y = cursor.y;
        w = (gControlPanelWidth - categoryPad);
        h = (rows * gTilePanelItemSize) + categoryPad + totalItemPad;

        Vec2 cur(gTilePanelInnerPad, gTilePanelInnerPad);
        BeginPanel(x,y,w,h, UI_NONE, gUiColorMedDark);

        SetDrawColor(gUiColorExDark); // The outline/border for the category.
        DrawQuad(0, GetPanelOffset().y, w, GetPanelOffset().y+h);

        TextureAtlas& atlas = GetEditorAtlasSmall();

        SetTileBatchTexture(atlas.texture);
        SetTileBatchColor((isActive) ? Vec4(1,1,1,1) : Vec4(1,1,1,gTilePanelInactiveAlpha));

        for (size_t i=0; i<category.size(); ++i)
        {
            Vec2 tileCursor = cur;
            tileCursor.y += GetPanelOffset().y;

            const TileGroup& tileGroup = category[i];
            Quad tileGroupBounds = { tileCursor.x, tileCursor.y, gTilePanelItemSize, gTilePanelItemSize };

            if (isActive)
            {
                bool inBounds = MouseInUiBoundsXYWH(tileGroupBounds);
                if (inBounds)
                {
                    push_status_bar_message("Tile Group: %s", tileGroup.name.c_str());
                    SetCurrentTooltip(tileGroup.name, tileGroup.desc);
                }
                if (gTilePanel.mouseDown && !IsThereAHitUiElement())
                {
                    if (inBounds)
                    {
                        gTilePanel.selectedCategory = categoryIndex;
                        gTilePanel.selectedGroup = static_cast<int>(i);
                    }
                }
            }

            if (CurrentTabIsLevel() && !AreAllLayersInactive())
            {
                float qx = tileCursor.x - 1;
                float qy = tileCursor.y - 1;
                float qw = gTilePanelItemSize + 2;
                float qh = gTilePanelItemSize + 2;

                // If we are the currently selected group then we draw the highlight.
                if ((categoryIndex == gTilePanel.selectedCategory) && (i == gTilePanel.selectedGroup))
                {
                    SetDrawColor(gUiColorLight);
                    FillQuad(qx,qy,qx+qw,qy+qh);
                }
            }

            // We scissor the contents to avoid overspill.
            BeginScissor(tileCursor.x, tileCursor.y, gTilePanelItemSize, gTilePanelItemSize);

            float ex = tileCursor.x + (gTilePanelItemSize/2);
            float ey = tileCursor.y + (gTilePanelItemSize/2);

            TileID selectedID = tileGroup.tile[tileGroup.selectedIndex];
            DrawBatchedTile(ex, ey, &GetAtlasClip(atlas, selectedID));

            EndScissor();

            // Advance the cursor for the next entity's placement.
            float advance = gTilePanelItemSize + gTilePanelItemPad;
            cur.x += advance;
            if (cur.x + gTilePanelItemSize > w)
            {
                cur.x = gTilePanelInnerPad;
                cur.y += advance;
            }
        }

        FlushBatchedTiles();

        EndPanel();

        // Set the cursor for the next category (if there is one).
        cursor.x = gTilePanelInnerPad;
        cursor.y += h;

        // Return the category height for use with the scrollbar.
        return h;
    }

    TEINAPI void LoadFlipData (const GonObject& data, std::vector<TileFlipMap>& flip)
    {
        for (int i=0; i<static_cast<int>(data.children_array.size()); ++i)
        {
            flip.push_back({ data[i][0].Int(), data[i][1].Int() });
        }
    }

    TEINAPI float CalculateTilePanelHeight ()
    {
        float height = gTilePanelInnerPad;
        for (auto [category,groups]: gTilePanel.category)
        {
            float items = static_cast<float>(groups.size());
            float rows = ceilf(items / gTilePanelColumns);
            float totalItemPad = (rows-1) * gTilePanelItemPad;
            float categoryPad = (gTilePanelInnerPad*2);
            height += ((rows * gTilePanelItemSize) + categoryPad + totalItemPad) + gTilePanelLabelHeight;
        }
        return height;
    }

    TEINAPI void SetCategoryAsActive (TileCategory category)
    {
        int oldSelectedCategory = gTilePanel.selectedCategory;
        int oldSelectedGroup = gTilePanel.selectedGroup;

        gTilePanel.selectedCategory = category;
        gTilePanel.selectedGroup = 0;

        // Make sure we are actually jumping to an active category.
        if (!IsCategoryActive(static_cast<TileCategory>(gTilePanel.selectedCategory)))
        {
            SelectNextActiveGroup();
        }

        if (oldSelectedGroup != gTilePanel.selectedCategory || oldSelectedGroup != gTilePanel.selectedGroup)
        {
            // When the selected gets changed then we make a new state.
            if (gLevelEditor.toolType == ToolType::BRUSH || gLevelEditor.toolType == ToolType::FILL)
            {
                if (gLevelEditor.toolState != ToolState::IDLE)
                {
                    NewLevelHistoryState(LevelHistoryAction::NORMAL);
                }
            }
        }
    }

    TEINAPI void JumpToCategory (TileCategory category)
    {
        if (CurrentTabIsLevel())
        {
            if (!AreAllLayersInactive())
            {
                if (IsWindowFocused("WINMAIN"))
                {
                    SetCategoryAsActive(category);
                }
            }
        }
    }
}

TEINAPI bool InitTilePanel ()
{
    gTilePanel.flipMapHorz.clear();
    gTilePanel.flipMapVert.clear();

    gTilePanel.category.clear();

    try
    {
        // Load all of the tile groups and sort them into their categories.
        GonObject tileGonData = GonObject::LoadFromBuffer(LoadStringResource(gTileDataFile))["tiles"];

        for (auto [name,index]: tileGonData.children_map)
        {
            const GonObject& categoryData = tileGonData.children_array[index];
            if (std::stoi(name) < TILE_CATEGORY_TOTAL)
            {
                TileCategory categoryID = static_cast<TileCategory>(std::stoi(name));
                for (auto& tileGroupGonData: categoryData.children_array)
                {
                    TileGroup group;
                    group.selectedIndex = 0;
                    group.name = tileGroupGonData["name"].String();
                    group.desc = tileGroupGonData["tooltip"].String();
                    for (int i=0; i<tileGroupGonData["id"].size(); ++i)
                    {
                        TileID tileID = tileGroupGonData["id"][i].Int();
                        group.tile.push_back(tileID);
                    }
                    gTilePanel.category[categoryID].push_back(group);
                }
            }
        }

        // Load flip mappings between the tile IDs for smart level flipping.
        GonObject flipGonData = GonObject::LoadFromBuffer(LoadStringResource(gFlipDataFile))["flip"];

        Internal::LoadFlipData(flipGonData["horz"], gTilePanel.flipMapHorz);
        Internal::LoadFlipData(flipGonData["vert"], gTilePanel.flipMapVert);
    }
    catch (const char* msg)
    {
        LogError(ERR_MED, "%s", msg);
        return false;
    }

    // The starting selected is always the first entry in the first category.
    gTilePanel.selectedCategory = TILE_CATEGORY_BASIC;
    gTilePanel.selectedGroup = 0;

    // We need to calculate the height of all the tiles panel content so that
    // we can use this height value during the panel do_scrollbar operation.
    gTilePanel.contentHeight = Internal::CalculateTilePanelHeight();
    gTilePanel.scrollOffset = 0;

    return true;
}

TEINAPI void DoTilePanel (bool scrollbar)
{
    SetUiFont(&GetEditorRegularFont());

    Vec2 cursor(gTilePanelInnerPad, 0);
    gTilePanel.bounds = { 0,0,GetPanelWidth(),GetPanelHeight() };
    if (IsLayerPanelPresent())
    {
        gTilePanel.bounds.h = roundf(gTilePanel.bounds.h - GetLayerPanelHeight());
    }

    BeginPanel(gTilePanel.bounds, UI_NONE, gUiColorMedium);

    if (scrollbar)
    {
        float x = gControlPanelWidth;
        float y = gControlPanelInnerPad;
        float w = gControlPanelScrollbarWidth - gControlPanelInnerPad;
        float h = GetViewport().h - (gControlPanelInnerPad * 2);

        DoScrollbar(x,y,w,h, gTilePanel.contentHeight, gTilePanel.scrollOffset);
    }

    for (auto [categoryIndex,category]: gTilePanel.category)
    {
        Internal::DoTilePanelCategory(cursor, categoryIndex, category);
    }

    EndPanel();
}

TEINAPI bool TilePanelNeedsScrollbar ()
{
    return (gTilePanel.contentHeight > gTilePanel.bounds.h);
}

TEINAPI void HandleTilePanelEvents ()
{
    if (CurrentTabIsLevel())
    {
        if (!AreAllLayersInactive())
        {
            if (IsWindowFocused("WINMAIN"))
            {
                switch (main_event.type)
                {
                    case (SDL_MOUSEBUTTONDOWN):
                    case (SDL_MOUSEBUTTONUP):
                    {
                        if (main_event.button.button == SDL_BUTTON_LEFT)
                        {
                            gTilePanel.mouseDown = (main_event.button.state == SDL_PRESSED);
                        }
                    } break;
                    case (SDL_MOUSEWHEEL):
                    {
                        if (!MouseIsOverTabBar())
                        {
                            if (IsKeyModStateActive(0))
                            {
                                if (main_event.wheel.y > 0) IncrementSelectedTile();
                                else if (main_event.wheel.y < 0) DecrementSelectedTile();
                            }
                            else if (IsKeyModStateActive(KMOD_ALT))
                            {
                                if (main_event.wheel.y > 0) IncrementSelectedGroup();
                                else if (main_event.wheel.y < 0) DecrementSelectedGroup();
                            }
                            else if (IsKeyModStateActive(KMOD_ALT|KMOD_SHIFT))
                            {
                                if (main_event.wheel.y > 0) IncrementSelectedCategory();
                                else if (main_event.wheel.y < 0) DecrementSelectedCategory();
                            }
                        }
                    } break;
                }
            }
        }
    }
}

TEINAPI float GetTilePanelHeight ()
{
    return (gTilePanel.bounds.h + 1);
}

TEINAPI void ReloadTileGraphics ()
{
    FreeTextureAtlas(gResourceLarge);
    FreeTextureAtlas(gResourceSmall);

    if (gEditorSettings.tileGraphics == "new")
    {
        LoadAtlasResource("textures/editor_icons/new_large.txt", gResourceLarge);
        LoadAtlasResource("textures/editor_icons/new_small.txt", gResourceSmall);
    }
    else
    {
        LoadAtlasResource("textures/editor_icons/old_large.txt", gResourceLarge);
        LoadAtlasResource("textures/editor_icons/old_small.txt", gResourceSmall);
    }
}

TEINAPI TileCategory GetSelectedCategory ()
{
    return static_cast<TileCategory>(gTilePanel.selectedCategory);
}

TEINAPI TileID GetSelectedTile ()
{
    const auto& category = gTilePanel.category[static_cast<TileCategory>(gTilePanel.selectedCategory)];
    const auto& group = category[gTilePanel.selectedGroup];
    return group.tile[group.selectedIndex];
}

TEINAPI LevelLayer GetSelectedLayer ()
{
    return CategoryToLayer(static_cast<TileCategory>(gTilePanel.selectedCategory));
}

TEINAPI LevelLayer CategoryToLayer (TileCategory category)
{
    switch (category)
    {
        case (TILE_CATEGORY_BASIC  ): return LEVEL_LAYER_ACTIVE;  break;
        case (TILE_CATEGORY_TAG    ): return LEVEL_LAYER_TAG;     break;
        case (TILE_CATEGORY_OVERLAY): return LEVEL_LAYER_OVERLAY; break;
        case (TILE_CATEGORY_ENTITY ): return LEVEL_LAYER_ACTIVE;  break;
        case (TILE_CATEGORY_BACK1  ): return LEVEL_LAYER_BACK1;   break;
        case (TILE_CATEGORY_BACK2  ): return LEVEL_LAYER_BACK2;   break;
    }
    return LEVEL_LAYER_ACTIVE;
}

TEINAPI void SelectNextActiveGroup ()
{
    if (!AreAllLayersInactive())
    {
        while (!Internal::IsCategoryActive(static_cast<TileCategory>(gTilePanel.selectedCategory)))
        {
            ++gTilePanel.selectedCategory;
            if (gTilePanel.selectedCategory > TILE_CATEGORY_BACK2)
            {
                gTilePanel.selectedCategory = TILE_CATEGORY_BASIC;
            }
            gTilePanel.selectedGroup = 0;
        }
    }
}
TEINAPI void SelectPrevActiveGroup ()
{
    if (!AreAllLayersInactive())
    {
        while (!Internal::IsCategoryActive(static_cast<TileCategory>(gTilePanel.selectedCategory)))
        {
            --gTilePanel.selectedCategory;
            if (gTilePanel.selectedCategory < TILE_CATEGORY_BASIC)
            {
                gTilePanel.selectedCategory = TILE_CATEGORY_BACK2;
            }
            gTilePanel.selectedGroup = 0;
        }
    }
}

TEINAPI void ResetSelectedGroup ()
{
    gTilePanel.selectedCategory = 0;
    SelectNextActiveGroup();
}

TEINAPI void IncrementSelectedTile ()
{
    if (!CurrentTabIsLevel()) return;

    if (!AreAllLayersInactive())
    {
        auto& group = gTilePanel.category[static_cast<TileCategory>(gTilePanel.selectedCategory)][gTilePanel.selectedGroup];
        int oldSelectedIndex = group.selectedIndex;
        if ((++group.selectedIndex) > group.tile.size()-1)
        {
            group.selectedIndex = 0;
        }
        // Feels correct that a new history state should be made.
        if (oldSelectedIndex != group.selectedIndex)
        {
            if (gLevelEditor.toolType == ToolType::BRUSH || gLevelEditor.toolType == ToolType::FILL)
            {
                if (gLevelEditor.toolState != ToolState::IDLE)
                {
                    NewLevelHistoryState(LevelHistoryAction::NORMAL);
                }
            }
        }
    }
}
TEINAPI void DecrementSelectedTile ()
{
    if (!CurrentTabIsLevel()) return;

    if (!AreAllLayersInactive())
    {
        auto& group = gTilePanel.category[static_cast<TileCategory>(gTilePanel.selectedCategory)][gTilePanel.selectedGroup];
        int oldSelectedIndex = group.selectedIndex;
        if ((--group.selectedIndex) < 0)
        {
            group.selectedIndex = static_cast<int>(group.tile.size()-1);
        }
        // Feels correct that a new history state should be made.
        if (oldSelectedIndex != group.selectedIndex)
        {
            if (gLevelEditor.toolType == ToolType::BRUSH || gLevelEditor.toolType == ToolType::FILL)
            {
                if (gLevelEditor.toolState != ToolState::IDLE)
                {
                    NewLevelHistoryState(LevelHistoryAction::NORMAL);
                }
            }
        }
    }
}

TEINAPI void IncrementSelectedGroup ()
{
    if (!CurrentTabIsLevel()) return;

    if (!AreAllLayersInactive())
    {
        auto& category = gTilePanel.category[static_cast<TileCategory>(gTilePanel.selectedCategory)];
        int oldSelectedGroup = gTilePanel.selectedGroup;
        if ((++gTilePanel.selectedGroup) > category.size()-1)
        {
            gTilePanel.selectedGroup = 0;
        }
        // Feels correct that a new history state should be made.
        if (oldSelectedGroup != gTilePanel.selectedGroup)
        {
            if (gLevelEditor.toolType == ToolType::BRUSH || gLevelEditor.toolType == ToolType::FILL)
            {
                if (gLevelEditor.toolState != ToolState::IDLE)
                {
                    NewLevelHistoryState(LevelHistoryAction::NORMAL);
                }
            }
        }
    }
}
TEINAPI void DecrementSelectedGroup ()
{
    if (!CurrentTabIsLevel()) return;

    if (!AreAllLayersInactive())
    {
        auto& category = gTilePanel.category[static_cast<TileCategory>(gTilePanel.selectedCategory)];
        int oldSelectedGroup = gTilePanel.selectedGroup;
        if ((--gTilePanel.selectedGroup) < 0)
        {
            gTilePanel.selectedGroup = static_cast<int>(category.size()-1);
        }
        // Feels correct that a new history state should be made.
        if (oldSelectedGroup != gTilePanel.selectedGroup)
        {
            if (gLevelEditor.toolType == ToolType::BRUSH || gLevelEditor.toolType == ToolType::FILL)
            {
                if (gLevelEditor.toolState != ToolState::IDLE)
                {
                    NewLevelHistoryState(LevelHistoryAction::NORMAL);
                }
            }
        }
    }
}

TEINAPI void IncrementSelectedCategory ()
{
    if (!CurrentTabIsLevel()) return;

    int oldSelectedCategory = gTilePanel.selectedCategory;
    int oldSelectedGroup = gTilePanel.selectedGroup;

    if (!AreAllLayersInactive())
    {
        if ((++gTilePanel.selectedCategory) > TILE_CATEGORY_BACK2)
        {
            gTilePanel.selectedCategory = TILE_CATEGORY_BASIC;
        }
        gTilePanel.selectedGroup = 0;
        SelectNextActiveGroup();
        // Feels correct that a new history state should be made.
        if (oldSelectedCategory != gTilePanel.selectedCategory || oldSelectedGroup != gTilePanel.selectedGroup)
        {
            if (gLevelEditor.toolType == ToolType::BRUSH || gLevelEditor.toolType == ToolType::FILL)
            {
                if (gLevelEditor.toolState != ToolState::IDLE)
                {
                    NewLevelHistoryState(LevelHistoryAction::NORMAL);
                }
            }
        }
    }
}
TEINAPI void DecrementSelectedCategory ()
{
    if (!CurrentTabIsLevel()) return;

    int oldSelectedCategory = gTilePanel.selectedCategory;
    int oldSelectedGroup = gTilePanel.selectedGroup;

    if (!AreAllLayersInactive())
    {
        if ((--gTilePanel.selectedCategory) < TILE_CATEGORY_BASIC)
        {
            gTilePanel.selectedCategory = TILE_CATEGORY_BACK2;
        }
        gTilePanel.selectedGroup = 0;
        SelectPrevActiveGroup();
        // Feels correct that a new history state should be made.
        if (oldSelectedCategory != gTilePanel.selectedCategory || oldSelectedGroup != gTilePanel.selectedGroup)
        {
            if (gLevelEditor.toolType == ToolType::BRUSH || gLevelEditor.toolType == ToolType::FILL)
            {
                if (gLevelEditor.toolState != ToolState::IDLE)
                {
                    NewLevelHistoryState(LevelHistoryAction::NORMAL);
                }
            }
        }
    }
}

TEINAPI TileID GetTileHorizontalFlip (TileID id)
{
    if (id)
    {
        for (const auto& pair: gTilePanel.flipMapHorz)
        {
            if (id == pair.first) return pair.second;
            if (id == pair.second) return pair.first;
        }
    }
    return id;
}
TEINAPI TileID GetTileVerticalFlip (TileID id)
{
    if (id)
    {
        for (const auto& pair: gTilePanel.flipMapVert)
        {
            if (id == pair.first) return pair.second;
            if (id == pair.second) return pair.first;
        }
    }
    return id;
}

TEINAPI void JumpToCategoryBasic ()
{
    Internal::JumpToCategory(TILE_CATEGORY_BASIC);
}
TEINAPI void JumpToCategoryTag ()
{
    Internal::JumpToCategory(TILE_CATEGORY_TAG);
}
TEINAPI void JumpToCategoryOverlay ()
{
    Internal::JumpToCategory(TILE_CATEGORY_OVERLAY);
}
TEINAPI void JumpToCategoryEntity ()
{
    Internal::JumpToCategory(TILE_CATEGORY_ENTITY);
}
TEINAPI void JumpToCategoryBack1 ()
{
    Internal::JumpToCategory(TILE_CATEGORY_BACK1);
}
TEINAPI void JumpToCategoryBack2 ()
{
    Internal::JumpToCategory(TILE_CATEGORY_BACK2);
}
