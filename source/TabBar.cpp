static constexpr float gDefaultLevelTabWidth = 128;
static constexpr float gShiftTabButtonWidth = 13;

static constexpr size_t gNoTabToClose = static_cast<size_t>(-1);

static size_t gStartingTabOffset = 0;
static size_t gMaxNumberOfTabs = 0;

static bool gNeedToScrollTabBar;

static bool gCanScrollInTabBar = false;

namespace Internal
{
    TEINAPI bool DoLevelTab (float w, const Tab& tab, size_t index, bool current)
    {
        bool shouldClose = false;

        float xPad = 6;

        float tw = w;
        float th = GetPanelHeight();

        Vec2 cursor1(0,0);
        Vec2 cursor2(xPad,0);

        float bw = 24;
        float pw = tw-(bw);
        float lw = pw-(xPad*2);

        std::string name((!tab.name.empty()) ? StripFilePath(tab.name) : "Untitled");
        // We insert at the start so that it is always visible even if the
        // level's name gets cut off by the width of the final level tab.
        if (tab.unsavedChanges) name.insert(0, "* ");

        UiFlag flags = ((current) ? UI_HIGHLIGHT : UI_INACTIVE);
        BeginPanel(GetPanelCursor().x,0,tw,th, flags, gUiColorMedium);

        SetPanelCursorDir(UI_DIR_DOWN);
        SetPanelCursor(&cursor1);

        // We display the level tab's full file name in the status bar on hover.
        std::string info((tab.name.empty()) ? "Untitled" : tab.name);
        if (BeginClickPanelGradient(NULL, pw,th+1.0f, flags, info))
        {
            SetCurrentTab(index);
        }

        SetPanelCursorDir(UI_DIR_RIGHT);
        SetPanelCursor(&cursor2);

        DoLabel(UI_ALIGN_LEFT, UI_ALIGN_CENTER, lw,th, name);

        EndPanel();

        cursor1.x += pw;
        cursor1.y = 0.0f;

        if (DoImageButtonGradient(NULL, bw,th+1, flags, &gClipCross, info)) shouldClose = true;

        EndPanel();

        AdvancePanelCursor(tw+1);
        return shouldClose;
    }
}

TEINAPI void HandleTabBarEvents ()
{
    switch (gMainEvent.type)
    {
        case (SDL_MOUSEWHEEL):
        {
            if (gCanScrollInTabBar)
            {
                if (gMainEvent.wheel.y > 0) IncrementTab();
                if (gMainEvent.wheel.y < 0) DecrementTab();
            }
        } break;
    }
}

TEINAPI void DoTabBar ()
{
    float x = GetToolbarWidth() + 1;
    float y = 0;

    float bw = gShiftTabButtonWidth;
    float bh = gTabBarHeight;

    SetUiTexture(&gResourceIcons);
    SetUiFont(&GetEditorRegularFont());

    float wholeTabBarWidth = GetViewport().w - GetToolbarWidth() - GetControlPanelWidth();

    float pw = wholeTabBarWidth - (bw*2) - 4;
    float ph = gTabBarHeight;

    // To account for the control panel disappearing.
    if (!CurrentTabIsLevel()) pw += 1;

    // Figure out how many tabs we can fit on the bar before we need to start scrolling.
    gMaxNumberOfTabs = static_cast<int>(ceilf(pw / (gDefaultLevelTabWidth + 1)));

    if (gNeedToScrollTabBar)
    {
        gNeedToScrollTabBar = false;
        MaybeScrollTabBar();
    }

    float tabWidth = gDefaultLevelTabWidth;
    float leftover = 0;

    if (gEditor.tabs.size() >= gMaxNumberOfTabs)
    {
        tabWidth = floorf((pw-((gMaxNumberOfTabs-1)*1)) / gMaxNumberOfTabs);
        leftover = (pw-((gMaxNumberOfTabs-1)*1)) - (tabWidth * gMaxNumberOfTabs);
    }
    else
    {
        gStartingTabOffset = 0;
    }

    // Prevents the tab bar from being offset too far to the right creating an ugly space when there shouldn't be.
    if (gEditor.tabs.size() >= gMaxNumberOfTabs)
    {
        while (gStartingTabOffset+gMaxNumberOfTabs > gEditor.tabs.size())
        {
             --gStartingTabOffset;
        }
    }

    // THE LEFT ARROW BUTTON
    if (AreThereAnyTabs())
    {
        BeginPanel(x,y,bw,bh, UI_NONE);
        Vec2 tempCursor(0,0);
        SetPanelCursor(&tempCursor);
        bool leftArrowActive = (gStartingTabOffset != 0);
        UiFlag flags = ((leftArrowActive) ? UI_NONE : UI_LOCKED);
        if (DoImageButton(NULL, bw+1,bh, flags, &gClipArrowLeft))
        {
            --gStartingTabOffset;
        }
        EndPanel();
    }

    // THE LIST OF TABS
    Vec2 cursor(0,0);

    Vec4 color = ((AreThereAnyTabs()) ? gUiColorMedDark : gUiColorExDark);
    BeginPanel(x+bw+1,y,pw,ph, UI_NONE, color);

    SetPanelCursorDir(UI_DIR_RIGHT);
    SetPanelCursor(&cursor);

    // Check to see if the mouse is in the panel, if it is then the mouse scroll wheel will scroll through tabs.
    gCanScrollInTabBar = (MouseInUiBoundsXYWH(0,0,pw,ph) && IsKeyModStateActive(KMOD_NONE));

    size_t indexToClose = gNoTabToClose;
    size_t last = std::min(gEditor.tabs.size(), gStartingTabOffset+gMaxNumberOfTabs);
    for (size_t i=gStartingTabOffset; i<last; ++i)
    {
        float w = tabWidth + ((i == last-1) ? leftover : 0);
        bool current = (i == gEditor.currentTab);
        if (Internal::DoLevelTab(w, gEditor.tabs.at(i), i, current))
        {
            indexToClose = i;
        }
    }

    EndPanel();

    // THE RIGHT ARROW BUTTON
    if (AreThereAnyTabs())
    {
        BeginPanel(x+bw+2+pw,0,bw,bh, UI_NONE);
        Vec2 tempCursor(0,0);
        SetPanelCursor(&tempCursor);
        bool rightArrowActive = (gStartingTabOffset+gMaxNumberOfTabs < gEditor.tabs.size());
        UiFlag flags = ((rightArrowActive) ? UI_NONE : UI_LOCKED);
        if (DoImageButton(NULL, bw+1,bh, flags, &gClipArrowRight))
        {
            ++gStartingTabOffset;
        }
        EndPanel();
    }

    // If a level needs to be closed then we do it now.
    if (indexToClose != gNoTabToClose) CloseTab(indexToClose);
}

TEINAPI void MaybeScrollTabBar ()
{
    if (gEditor.currentTab < gStartingTabOffset) gStartingTabOffset = gEditor.currentTab;
    while (gEditor.currentTab >= std::min(gEditor.tabs.size(), gStartingTabOffset+gMaxNumberOfTabs))
    {
        ++gStartingTabOffset;
    }
}

TEINAPI void MoveTabLeft ()
{
    if (AreThereAnyTabs())
    {
        Tab& tab = GetCurrentTab();
        if (tab.type != TabType::MAP || !tab.mapNodeInfo.active)
        {
            if (gEditor.currentTab > 0)
            {
                auto begin = gEditor.tabs.begin();
                std::iter_swap(begin+gEditor.currentTab-1, begin+gEditor.currentTab);
                --gEditor.currentTab;
                MaybeScrollTabBar();
            }
        }
    }
}
TEINAPI void MoveTabRight ()
{
    if (AreThereAnyTabs())
    {
        Tab& tab = GetCurrentTab();
        if (tab.type != TabType::MAP || !tab.mapNodeInfo.active)
        {
            if (gEditor.currentTab < gEditor.tabs.size()-1)
            {
                auto begin = gEditor.tabs.begin();
                std::iter_swap(begin+gEditor.currentTab+1, begin+gEditor.currentTab);
                ++gEditor.currentTab;
                MaybeScrollTabBar();
            }
        }
    }
}

TEINAPI void NeedToScrollNextUpdate ()
{
    gNeedToScrollTabBar = true;
}

TEINAPI bool MouseIsOverTabBar ()
{
    return gCanScrollInTabBar;
}
