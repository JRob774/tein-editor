enum class ToolState { Idle, Place, Erase };
enum class ToolType { Brush, Fill, Select };

struct Tab;

struct ToolFill
{
    std::vector<Vec2> frontier;
    std::vector<bool> searched;

    LevelLayer layer;

    TileID findID;
    TileID replaceID;

    Vec2 start;

    bool insideSelect;
};

struct SelectBounds
{
    int top;
    int right;
    int bottom;
    int left;

    bool visible;
};

struct ToolSelect
{
    std::vector<SelectBounds> bounds;

    bool start;
    bool add;

    size_t cachedSize;
};

struct ToolInfo
{
    ToolFill fill;
    ToolSelect select;
};

enum class LevelHistoryAction { Normal, FlipLevelH, FlipLevelV, SelectState, Clear, Resize };

struct LevelHistoryInfo
{
    int x;
    int y;

    TileID oldID;
    TileID newID;

    LevelLayer tileLayer;
};

struct LevelHistoryState
{
    LevelHistoryAction action;

    std::vector<LevelHistoryInfo> info;

    // What layers were active at the time. Used by flips so only those
    // layers end up getting flipped during the undo and redo actions.
    bool tileLayerActive[static_cast<int>(LevelLayer::Total)];

    // Used by the select box history to know the size and position.
    std::vector<SelectBounds> oldSelectState;
    std::vector<SelectBounds> newSelectState;

    // Used by the resizing history to restore old/new size.
    ResizeDir resizeDir;

    int oldWidth;
    int oldHeight;

    int newWidth;
    int newHeight;

    // The data of the level before and after a resize.
    LevelData oldData;
    LevelData newData;
};

struct LevelHistory
{
    int currentPosition;
    std::vector<LevelHistoryState> state;
};

static constexpr float gDefaultTileSize = 16;
static constexpr float gDefaultTileSizeHalf = gDefaultTileSize / 2;

struct LevelClipboard
{
    LevelData data;

    int x;
    int y;
    int w;
    int h;
};

struct LevelEditor
{
    ToolState toolState = ToolState::Idle;
    ToolType toolType = ToolType::Brush;

    std::vector<LevelClipboard> clipboard;

    Vec2 mouseWorld;
    Vec2 mouse;
    Vec2 mouseTile;

    bool boundsVisible;
    bool layerTransparency;
    bool largeTiles = true;
    bool entityGuides;

    bool mirrorH;
    bool mirrorV;

    Quad bounds;
    Quad viewport;
};

static LevelEditor gLevelEditor;

TEINAPI void InitLevelEditor ();
TEINAPI void DoLevelEditor ();

TEINAPI void HandleLevelEditorEvents ();

TEINAPI bool MouseInsideLevelEditorViewport ();

TEINAPI void NewLevelHistoryState (LevelHistoryAction action);

TEINAPI void AddToHistoryNormalState (LevelHistoryInfo info);
TEINAPI void AddToHistoryClearState (LevelHistoryInfo info);

TEINAPI bool AreAllLayersInactive ();

TEINAPI bool AreAnySelectBoxesVisible ();
TEINAPI void GetOrderedSelectBounds (const SelectBounds& bounds, int* l, int* t, int* r, int* b);
TEINAPI void GetTotalSelectBoundary (int* l, int* t, int* r, int* b);

TEINAPI void LoadLevelTab (std::string fileName);

TEINAPI bool LevelEditorSave (Tab& tab);
TEINAPI bool LevelEditorSaveAs ();
TEINAPI void LevelEditorClearSelect ();
TEINAPI void LevelEditorDeselect ();
TEINAPI void LevelEditorCursorDeseelct ();
TEINAPI void LevelEditorSelectAll ();
TEINAPI void LevelEditorCopy ();
TEINAPI void LevelEditorCut ();
TEINAPI void LevelEditorPaste ();

TEINAPI void FlipLevelH ();
TEINAPI void FlipLevelV ();

TEINAPI void LevelHasUnsavedChanges ();

TEINAPI void LevelEditorUndo ();
TEINAPI void LevelEditorRedo ();

TEINAPI void LevelEditorHistoryBegin ();
TEINAPI void LevelEditorHistoryEnd ();

TEINAPI void LevelEditorResize ();
TEINAPI void LevelEditorResizeOkay ();

TEINAPI void LevelEditorLoadPrevLevel ();
TEINAPI void LevelEditorLoadNextLevel ();

TEINAPI void LevelDropFile (Tab* tab, std::string fileName);

TEINAPI void BackupLevelTab (const Level& level, const std::string& fileName);

TEINAPI bool IsCurrentLevelEmpty ();
