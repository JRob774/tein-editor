/*******************************************************************************
 * Editor GUI widget containing a palette of all the game's tiles.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr const char* TILE_DATA_FILE = "data/editor_tiles.txt";
static constexpr const char* FLIP_DATA_FILE = "data/editor_flips.txt";

static constexpr float TILE_PANEL_ITEM_PAD   =    4;
static constexpr float TILE_PANEL_ITEM_SIZE  =   20;
static constexpr float TILE_PANEL_COLUMNS    =   11;
static constexpr float TILE_PANEL_INNER_PAD  =    3;
static constexpr float TILE_PANEL_LABEL_H    =   20;
static constexpr float TILE_PANEL_INACTIVE_A = .33f;

typedef std::pair<Tile_ID, Tile_ID> Tile_Flip_Map;

struct Tile_Group
{
    std::string              name;
    std::string              desc;
    std::vector<Tile_ID>     tile;
    int            selected_index;
};

struct Tile_Panel
{
    std::vector<Tile_Flip_Map> flip_map_horz;
    std::vector<Tile_Flip_Map> flip_map_vert;

    std::map<Tile_Category, std::vector<Tile_Group>> category;

    Quad bounds;

    int selected_category;
    int selected_group;

    bool mouse_down;

    float content_height;
    float scroll_offset;
};

static Tile_Panel tile_panel;

/* -------------------------------------------------------------------------- */

TEINAPI bool internal__is_category_active (Tile_Category category)
{
    if (!current_tab_is_level()) return false;
    const Tab& tab = get_current_tab();
    return tab.tile_layer_active[category_to_layer(category)];
}

TEINAPI float internal__do_tile_panel_category (Vec2& cursor, Tile_Category category_index, std::vector<Tile_Group>& category)
{
    // Determine how many rows of entities are present in the category.
    float items = static_cast<float>(category.size());
    float rows = ceilf(items / TILE_PANEL_COLUMNS);

    float total_item_pad = (rows - 1) * TILE_PANEL_ITEM_PAD;
    float category_pad = TILE_PANEL_INNER_PAD * 2;

    // Do a separate panel for the category's label before the main panel.
    float x = TILE_PANEL_INNER_PAD;
    float y = cursor.y;
    float w = (CONTROL_PANEL_WIDTH - category_pad);
    float h = TILE_PANEL_LABEL_H;

    bool is_active = internal__is_category_active(category_index);

    Vec2 label_cur(0,0);
    BeginPanel(x, y, w, h, (is_active) ? UI_NONE : UI_INACTIVE);

    SetPanelCursor(&label_cur);
    SetPanelCursorDir(UI_DIR_DOWN);

    const char* label_name = NULL;
    switch (category_index)
    {
        case (TILE_CATEGORY_BASIC  ): label_name = "Basic";   break;
        case (TILE_CATEGORY_TAG    ): label_name = "Tag";     break;
        case (TILE_CATEGORY_OVERLAY): label_name = "Overlay"; break;
        case (TILE_CATEGORY_ENTITY ): label_name = "Entity";  break;
        case (TILE_CATEGORY_BACK1  ): label_name = "Back 1";  break;
        case (TILE_CATEGORY_BACK2  ): label_name = "Back 2";  break;
    }

    DoLabel(UI_ALIGN_LEFT,UI_ALIGN_CENTER, w, TILE_PANEL_LABEL_H, label_name);
    EndPanel();

    cursor.y += TILE_PANEL_LABEL_H;

    // Bounding box for the particular category in the panel.
    x = TILE_PANEL_INNER_PAD;
    y = cursor.y;
    w = (CONTROL_PANEL_WIDTH - category_pad);
    h = (rows * TILE_PANEL_ITEM_SIZE) + category_pad + total_item_pad;

    Vec2 cur(TILE_PANEL_INNER_PAD, TILE_PANEL_INNER_PAD);
    BeginPanel(x, y, w, h, UI_NONE, gUiColorMedDark);

    SetDrawColor(gUiColorExDark); // The outline/border for the category.
    DrawQuad(0, GetPanelOffset().y, w, GetPanelOffset().y+h);

    TextureAtlas& atlas = GetEditorAtlasSmall();

    SetTileBatchTexture(atlas.texture);
    SetTileBatchColor((is_active) ? Vec4(1,1,1,1) : Vec4(1,1,1,TILE_PANEL_INACTIVE_A));

    for (size_t i=0; i<category.size(); ++i)
    {
        Vec2 tile_cursor = cur;
        tile_cursor.y += GetPanelOffset().y;

        const Tile_Group& tile_group = category[i];
        Quad tile_group_bounds = { tile_cursor.x, tile_cursor.y, TILE_PANEL_ITEM_SIZE, TILE_PANEL_ITEM_SIZE };

        if (is_active)
        {
            bool in_bounds = MouseInUiBoundsXYWH(tile_group_bounds);
            if (in_bounds)
            {
                push_status_bar_message("Tile Group: %s", tile_group.name.c_str());
                set_current_tooltip(tile_group.name, tile_group.desc);
            }
            if (tile_panel.mouse_down && !IsThereAHitUiElement())
            {
                if (in_bounds)
                {
                    tile_panel.selected_category = category_index;
                    tile_panel.selected_group = static_cast<int>(i);
                }
            }
        }

        if (current_tab_is_level() && !are_all_layers_inactive())
        {
            float qx = tile_cursor.x        - 1;
            float qy = tile_cursor.y        - 1;
            float qw = TILE_PANEL_ITEM_SIZE + 2;
            float qh = TILE_PANEL_ITEM_SIZE + 2;

            // If we are the currently selected group then we draw the highlight.
            if ((category_index == tile_panel.selected_category) && (i == tile_panel.selected_group))
            {
                SetDrawColor(gUiColorLight);
                FillQuad(qx, qy, qx+qw, qy+qh);
            }
        }

        // We scissor the contents to avoid overspill.
        BeginScissor(tile_cursor.x, tile_cursor.y, TILE_PANEL_ITEM_SIZE, TILE_PANEL_ITEM_SIZE);

        float ex = tile_cursor.x + (TILE_PANEL_ITEM_SIZE/2);
        float ey = tile_cursor.y + (TILE_PANEL_ITEM_SIZE/2);

        Tile_ID selected_id = tile_group.tile[tile_group.selected_index];
        DrawBatchedTile(ex, ey, &GetAtlasClip(atlas, selected_id));

        EndScissor();

        // Advance the cursor for the next entity's placement.
        float advance = TILE_PANEL_ITEM_SIZE + TILE_PANEL_ITEM_PAD;
        cur.x += advance;
        if (cur.x + TILE_PANEL_ITEM_SIZE > w)
        {
            cur.x = TILE_PANEL_INNER_PAD;
            cur.y += advance;
        }
    }

    FlushBatchedTiles();

    EndPanel();

    // Set the cursor for the next category (if there is one).
    cursor.x  = TILE_PANEL_INNER_PAD;
    cursor.y += h;

    // Return the category height for use with the scrollbar.
    return h;
}

TEINAPI void internal__load_flip_data (const GonObject& data, std::vector<Tile_Flip_Map>& flip)
{
    for (int i=0; i<static_cast<int>(data.children_array.size()); ++i)
    {
        Tile_ID a = data[i][0].Int();
        Tile_ID b = data[i][1].Int();

        flip.push_back(Tile_Flip_Map(a, b));
    }
}

TEINAPI float internal__calculate_tile_panel_height ()
{
    float height = TILE_PANEL_INNER_PAD;
    for (auto& it: tile_panel.category)
    {
        const std::vector<Tile_Group>& c = it.second;

        float items          = static_cast<float>(c.size());
        float rows           = ceilf(items / TILE_PANEL_COLUMNS);
        float total_item_pad = (rows-1) * TILE_PANEL_ITEM_PAD;
        float category_pad   = (TILE_PANEL_INNER_PAD*2);

        height += (rows * TILE_PANEL_ITEM_SIZE) + category_pad + total_item_pad;
        height += TILE_PANEL_LABEL_H;
    }
    return height;
}

TEINAPI void internal__set_category_as_active (Tile_Category category)
{
    int old_selected_category = tile_panel.selected_category;
    int old_selected_group = tile_panel.selected_group;

    tile_panel.selected_category = category;
    tile_panel.selected_group = 0;

    // Make sure we are actually jumping to an active category.
    if (!internal__is_category_active(tile_panel.selected_category))
    {
        select_next_active_group();
    }

    if (old_selected_category != tile_panel.selected_category || old_selected_group != tile_panel.selected_group)
    {
        // When the selected gets changed then we make a new state.
        if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
        {
            if (level_editor.tool_state != Tool_State::IDLE)
            {
                new_level_history_state(Level_History_Action::NORMAL);
            }
        }
    }
}

TEINAPI void internal__jump_to_category (Tile_Category category)
{
    if (current_tab_is_level())
    {
        if (!are_all_layers_inactive())
        {
            if (IsWindowFocused("WINMAIN"))
            {
                internal__set_category_as_active(category);
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI bool init_tile_panel ()
{
    tile_panel.flip_map_horz.clear();
    tile_panel.flip_map_vert.clear();

    tile_panel.category.clear();

    try
    {
        // Load all of the tile groups and sort them into their categories.
        GonObject tile_gon_data = GonObject::LoadFromBuffer(LoadStringResource(TILE_DATA_FILE))["tiles"];

        for (auto& it: tile_gon_data.children_map)
        {
            const GonObject& category_data = tile_gon_data.children_array[it.second];
            if (std::stoi(it.first) < TILE_CATEGORY_TOTAL)
            {
                Tile_Category category_id = std::stoi(it.first);
                for (auto& tile_group_gon_data: category_data.children_array)
                {
                    Tile_Group group;
                    group.selected_index = 0;
                    group.name = tile_group_gon_data["name"].String();
                    group.desc = tile_group_gon_data["tooltip"].String();
                    for (int i=0; i<tile_group_gon_data["id"].size(); ++i)
                    {
                        Tile_ID tile_id = tile_group_gon_data["id"][i].Int();
                        group.tile.push_back(tile_id);
                    }
                    tile_panel.category[category_id].push_back(group);
                }
            }
        }

        // Load flip mappings between the tile IDs for smart level flipping.
        GonObject flip_gon_data = GonObject::LoadFromBuffer(LoadStringResource(FLIP_DATA_FILE))["flip"];

        internal__load_flip_data(flip_gon_data["horz"], tile_panel.flip_map_horz);
        internal__load_flip_data(flip_gon_data["vert"], tile_panel.flip_map_vert);
    }
    catch (const char* msg)
    {
        LogError(ERR_MED, "%s", msg);
        return false;
    }

    // The starting selected is always the first entry in the first category.
    tile_panel.selected_category = TILE_CATEGORY_BASIC;
    tile_panel.selected_group = 0;

    // We need to calculate the height of all the tiles panel content so that
    // we can use this height value during the panel do_scrollbar operation.
    tile_panel.content_height = internal__calculate_tile_panel_height();
    tile_panel.scroll_offset = 0;

    return true;
}

TEINAPI void do_tile_panel (bool scrollbar)
{
    SetUiFont(&GetEditorRegularFont());

    Vec2 cursor(TILE_PANEL_INNER_PAD, 0);
    tile_panel.bounds = { 0, 0, GetPanelWidth(), GetPanelHeight() };
    if (is_layer_panel_present())
    {
        tile_panel.bounds.h = roundf(tile_panel.bounds.h - get_layer_panel_height());
    }

    BeginPanel(tile_panel.bounds, UI_NONE, gUiColorMedium);

    if (scrollbar)
    {
        float x = CONTROL_PANEL_WIDTH;
        float y = CONTROL_PANEL_INNER_PAD;
        float w = CONTROL_PANEL_SCROLLBAR_WIDTH - CONTROL_PANEL_INNER_PAD;
        float h = GetViewport().h - (CONTROL_PANEL_INNER_PAD * 2);

        Quad sb = { x,y,w,h };

        DoScrollbar(sb, tile_panel.content_height, tile_panel.scroll_offset);
    }

    for (auto& it: tile_panel.category)
    {
        internal__do_tile_panel_category(cursor, it.first, it.second);
    }

    EndPanel();
}

/* -------------------------------------------------------------------------- */

TEINAPI bool tile_panel_needs_scrollbar ()
{
    return (tile_panel.content_height > tile_panel.bounds.h);
}

/* -------------------------------------------------------------------------- */

TEINAPI void handle_tile_panel_events ()
{
    if (current_tab_is_level())
    {
        if (!are_all_layers_inactive())
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
                            tile_panel.mouse_down = (main_event.button.state == SDL_PRESSED);
                        }
                    } break;
                    case (SDL_MOUSEWHEEL):
                    {
                        if (!mouse_is_over_tab_bar())
                        {
                            if (IsKeyModStateActive(0))
                            {
                                if      (main_event.wheel.y > 0) increment_selected_tile();
                                else if (main_event.wheel.y < 0) decrement_selected_tile();
                            }
                            else if (IsKeyModStateActive(KMOD_ALT))
                            {
                                if      (main_event.wheel.y > 0) increment_selected_group();
                                else if (main_event.wheel.y < 0) decrement_selected_group();
                            }
                            else if (IsKeyModStateActive(KMOD_ALT|KMOD_SHIFT))
                            {
                                if      (main_event.wheel.y > 0) increment_selected_category();
                                else if (main_event.wheel.y < 0) decrement_selected_category();
                            }
                        }
                    } break;
                }
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI float get_tile_panel_height ()
{
    return (tile_panel.bounds.h + 1);
}

/* -------------------------------------------------------------------------- */

TEINAPI void reload_tile_graphics ()
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

/* -------------------------------------------------------------------------- */

TEINAPI Tile_Category get_selected_category ()
{
    return tile_panel.selected_category;
}

TEINAPI Tile_ID get_selected_tile ()
{
    const auto& category = tile_panel.category[tile_panel.selected_category];
    const auto& group = category[tile_panel.selected_group];
    return group.tile[group.selected_index];
}

TEINAPI Level_Layer get_selected_layer ()
{
    return category_to_layer(tile_panel.selected_category);
}

/* -------------------------------------------------------------------------- */

TEINAPI Level_Layer category_to_layer (Tile_Category category)
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

/* -------------------------------------------------------------------------- */

TEINAPI void select_next_active_group ()
{
    if (!are_all_layers_inactive())
    {
        while (!internal__is_category_active(tile_panel.selected_category))
        {
            ++tile_panel.selected_category;
            if (tile_panel.selected_category > TILE_CATEGORY_BACK2)
            {
                tile_panel.selected_category = TILE_CATEGORY_BASIC;
            }
            tile_panel.selected_group = 0;
        }
    }
}

TEINAPI void select_prev_active_group ()
{
    if (!are_all_layers_inactive())
    {
        while (!internal__is_category_active(tile_panel.selected_category))
        {
            --tile_panel.selected_category;
            if (tile_panel.selected_category < TILE_CATEGORY_BASIC)
            {
                tile_panel.selected_category = TILE_CATEGORY_BACK2;
            }
            tile_panel.selected_group = 0;
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void reset_selected_group ()
{
    tile_panel.selected_category = 0;
    select_next_active_group();
}

/* -------------------------------------------------------------------------- */

TEINAPI void increment_selected_tile ()
{
    if (!current_tab_is_level()) return;

    if (!are_all_layers_inactive())
    {
        auto& group = tile_panel.category[tile_panel.selected_category][tile_panel.selected_group];
        int old_selected_index = group.selected_index;
        if ((++group.selected_index) > group.tile.size()-1)
        {
            group.selected_index = 0;
        }
        // Feels correct that a new history state should be made.
        if (old_selected_index != group.selected_index)
        {
            if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
            {
                if (level_editor.tool_state != Tool_State::IDLE)
                {
                    new_level_history_state(Level_History_Action::NORMAL);
                }
            }
        }
    }
}

TEINAPI void decrement_selected_tile ()
{
    if (!current_tab_is_level()) return;

    if (!are_all_layers_inactive())
    {
        auto& group = tile_panel.category[tile_panel.selected_category][tile_panel.selected_group];
        int old_selected_index = group.selected_index;
        if ((--group.selected_index) < 0)
        {
            group.selected_index = static_cast<int>(group.tile.size()-1);
        }
        // Feels correct that a new history state should be made.
        if (old_selected_index != group.selected_index)
        {
            if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
            {
                if (level_editor.tool_state != Tool_State::IDLE)
                {
                    new_level_history_state(Level_History_Action::NORMAL);
                }
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void increment_selected_group ()
{
    if (!current_tab_is_level()) return;

    if (!are_all_layers_inactive())
    {
        auto& category = tile_panel.category[tile_panel.selected_category];
        int old_selected_group = tile_panel.selected_group;
        if ((++tile_panel.selected_group) > category.size()-1)
        {
            tile_panel.selected_group = 0;
        }
        // Feels correct that a new history state should be made.
        if (old_selected_group != tile_panel.selected_group)
        {
            if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
            {
                if (level_editor.tool_state != Tool_State::IDLE)
                {
                    new_level_history_state(Level_History_Action::NORMAL);
                }
            }
        }
    }
}

TEINAPI void decrement_selected_group ()
{
    if (!current_tab_is_level()) return;

    if (!are_all_layers_inactive())
    {
        auto& category = tile_panel.category[tile_panel.selected_category];
        int old_selected_group = tile_panel.selected_group;
        if ((--tile_panel.selected_group) < 0)
        {
            tile_panel.selected_group = static_cast<int>(category.size()-1);
        }
        // Feels correct that a new history state should be made.
        if (old_selected_group != tile_panel.selected_group)
        {
            if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
            {
                if (level_editor.tool_state != Tool_State::IDLE)
                {
                    new_level_history_state(Level_History_Action::NORMAL);
                }
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void increment_selected_category ()
{
    if (!current_tab_is_level()) return;

    int old_selected_category = tile_panel.selected_category;
    int old_selected_group = tile_panel.selected_group;

    if (!are_all_layers_inactive())
    {
        if ((++tile_panel.selected_category) > TILE_CATEGORY_BACK2)
        {
            tile_panel.selected_category = TILE_CATEGORY_BASIC;
        }
        tile_panel.selected_group = 0;
        select_next_active_group();
        // Feels correct that a new history state should be made.
        if (old_selected_category != tile_panel.selected_category || old_selected_group != tile_panel.selected_group)
        {
            if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
            {
                if (level_editor.tool_state != Tool_State::IDLE)
                {
                    new_level_history_state(Level_History_Action::NORMAL);
                }
            }
        }
    }
}

TEINAPI void decrement_selected_category ()
{
    if (!current_tab_is_level()) return;

    int old_selected_category = tile_panel.selected_category;
    int old_selected_group = tile_panel.selected_group;

    if (!are_all_layers_inactive())
    {
        if ((--tile_panel.selected_category) < TILE_CATEGORY_BASIC)
        {
            tile_panel.selected_category = TILE_CATEGORY_BACK2;
        }
        tile_panel.selected_group = 0;
        select_prev_active_group();
        // Feels correct that a new history state should be made.
        if (old_selected_category != tile_panel.selected_category || old_selected_group != tile_panel.selected_group)
        {
            if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
            {
                if (level_editor.tool_state != Tool_State::IDLE)
                {
                    new_level_history_state(Level_History_Action::NORMAL);
                }
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI Tile_ID get_tile_horizontal_flip (Tile_ID id)
{
    if (id)
    {
        for (const auto& pair: tile_panel.flip_map_horz)
        {
            if (id == pair.first)  return pair.second;
            if (id == pair.second) return pair.first;
        }
    }
    return id;
}
TEINAPI Tile_ID get_tile_vertical_flip (Tile_ID id)
{
    if (id)
    {
        for (const auto& pair: tile_panel.flip_map_vert)
        {
            if (id == pair.first)  return pair.second;
            if (id == pair.second) return pair.first;
        }
    }
    return id;
}

/* -------------------------------------------------------------------------- */

TEINAPI void jump_to_category_basic ()
{
    internal__jump_to_category(TILE_CATEGORY_BASIC);
}

TEINAPI void jump_to_category_tag ()
{
    internal__jump_to_category(TILE_CATEGORY_TAG);
}

TEINAPI void jump_to_category_overlay ()
{
    internal__jump_to_category(TILE_CATEGORY_OVERLAY);
}

TEINAPI void jump_to_category_entity ()
{
    internal__jump_to_category(TILE_CATEGORY_ENTITY);
}

TEINAPI void jump_to_category_back1 ()
{
    internal__jump_to_category(TILE_CATEGORY_BACK1);
}

TEINAPI void jump_to_category_back2 ()
{
    internal__jump_to_category(TILE_CATEGORY_BACK2);
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
