//
// Base Widget
//

Widget::Widget (std::string name)
{
    mName = name;
    mOpen = true;
}
Widget::~Widget ()
{
    // Nothing...
}
bool Widget::Create ()
{
    return true;
}
void Widget::Delete ()
{
    // Nothing...
}
void Widget::Update ()
{
    if (!mOpen) return; // If the widget is closed then don't update it.
    ImGui::Begin(mName.c_str(), &mOpen, ImGuiWindowFlags_NoCollapse);
    InternalUpdate();
    ImGui::End();
}

//
// Widget Manager
//

EditorAPI bool InitWidgetManager ()
{
    if (!CreateWidget<  TilesWidget>()) return false;
    if (!CreateWidget<  ToolsWidget>()) return false;
    if (!CreateWidget< LayersWidget>()) return false;
    if (!CreateWidget<HistoryWidget>()) return false;
    return true;
}
EditorAPI void QuitWidgetManager ()
{
    for (auto& widget: gWidgetManager.widgets) {
        widget->Delete();
        delete widget;
    }
}

EditorAPI void UpdateWidgets ()
{
    for (auto& widget: gWidgetManager.widgets) {
        widget->Update();
    }
}

//
// Tiles Widget
//

TilesWidget::TilesWidget (): Widget("Tiles")
{
    // Nothing...
}
TilesWidget::~TilesWidget ()
{
    // Nothing...
}
bool TilesWidget::Create ()
{
    PushLogSystem("tiles");
    Defer { PopLogSystem(); };

    // Load Tiles
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
            tile.layer = LevelLayer::Active; // data["layer"].String(); // @Incomplete: Convert layer string to int!!!!
            for (auto& id: data["ids"].children_array) {
                tile.ids.push_back(id.Int());
            }
            mTiles.insert({ data.name, std::move(tile) });
        }
    }

    // Load Categories
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
            mCategories.insert({ data.name, std::move(category) });
            mCategoryList.push_back(data.name);
        }
    }

    return true;
}
void TilesWidget::InternalUpdate ()
{
    // Code for wrapping button items taken from this issue in the imgui repo:
    //   https://github.com/ocornut/imgui/issues/1977#issuecomment-408847708
    ImGuiStyle& style = ImGui::GetStyle();
    float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    ImVec2 tileSize(20,20);
    int count = 0;
    for (auto& categoryId: mCategoryList) {
        auto& category = mCategories.at(categoryId);
        if (ImGui::CollapsingHeader(category.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2,2));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3,3));
            int index = 0;
            for (auto& tile: category.tiles) {
                std::string textureName = "tiles_small/" + std::to_string(mTiles.at(tile).ids.at(0));
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
}

//
// Tools Widget
//

ToolsWidget::ToolsWidget (): Widget("Tools")
{
    // Nothing...
}
ToolsWidget::~ToolsWidget ()
{
    // Nothing...
}
void ToolsWidget::InternalUpdate ()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2,2));
    ImVec2 buttonSize(24,24);
    DoImageButton("icons/brush", buttonSize);
    DoImageButton("icons/fill", buttonSize);
    DoImageButton("icons/select", buttonSize);
    DoImageButton("icons/grid", buttonSize);
    DoImageButton("icons/bounds", buttonSize);
    DoImageButton("icons/layertrans", buttonSize);
    DoImageButton("icons/resetcamera", buttonSize);
    DoImageButton("icons/entitylarge", buttonSize);
    DoImageButton("icons/entityguide", buttonSize);
    DoImageButton("icons/resize", buttonSize);
    DoImageButton("icons/fliph", buttonSize);
    DoImageButton("icons/flipv", buttonSize);
    DoImageButton("icons/mirrorh", buttonSize);
    DoImageButton("icons/mirrorv", buttonSize);
    DoImageButton("icons/cut", buttonSize);
    DoImageButton("icons/copy", buttonSize);
    DoImageButton("icons/deselect", buttonSize);
    DoImageButton("icons/clearselect", buttonSize);
    ImGui::PopStyleVar();
}

//
// Layers Widget
//

LayersWidget::LayersWidget (): Widget("Layers")
{
    // Nothing...
}
LayersWidget::~LayersWidget ()
{
    // Nothing...
}
void LayersWidget::InternalUpdate ()
{
    // @Incomplete: ...
}

//
// History Widget
//

HistoryWidget::HistoryWidget (): Widget("History")
{
    // Nothing...
}
HistoryWidget::~HistoryWidget ()
{
    // Nothing...
}
void HistoryWidget::InternalUpdate ()
{
    // @Incomplete: ...
}
