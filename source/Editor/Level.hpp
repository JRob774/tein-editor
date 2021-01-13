Global constexpr float gDefaultLevelWidth = 54;
Global constexpr float gDefaultLevelHeight = 32;

enum class LevelLayer: U32 { Tag, Overlay, Active, Back1, Back2, Total };

typedef S32 TileID;

struct LevelHeader
{
    S32 version;
    S32 width;
    S32 height;
    S32 layers;
};

typedef std::array<std::vector<TileID>, static_cast<int>(LevelLayer::Total)> LevelData;

struct Level
{
    LevelHeader header;
    LevelData data;
};

EditorAPI bool CreateBlankLevel (Level& level, int w = gDefaultLevelWidth, int h = gDefaultLevelHeight);

EditorAPI bool LoadLevel (Level& level, std::string fileName);
EditorAPI bool SaveLevel (const Level& level, std::string fileName);
