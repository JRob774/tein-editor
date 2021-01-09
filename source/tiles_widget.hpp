typedef S32 TileID;

struct Tile
{
    std::string name, tooltip;
    int layer; // @Improve; Make an enum for this??
    std::vector<TileID> ids;
};

struct Category
{
    std::string name;
    std::vector<std::string> tiles; // These strings point into the gTilesWidget.tiles map.
};

struct TilesWidget
{
    std::map<std::string,Tile> tiles;
    std::map<std::string,Category> categories;

    std::vector<std::string> categoryList; // These strings point into the gTilesWidget.categories map.

    bool open;
};

Global TilesWidget gTilesWidget;

EditorAPI bool InitTilesWidget ();
EditorAPI void QuitTilesWidget ();
EditorAPI void DoTilesWidget ();
EditorAPI bool& GetTilesWidgetOpen ();
