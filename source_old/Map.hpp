struct MapNode
{
    int x,y;
    std::string lvl;
};

typedef std::vector<MapNode> Map;

TEINAPI bool LoadMap (Tab& tab, std::string fileName);
TEINAPI bool SaveMap (const Tab& tab, std::string fileName);

struct Tab;

// A custom file format. Exactly the same as the default world format except
// the first part of the file until zero is the name of the level. This is
// done so that the name of the file can also be restored when the editor
// is loaded again after a fatal failure occurs and restore files are saved.
TEINAPI bool LoadRestoreMap (Tab& tab, std::string fileName);
TEINAPI bool SaveRestoreMap (const Tab& tab, std::string fileName);

TEINAPI int GetMapXPos   (const Map& map);
TEINAPI int GetMapYPos   (const Map& map);
TEINAPI int GetMapWidth  (const Map& map);
TEINAPI int GetMapHeight (const Map& map);
