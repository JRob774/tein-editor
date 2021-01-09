EditorAPI bool InitToolsWidget ()
{
    // @Incomplete: ...

    // @Improve: We don't want this to always be true on start up. Store this
    // value so we can restore the GUI state properly for later sessions.
    gToolsWidget.open = true;

    return true;
}

EditorAPI void QuitToolsWidget ()
{
    // Nothing...
}

EditorAPI void DoToolsWidget ()
{
    if (!gToolsWidget.open) return; // If the window is closed then don't do it.

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Tools", &gToolsWidget.open, windowFlags);

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

    ImGui::End();
}

EditorAPI bool& GetToolsWidgetOpen ()
{
    return gToolsWidget.open;
}
