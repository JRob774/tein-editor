typedef void(*MenuBarCallback)(void);

struct UserInterface
{
    MenuBarCallback menuBarCallback;
};

Global UserInterface gUserInterface;

EditorAPI bool InitUi ();
EditorAPI void QuitUi ();

EditorAPI void HandleUiEvents (const SDL_Event& event);

EditorAPI void BeginUiFrame ();
EditorAPI void EndUiFrame ();

// Wrap ImGui::ImageButton with getting a texture from the asset manager and using it.
// If a texture cannot be received from the asset manager then NULL is passed to ImGui.
EditorAPI bool DoImageButton (std::string imageName, ImVec2 size);
