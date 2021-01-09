Global constexpr const char* gSettingsFile = "settings.data";

Global constexpr int  gSettingsDefaultWindowWidth = gWindowStartWidth;
Global constexpr int  gSettingsDefaultWindowHeight = gWindowStartHeight;
Global constexpr bool gSettingsDefaultFullscreen = true;

struct Settings
{
    int windowWidth;
    int windowHeight;
    bool fullscreen;
};

Global Settings gSettings;

EditorAPI void SaveSettings  ();
EditorAPI void LoadSettings  ();
EditorAPI void ResetSettings ();
