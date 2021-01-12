//
// Base Widget
//

class Widget
{
public:
             Widget (std::string name);
    virtual ~Widget ();
    virtual bool Create ();
    virtual void Delete ();
            void Update ();

    std::string mName;
    bool        mOpen;

protected:
    virtual void InternalUpdate () = 0;
};

//
// Widget Manager
//

struct WidgetManager
{
    std::vector<Widget*> widgets;
};

Global WidgetManager gWidgetManager;

EditorAPI bool InitWidgetManager ();
EditorAPI void QuitWidgetManager ();

EditorAPI void UpdateWidgets ();

template<typename T>
EditorAPI bool CreateWidget ()
{
    gWidgetManager.widgets.push_back(new T);
    return gWidgetManager.widgets.back()->Create();
}

//
// Tiles Widget
//

struct Tile
{
    std::string name, tooltip;
    LevelLayer layer;
    std::vector<TileID> ids;
};

struct Category
{
    std::string name;
    std::vector<std::string> tiles;
};

class TilesWidget: public Widget
{
public:
     TilesWidget ();
    ~TilesWidget ();
    bool  Create () override;
private:
    std::map<std::string,Tile>     mTiles;
    std::map<std::string,Category> mCategories;
    std::vector<std::string>       mCategoryList;

    void InternalUpdate () override;
};

//
// Tools Widget
//

class ToolsWidget: public Widget
{
public:
     ToolsWidget ();
    ~ToolsWidget ();
private:
    void InternalUpdate () override;
};

//
// Layers Widget
//

class LayersWidget: public Widget
{
public:
     LayersWidget ();
    ~LayersWidget ();
private:
    void InternalUpdate () override;
};

//
// History Widget
//

class HistoryWidget: public Widget
{
public:
     HistoryWidget ();
    ~HistoryWidget ();
private:
    void InternalUpdate () override;
};
