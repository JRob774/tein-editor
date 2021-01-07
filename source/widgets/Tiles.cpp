typedef S32 TileID;

struct Tile
{
    std::string name, tooltip;
    int layer; // @Improve; Make an enum for this??
    std::vector<TileID> ids;
};

struct Category
{
    std::string name;
    std::vector<std::string> tiles; // These strings point into the iTilesWidget.tiles map.
};

Internal struct
{
    std::map<std::string,Tile> tiles;
    std::map<std::string,Category> categories;

    std::vector<std::string> categoryList; // These strings point into the iTilesWidget.categories map.

    bool open;

} iTilesWidget;

EditorAPI bool InitTilesWidget ()
{
    PushLogSystem("tiles");
    Defer { PopLogSystem(); };

    //
    // Load Tiles
    //

    auto* tileDataPointer = GetAsset<AssetData>("tiles");
    if (!tileDataPointer) {
        LogSystemMessage("Failed to locate the tile data file!");
        return false;
    } else {
        auto& tileData = *tileDataPointer; // Safe to dereference now.
        for (auto& data: tileData.children_array) {
            Tile tile;
            tile.name = data["name"].String();
            tile.tooltip = data["tooltip"].String();
            tile.layer = 0; // data["layer"].String(); // @Incomplete: Convert layer string to int!
            for (auto& id: data["ids"].children_array) {
                tile.ids.push_back(id.Int());
            }
            iTilesWidget.tiles.insert({ data.name, std::move(tile) });
        }
    }

    //
    // Load Categories
    //

    auto* categoryDataPointer = GetAsset<AssetData>("categories");
    if (!categoryDataPointer) {
        LogSystemMessage("Failed to locate the category data file!");
        return false;
    } else {
        auto& categoryData = *categoryDataPointer; // Safe to dereference now.
        for (auto& data: categoryData.children_array) {
            Category category;
            category.name = data["name"].String();
            for (auto& tile: data["tiles"].children_array) {
                category.tiles.push_back(tile.String());
            }
            iTilesWidget.categories.insert({ data.name, std::move(category) });
            iTilesWidget.categoryList.push_back(data.name);
        }
    }

    // @Improve: We don't want this to always be true on start up. Store this
    // value so we can restore the GUI state properly for later sessions.
    iTilesWidget.open = true;

    return true;
}

EditorAPI void QuitTilesWidget ()
{
    // Nothing...
}

EditorAPI void DoTilesWidget ()
{
    if (!iTilesWidget.open) return; // If the window is closed then don't do it.

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;
    ImGui::Begin("Tiles", &iTilesWidget.open, windowFlags);

    // Code for wrapping button items taken from this issue in the imgui repo:
    //   https://github.com/ocornut/imgui/issues/1977#issuecomment-408847708
    ImGuiStyle& style = ImGui::GetStyle();
    float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    ImVec2 tileSize(20,20);
    int count = 0;
    for (auto& categoryId: iTilesWidget.categoryList) {
        auto& category = iTilesWidget.categories.at(categoryId);
        int index = 0;
        for (auto& tile: category.tiles) {
            Texture* tileTex = GetAsset<AssetTexture>("small_icons/" + std::to_string(iTilesWidget.tiles.at(tile).ids.at(0)));
            if (tileTex) {
                ImGui::PushID(count);
                ImGui::ImageButton((ImTextureID)(intptr_t)tileTex->handle, tileSize, ImVec2(0,0), ImVec2(1,1), 2, ImVec4(0,0,0,0), ImVec4(1,1,1,1));
                float lastButtonX2 = ImGui::GetItemRectMax().x;
                float nextButtonX2 = lastButtonX2 + style.ItemSpacing.x + tileSize.x; // Expected position if next button was on same line.
                if ((index+1 < category.tiles.size()) && (nextButtonX2 < windowVisibleX2)) ImGui::SameLine();
                ++index;
                ++count;
                ImGui::PopID();
            }
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

    ImGui::End();
}

EditorAPI void ToggleTilesWidget ()
{
    iTilesWidget.open = !iTilesWidget.open;
}

EditorAPI bool IsTilesWidgetOpen ()
{
    return iTilesWidget.open;
}
