EditorAPI bool InitApplication ()
{
    if (!InitToolsWidget()) return false;
    if (!InitTilesWidget()) return false;
    InitHotbar();
    return true;
}

EditorAPI void QuitApplication ()
{
    QuitTilesWidget();
    QuitToolsWidget();
}

EditorAPI void DoApplication ()
{
    // ImGui::ShowDemoWindow();
    DoToolsWidget();
    DoTilesWidget();
}
