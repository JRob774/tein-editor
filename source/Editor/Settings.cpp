EditorAPI void SaveSettings ()
{
    std::ofstream settings(gSettingsFile, std::ios::trunc);
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
    GonObject settings = GonObject::Load(gSettingsFile);
    gSettings.windowWidth = settings["window_width"].Int(gSettingsDefaultWindowWidth);
    gSettings.windowHeight = settings["window_height"].Int(gSettingsDefaultWindowHeight);
    gSettings.fullscreen = settings["fullscreen"].Bool(gSettingsDefaultFullscreen);
}

EditorAPI void ResetSettings ()
{
    SetWindowSize(gSettingsDefaultWindowWidth, gSettingsDefaultWindowHeight);
    EnableWindowFullscreen(gSettingsDefaultFullscreen);
}
