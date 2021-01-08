Internal constexpr const char* iSettingsFile = "settings.data";

Internal constexpr int iSettingsDefaultWindowWidth = gWindowStartWidth;
Internal constexpr int iSettingsDefaultWindowHeight = gWindowStartHeight;
Internal constexpr bool iSettingsDefaultFullscreen = true;

EditorAPI void SaveSettings ()
{
    std::ofstream settings(iSettingsFile, std::ios::trunc);
    if (!settings.is_open()) {
        LogSingleSystemMessage("settings", "Failed to save settings!");
    } else {
        settings << std::boolalpha;
        settings << "window_width " << GetCachedWindowWidth() << std::endl;
        settings << "window_height " << GetCachedWindowHeight() << std:: endl;
        settings << "fullscreen " << IsWindowFullscreen() << std::endl;
    }
}

EditorAPI void LoadSettings ()
{
    GonObject settings = GonObject::Load(iSettingsFile);
    gSettings.windowWidth = settings["window_width"].Int(iSettingsDefaultWindowWidth);
    gSettings.windowHeight = settings["window_height"].Int(iSettingsDefaultWindowHeight);
    gSettings.fullscreen = settings["fullscreen"].Bool(iSettingsDefaultFullscreen);
}

EditorAPI void ResetSettings ()
{
    SetWindowSize(iSettingsDefaultWindowWidth, iSettingsDefaultWindowHeight);
    EnableWindowFullscreen(iSettingsDefaultFullscreen);
}
