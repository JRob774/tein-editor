Internal struct
{
    bool open = true; // @Incomplete: Store this value so we can restore the GUI state properly for later sessions.

} iTilesWidget;

EditorAPI void InitTilesWidget ()
{
    // Nothing...
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

    Texture* tileTex = GetAsset<AssetTexture>("small_icons/10");
    if (tileTex) {
        // Code for wrapping button items taken from this issue in the imgui repo:
        //   https://github.com/ocornut/imgui/issues/1977#issuecomment-408847708
        ImGuiStyle& style = ImGui::GetStyle();
        float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        ImVec2 tileSize(20,20);
        for (int i=0, n=50; i<n; ++i) {
            ImGui::PushID(i);
            ImGui::ImageButton((ImTextureID)(intptr_t)tileTex->handle, tileSize, ImVec2(0,0), ImVec2(1,1), 2, ImVec4(0,0,0,0), ImVec4(1,1,1,1));
            float lastButtonX2 = ImGui::GetItemRectMax().x;
            float nextButtonX2 = lastButtonX2 + style.ItemSpacing.x + tileSize.x; // Expected position if next button was on same line.
            if ((i+1 < n) && (nextButtonX2 < windowVisibleX2)) ImGui::SameLine();
            ImGui::PopID();
        }
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
