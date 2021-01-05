Internal struct
{
    // Nothing...

} iEditor;

EditorAPI bool InitEditor ()
{
    // Nothing...
    return true;
}
EditorAPI void QuitEditor ()
{
    // Nothing...
}

EditorAPI void HandleEditorEvents (const SDL_Event& event)
{
    // Nothing...
}

EditorAPI void DoEditorMenuBar ()
{
    if (ImGui::BeginMenu("Windows")) {
        if (ImGui::MenuItem("Tiles")) ToggleTilesWidget();
        ImGui::EndMenu();
    }
}

EditorAPI void DoEditor ()
{
    ImGui::ShowDemoWindow();

    DoTilesWidget();
}
