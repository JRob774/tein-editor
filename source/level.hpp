/*******************************************************************************
 * Loads and saves raw level data to and from the TEIN level file format.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float DEFAULT_LEVEL_WIDTH  = 54;
static constexpr float DEFAULT_LEVEL_HEIGHT = 32;
static constexpr int   MINIMUM_LEVEL_WIDTH  = 54;
static constexpr int   MINIMUM_LEVEL_HEIGHT = 32;
static constexpr int   MAXIMUM_LEVEL_WIDTH  = 2840;
static constexpr int   MAXIMUM_LEVEL_HEIGHT = 2840;

typedef u32 Level_Layer;

static constexpr Level_Layer LEVEL_LAYER_TAG     = 0;
static constexpr Level_Layer LEVEL_LAYER_OVERLAY = 1;
static constexpr Level_Layer LEVEL_LAYER_ACTIVE  = 2; // Combo of Basic and Entity.
static constexpr Level_Layer LEVEL_LAYER_BACK1   = 3;
static constexpr Level_Layer LEVEL_LAYER_BACK2   = 4;
static constexpr Level_Layer LEVEL_LAYER_TOTAL   = 5;

typedef s32 Tile_ID;

struct Level_Header
{
    s32 version;
    s32 width;
    s32 height;
    s32 layers;
};

typedef std::array<std::vector<Tile_ID>, LEVEL_LAYER_TOTAL> Level_Data;

struct Level
{
    Level_Header header;
    Level_Data data;
};

/* -------------------------------------------------------------------------- */

STDDEF bool load_level         (      Level& level, std::string file_name);
STDDEF bool save_level         (const Level& level, std::string file_name);

// A custom file format. Exactly the same as the default level format except
// the first part of the file until zero is the name of the level. This is
// done so that the name of the file can also be restored when the editor
// is loaded again after a fatal failure occurs and restore files are saved.

struct Tab; // Defined in <editor.hpp>

STDDEF bool load_restore_level (      Tab&   tab,   std::string file_name);
STDDEF bool save_restore_level (const Tab&   tab,   std::string file_name);



FILDEF bool create_blank_level (Level& level, int w = DEFAULT_LEVEL_WIDTH,
                                              int h = DEFAULT_LEVEL_HEIGHT);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
