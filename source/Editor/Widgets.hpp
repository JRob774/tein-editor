#pragma once

#include "Utility.hpp"

#include <string>
#include <map>
#include <vector>

namespace TEIN
{
    class Widget // Base type for widgets, all custom widget types should inherit from this!
    {
    public:
        virtual ~Widget     () = default;
        virtual bool Create ();
        virtual void Delete ();
                void Update ();

        std::string m_Name;
        bool        m_Open;

    protected:
        virtual void InternalUpdate () = 0;
    };

    typedef U32 TileID;

    struct Tile
    {
        std::string name, tooltip;
        int layer;
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
        bool Create () override;
    private:
        void InternalUpdate () override;

        std::map<std::string,Tile>     m_Tiles;
        std::map<std::string,Category> m_Categories;
        std::vector<std::string>       m_CategoryList;
    };

    class ToolsWidget: public Widget
    {
    public:
        bool Create () override;
    private:
        void InternalUpdate () override;
    };

    class LayersWidget: public Widget
    {
    public:
        bool Create () override;
    private:
        void InternalUpdate () override;
    };

    class HistoryWidget: public Widget
    {
    public:
        bool Create () override;
    private:
        void InternalUpdate () override;
    };

    namespace WidgetManager
    {
        extern std::vector<Widget*> g_Widgets;

        bool Init   ();
        void Quit   ();
        void Update ();

        template<typename T>
        bool CreateWidget ()
        {
            g_Widgets.push_back(new T);
            return g_Widgets.back()->Create();
        }
    }
}
