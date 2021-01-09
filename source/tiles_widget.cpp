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
            gTilesWidget.tiles.insert({ data.name, std::move(tile) });
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
            gTilesWidget.categories.insert({ data.name, std::move(category) });
            gTilesWidget.categoryList.push_back(data.name);
        }
    }

    // @Improve: We don't want this to always be true on start up. Store this
    // value so we can restore the GUI state properly for later sessions.
    gTilesWidget.open = true;

    return true;
}

EditorAPI void QuitTilesWidget ()
{
    // Nothing...
}

EditorAPI void DoTilesWidget ()
{
    if (!gTilesWidget.open) return; // If the window is closed then don't do it.

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Tiles", &gTilesWidget.open, windowFlags);

    // Code for wrapping button items taken from this issue in the imgui repo:
    //   https://github.com/ocornut/imgui/issues/1977#issuecomment-408847708
    ImGuiStyle& style = ImGui::GetStyle();
    float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    ImVec2 tileSize(20,20);
    int count = 0;
    for (auto& categoryId: gTilesWidget.categoryList) {
        auto& category = gTilesWidget.categories.at(categoryId);
        if (ImGui::CollapsingHeader(category.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2,2));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3,3));
            int index = 0;
            for (auto& tile: category.tiles) {
                std::string textureName = "tiles_small/" + std::to_string(gTilesWidget.tiles.at(tile).ids.at(0));
                ImGui::PushID(count);
                DoImageButton(textureName, tileSize);
                float lastButtonX2 = ImGui::GetItemRectMax().x;
                float nextButtonX2 = lastButtonX2 + style.ItemSpacing.x + tileSize.x; // Expected position if next button was on same line.
                if ((index+1 < category.tiles.size()) && (nextButtonX2 < windowVisibleX2)) ImGui::SameLine();
                ++index;
                ++count;
                ImGui::PopID();
            }
            ImGui::PopStyleVar(2);
        }
    }

    ImGui::End();
}

EditorAPI bool& GetTilesWidgetOpen ()
{
    return gTilesWidget.open;
}
