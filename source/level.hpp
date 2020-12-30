#pragma once

static constexpr float gDefaultLevelWidth  = 54;
static constexpr float gDefaultLevelHeight = 32;
static constexpr int   gMinimumLevelWidth  = 54;
static constexpr int   gMinimumLevelHeight = 32;
static constexpr int   gMaximumLevelWidth  = 2840;
static constexpr int   gMaximumLevelHeight = 2840;

enum LevelLayer: U32
{
    LEVEL_LAYER_TAG,
    LEVEL_LAYER_OVERLAY,
    LEVEL_LAYER_ACTIVE, // Combo of Basic and Entity.
    LEVEL_LAYER_BACK1,
    LEVEL_LAYER_BACK2,
    LEVEL_LAYER_TOTAL
};

typedef S32 TileID;

struct LevelHeader
{
    S32 version;
    S32 width;
    S32 height;
    S32 layers;
};

typedef std::array<std::vector<TileID>, LEVEL_LAYER_TOTAL> LevelData;

struct Level
{
    LevelHeader header;
    LevelData data;
};

TEINAPI bool CreateBlankLevel (Level& level, int w = gDefaultLevelWidth, int h = gDefaultLevelHeight);

TEINAPI bool LoadLevel (Level& level, std::string fileName);
TEINAPI bool SaveLevel (const Level& level, std::string fileName);

struct Tab;

// A custom file format. Exactly the same as the default level format except
// the first part of the file until zero is the name of the level. This is
// done so that the name of the file can also be restored when the editor
// is loaded again after a fatal failure occurs and restore files are saved.
TEINAPI bool LoadRestoreLevel (Tab& tab, std::string fileName);
TEINAPI bool SaveRestoreLevel (const Tab& tab, std::string fileName);
