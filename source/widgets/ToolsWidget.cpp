Internal struct
{
    // @Incomplete: ...
    bool open;

} iToolsWidget;

EditorAPI bool InitToolsWidget ()
{
    // @Incomplete: ...

    // @Improve: We don't want this to always be true on start up. Store this
    // value so we can restore the GUI state properly for later sessions.
    iToolsWidget.open = true;

    return true;
}

EditorAPI void QuitToolsWidget ()
{
    // Nothing...
}

EditorAPI void DoToolsWidget ()
{
    if (!iToolsWidget.open) return; // If the window is closed then don't do it.

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;
    ImGui::Begin("Tools", &iToolsWidget.open, windowFlags);

    // @Incomplete: ...

    ImGui::End();
}

EditorAPI bool& GetToolsWidgetOpen ()
{
    return iToolsWidget.open;
}
