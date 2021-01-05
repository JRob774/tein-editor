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

EditorAPI void DoEditor ()
{
    ImGui::ShowAboutWindow();
    ImGui::ShowDemoWindow();
}
