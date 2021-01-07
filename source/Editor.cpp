Internal struct
{
    // Nothing...

} iEditor;

EditorAPI bool InitEditor ()
{
    if (!InitTilesWidget()) return false;
    return true;
}
EditorAPI void QuitEditor ()
{
    QuitTilesWidget();
}

EditorAPI void HandleEditorEvents (const SDL_Event& event)
{
    // Nothing...
}

EditorAPI void DoEditorMenuBar ()
{
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New", "Ctrl+N")) {} // @Incomplete: ...
        if (ImGui::MenuItem("Open", "Ctrl+O")) {} // @Incomplete: ...
        if (ImGui::MenuItem("Save", "Ctrl+S")) {} // @Incomplete: ...
        if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) {} // @Incomplete: ...
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "Ctrl+Z")) {} // @Incomplete: ...
        if (ImGui::MenuItem("Redo", "Ctrl+Y")) {} // @Incomplete: ...
        if (ImGui::MenuItem("Start of History", "Ctrl+Shift+Z")) {} // @Incomplete: ...
        if (ImGui::MenuItem("End of History", "Ctrl+Shift+Y")) {} // @Incomplete: ...
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Zoom In", "Ctrl+Plus")) {} // @Incomplete: ...
        if (ImGui::MenuItem("Zoom Out", "Ctrl+Minus")) {} // @Incomplete: ...
        if (ImGui::MenuItem("Reset View", "Ctrl+0")) {} // @Incomplete: ...
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("GPAK")) {
        if (ImGui::MenuItem("Pack", "Ctrl+P")) {} // @Incomplete: ...
        if (ImGui::MenuItem("Unpack", "Ctrl+U")) {} // @Incomplete: ...
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Window")) {
        ImGui::MenuItem("Tiles", NULL, &GetTilesWidgetOpen());
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Launch Game")) {
        // @Incomplete: ...
    }
    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("Settings")) {} // @Incomplete: ...
        if (ImGui::MenuItem("Report Bug")) {} // @Incomplete: ...
        if (ImGui::MenuItem("Manual")) {} // @Incomplete: ...
        if (ImGui::MenuItem("About")) {} // @Incomplete: ...
        ImGui::EndMenu();
    }
}

EditorAPI void DoEditor ()
{
    ImGui::ShowDemoWindow();

    DoTilesWidget();
}
