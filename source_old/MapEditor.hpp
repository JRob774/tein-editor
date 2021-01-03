static constexpr float gMapEditorTextCutOff = .5f;
static constexpr float gMapEditorTextPad = 3;

static constexpr float gMapNodeWidth = 88;
static constexpr float gMapNodeHeight = 22;

struct MapHistory
{
    int currentPosition;
    std::vector<Map> state;
};

struct MapSelect
{
    IVec2 a; // Start
    IVec2 b; // End
};

typedef std::vector<MapNode> MapClipboard;

struct MapNodeInfo
{
    std::string cachedLevelText;

    IVec2 pressedNodePos;
    IVec2 activePos;
    MapNode* active;

    size_t cursor;
    size_t select; // Start point of text selection.
    bool selecting;
};

struct MapEditor
{
    struct ColorMapCompare
    {
        inline bool operator() (const Vec4& lhs, const Vec4& rhs) const
        {
            return ((lhs.x+lhs.y+lhs.z+lhs.w) < (rhs.x+rhs.y+rhs.z+rhs.w));
        }
    };

    MapClipboard clipboard;

    Vec2 mouseWorld;
    Vec2 mouse;
    Vec2 mouseTile;

    bool pressed;
    bool leftPressed;

    SDL_TimerID cursorBlinkTimer;
    bool cursorVisible;

    Quad bounds;
    Quad viewport;

    std::map<Vec4,Vec4, ColorMapCompare> textColorMap;
};

static MapEditor gMapEditor;

TEINAPI void InitMapEditor ();
TEINAPI void DoMapEditor ();

TEINAPI void HandleMapEditorEvents ();

TEINAPI void LoadMapTab (std::string fileName);
TEINAPI bool SaveMapTab (Tab& tab);
TEINAPI void SaveMapTabAs ();

TEINAPI void MapDropFile (Tab* tab, std::string fileName);

TEINAPI void BackupMapTab (const Tab& tab, const std::string& fileName);

TEINAPI bool IsCurrentMapEmpty ();

TEINAPI float GetMinMapBoundsX ();
TEINAPI float GetMinMapBoundsY ();
TEINAPI float GetMaxMapBoundsX ();
TEINAPI float GetMaxMapBoundsY ();

TEINAPI void MapEditorCut ();
TEINAPI void MapEditorCopy ();
TEINAPI void MapEditorPaste ();

TEINAPI void MapEditorDeselect ();
TEINAPI void MapEditorClearSelect ();
TEINAPI void MapEditorSelectAll ();

TEINAPI void MapEditorUndo ();
TEINAPI void MapEditorRedo ();

TEINAPI void MapEditorHistoryBegin ();
TEINAPI void MapEditorHistoryEnd ();

TEINAPI void NewMapHistoryState (Map& map);

TEINAPI bool MapSelectBoxPresent ();
TEINAPI void GetMapSelectBounds (int* l, int* t, int* r, int* b);
