TEINAPI void DoAbout ()
{
    SetUiFont(&GetEditorRegularFont());

    BeginPanel(gWindowBorder,gWindowBorder,GetViewport().w-(gWindowBorder*2),GetViewport().h-(gWindowBorder*2), UiFlag::None, gUiColorExDark);
    BeginPanel(1,1,GetViewport().w-2,GetViewport().h-2, UiFlag::None, gUiColorMedium);

    constexpr float XPad = 4;
    constexpr float YPad = 4;

    Vec2 cursor(XPad, YPad);

    SetPanelCursorDir(UiDir::Down);
    SetPanelCursor(&cursor);

    #if defined(BUILD_DEBUG)
    const char* build = "Debug";
    #else
    const char* build = "Release";
    #endif // BUILD_DEBUG

    float lw = GetViewport().w - (XPad*2);
    float lh = 18;

    constexpr const char* Changelog = "https://github.com/thatb0y/tein-editor/blob/master/CHANGES.md";
    std::string changelogURL(FormatString("%s#v%d.%d.%d", Changelog, gAppVerMajor,gAppVerMinor,gAppVerPatch));

    DoLabelHyperlink(UiAlign::Left,UiAlign::Center, lw,lh, FormatString("The End is Nigh Editor (v%d.%d.%d) %s Build ", gAppVerMajor,gAppVerMinor,gAppVerPatch, build), "(Changelog)", changelogURL);
    DoLabel(UiAlign::Left,UiAlign::Center, lw,lh, "Copyright (C) 2020 Joshua Robertson");
    AdvancePanelCursor(lh/2);
    DoLabelHyperlink(UiAlign::Left,UiAlign::Center, lw,lh, "Flame icon made by Those Icons from ", "Flaticon", "https://www.flaticon.com/");
    DoLabel(UiAlign::Left,UiAlign::Center, lw,lh, "Editor icon made by Radley McTuneston");

    EndPanel();
    EndPanel();
}

TEINAPI void HandleAboutEvents ()
{
    if (!IsWindowFocused("WINABOUT")) return;

    if (gMainEvent.type == SDL_KEYDOWN) {
        if (gMainEvent.key.keysym.sym == SDLK_ESCAPE || gMainEvent.key.keysym.sym == SDLK_RETURN) {
            HideWindow("WINABOUT");
        }
    }
}
