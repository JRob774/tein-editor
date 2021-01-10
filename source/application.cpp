EditorAPI bool InitApplication ()
{
    if (!CreateWidget<TilesWidget>()) return false;
    if (!CreateWidget<ToolsWidget>()) return false;

    InitHotbar();
    return true;
}

EditorAPI void QuitApplication ()
{
    FreeWidgets();
}

EditorAPI void DoApplication ()
{
    // ImGui::ShowDemoWindow();
    UpdateWidgets();
}
