/*******************************************************************************
 * Loads and saves raw world map data ot and from the CSV file format.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

struct Map_Node
{
    int x;
    int y;

    std::string lvl;
};

typedef std::vector<Map_Node> Map;

/* -------------------------------------------------------------------------- */

TEINAPI bool load_map         (      Tab& tab, std::string file_name);
TEINAPI bool save_map         (const Tab& tab, std::string file_name);

// A custom file format. Exactly the same as the default world format except
// the first part of the file until zero is the name of the level. This is
// done so that the name of the file can also be restored when the editor
// is loaded again after a fatal failure occurs and restore files are saved.

struct Tab; // Defined in <editor.hpp>

TEINAPI bool load_restore_map (      Tab& tab, std::string file_name);
TEINAPI bool save_restore_map (const Tab& tab, std::string file_name);

/* -------------------------------------------------------------------------- */

TEINAPI int  get_map_x_pos    (const Map& map);
TEINAPI int  get_map_y_pos    (const Map& map);
TEINAPI int  get_map_width    (const Map& map);
TEINAPI int  get_map_height   (const Map& map);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
