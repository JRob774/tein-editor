#pragma once

static constexpr float gMinLevelEditorZoom = .25f;
static constexpr float gMaxLevelEditorZoom = 4;

static constexpr float gMinMapEditorZoom = .125f;
static constexpr float gMaxMapEditorZoom = 1;

static constexpr float gEditorZoomIncrement = .1f;

struct Camera
{
    float x,y;
    float zoom;
};

static constexpr size_t gInvalidTab = static_cast<size_t>(-1);

enum class TabType { LEVEL, MAP };

struct Tab
{
    // GENERAL EDITOR
    TabType type;
    std::string name;
    Camera camera;
    bool unsavedChanges;
    // LEVEL EDITOR
    Level level;
    ToolInfo toolInfo;
    LevelHistory levelHistory;
    bool tileLayerActive[LEVEL_LAYER_TOTAL];
    std::vector<SelectBounds> oldSelectState; // We use this for the selection history undo/redo system.
    // MAP EDITOR
    Map map;
    MapHistory mapHistory;
    MapSelect mapSelect;
    MapNodeInfo mapNodeInfo;
};

struct Editor
{
    // Names of recently closed tabs so that they can be restored.
    std::vector<std::string> closedTabs;

    std::vector<Tab> tabs;
    size_t currentTab;

    SDL_TimerID backupTimer;
    SDL_TimerID cooldownTimer;

    bool gridVisible;
    bool isPanning;
    bool dialogBox; // NOTE: See <dialog.cpp> for information.
};

static Editor gEditor;

TEINAPI void InitEditor (int argc, char** argv);
TEINAPI void QuitEditor ();

TEINAPI void DoEditor ();

TEINAPI void HandleEditorEvents ();

TEINAPI void UpdateBackupTimer ();

TEINAPI void SetCurrentTab (size_t index);
TEINAPI Tab& GetCurrentTab ();
TEINAPI Tab& GetTabAtIndex (size_t index);

TEINAPI bool AreThereAnyTabs ();

TEINAPI void IncrementTab ();
TEINAPI void DecrementTab ();

TEINAPI void SetMainWindowSubtitleForTab (const std::string& subtitle);

TEINAPI bool AreThereAnyLevelTabs ();
TEINAPI bool AreThereAnyMapTabs ();

TEINAPI void CreateNewLevelTabAndFocus (int w = gDefaultLevelWidth, int h = gDefaultLevelHeight);
TEINAPI void CreateNewMapTabAndFocus ();

TEINAPI bool CurrentTabIsLevel ();
TEINAPI bool CurrentTabIsMap ();

TEINAPI void CloseTab (size_t index);
TEINAPI void CloseCurrentTab ();
TEINAPI void CloseAllTabs ();

TEINAPI size_t GetTabIndexWithThisFileName (std::string fileName);

TEINAPI void PushEditorCameraTransform ();
TEINAPI void PopEditorCameraTransform ();

TEINAPI int SaveChangesPrompt (Tab& tab);

TEINAPI void BackupTab (Tab& tab);

TEINAPI bool IsCurrentTabEmpty ();

TEINAPI void EditorSelectAll ();
TEINAPI void EditorPaste ();

TEINAPI bool SavePromptAllEditorTabs ();

TEINAPI void OpenRecentlyClosedTab ();

TEINAPI void SaveRestoreFiles ();
