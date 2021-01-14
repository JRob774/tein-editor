#include "Widgets.hpp"

#include "Logger.hpp"

#include "AssetManager.hpp"
#include "UserInterface.hpp"

namespace TEIN
{
    bool Widget::Create ()
    {
        m_Name = "Widget";
        m_Open = true;
        return true;
    }
    void Widget::Delete ()
    {
        // Nothing...
    }
    void Widget::Update ()
    {
        if (!m_Open) return; // If the widget is closed then don't update it.
        ImGui::Begin(m_Name.c_str(), &m_Open, ImGuiWindowFlags_NoCollapse);
        InternalUpdate();
        ImGui::End();
    }

    bool TilesWidget::Create ()
    {
        m_Name = "Tiles";
        m_Open = true;

        Logger::PushSystem("tiles");
        Defer { Logger::PopSystem(); };

        // Load Tiles
        auto* tileDataPointer = AssetManager::Get<AssetData>("tiles");
        if (!tileDataPointer) {
            Logger::SystemMessage("Failed to locate the tile data file!");
            return false;
        } else {
            auto& tileData = *tileDataPointer; // Safe to dereference now.
            for (auto& data: tileData.children_array) {
                Tile tile;
                tile.name = data["name"].String();
                tile.tooltip = data["tooltip"].String();
                tile.layer = 0; // data["layer"].String(); // @Incomplete: Convert layer string to int!!!!
                for (auto& id: data["ids"].children_array) {
                    tile.ids.push_back(id.Int());
                }
                m_Tiles.insert({ data.name, std::move(tile) });
            }
        }

        // Load Categories
        auto* categoryDataPointer = AssetManager::Get<AssetData>("categories");
        if (!categoryDataPointer) {
            Logger::SystemMessage("Failed to locate the category data file!");
            return false;
        } else {
            auto& categoryData = *categoryDataPointer; // Safe to dereference now.
            for (auto& data: categoryData.children_array) {
                Category category;
                category.name = data["name"].String();
                for (auto& tile: data["tiles"].children_array) {
                    category.tiles.push_back(tile.String());
                }
                m_Categories.insert({ data.name, std::move(category) });
                m_CategoryList.push_back(data.name);
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
        for (auto& categoryId: m_CategoryList) {
            auto& category = m_Categories.at(categoryId);
            if (ImGui::CollapsingHeader(category.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2,2));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3,3));
                int index = 0;
                for (auto& tile: category.tiles) {
                    std::string textureName = "tiles_small/" + std::to_string(m_Tiles.at(tile).ids.at(0));
                    ImGui::PushID(count);
                    Ui::DoImageButton(textureName, tileSize);
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

    bool ToolsWidget::Create ()
    {
        m_Name = "Tools";
        m_Open = true;
        return true;
    }
    void ToolsWidget::InternalUpdate ()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2,2));
        ImVec2 buttonSize(24,24);
        Ui::DoImageButton("icons/brush", buttonSize);
        Ui::DoImageButton("icons/fill", buttonSize);
        Ui::DoImageButton("icons/select", buttonSize);
        Ui::DoImageButton("icons/grid", buttonSize);
        Ui::DoImageButton("icons/bounds", buttonSize);
        Ui::DoImageButton("icons/layertrans", buttonSize);
        Ui::DoImageButton("icons/resetcamera", buttonSize);
        Ui::DoImageButton("icons/entitylarge", buttonSize);
        Ui::DoImageButton("icons/entityguide", buttonSize);
        Ui::DoImageButton("icons/resize", buttonSize);
        Ui::DoImageButton("icons/fliph", buttonSize);
        Ui::DoImageButton("icons/flipv", buttonSize);
        Ui::DoImageButton("icons/mirrorh", buttonSize);
        Ui::DoImageButton("icons/mirrorv", buttonSize);
        Ui::DoImageButton("icons/cut", buttonSize);
        Ui::DoImageButton("icons/copy", buttonSize);
        Ui::DoImageButton("icons/deselect", buttonSize);
        Ui::DoImageButton("icons/clearselect", buttonSize);
        ImGui::PopStyleVar();
    }

    bool LayersWidget::Create ()
    {
        m_Name = "Layers";
        m_Open = true;
        return true;
    }
    void LayersWidget::InternalUpdate ()
    {
        // @Incomplete: ...
    }

    bool HistoryWidget::Create ()
    {
        m_Name = "History";
        m_Open = true;
        return true;
    }
    void HistoryWidget::InternalUpdate ()
    {
        // @Incomplete: ...
    }

    namespace WidgetManager
    {
        std::vector<Widget*> g_Widgets;

        bool Init ()
        {
            if (!CreateWidget<  TilesWidget>()) return false;
            if (!CreateWidget<  ToolsWidget>()) return false;
            if (!CreateWidget< LayersWidget>()) return false;
            if (!CreateWidget<HistoryWidget>()) return false;
            return true;
        }
        void Quit ()
        {
            for (auto& widget: g_Widgets) {
                widget->Delete();
                delete widget;
            }
        }
        void Update ()
        {
            for (auto& widget: g_Widgets) {
                widget->Update();
            }
        }
    }
}
