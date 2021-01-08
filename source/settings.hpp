Global struct
{
    int windowWidth;
    int windowHeight;
    bool fullscreen;

} gSettings;

EditorAPI void SaveSettings ();
EditorAPI void LoadSettings ();

EditorAPI void ResetSettings ();
