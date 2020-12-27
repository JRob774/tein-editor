/*******************************************************************************
 * Editor GUI widget containing a palette of all the game's tiles.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float TILE_IMAGE_SIZE = 64;

typedef int Tile_Category;

static constexpr Tile_Category TILE_CATEGORY_BASIC   = 0;
static constexpr Tile_Category TILE_CATEGORY_TAG     = 1;
static constexpr Tile_Category TILE_CATEGORY_OVERLAY = 2;
static constexpr Tile_Category TILE_CATEGORY_ENTITY  = 3;
static constexpr Tile_Category TILE_CATEGORY_BACK1   = 4;
static constexpr Tile_Category TILE_CATEGORY_BACK2   = 5;
static constexpr Tile_Category TILE_CATEGORY_TOTAL   = 6;

/* -------------------------------------------------------------------------- */

TEINAPI bool init_tile_panel ();
TEINAPI void do_tile_panel   (bool scrollbar);

TEINAPI bool tile_panel_needs_scrollbar ();

TEINAPI void handle_tile_panel_events ();

TEINAPI float get_tile_panel_height ();

TEINAPI void reload_tile_graphics ();

TEINAPI Tile_Category get_selected_category ();
TEINAPI Tile_ID       get_selected_tile     ();
TEINAPI Level_Layer   get_selected_layer    ();

TEINAPI Level_Layer category_to_layer (Tile_Category category);

TEINAPI void select_next_active_group ();
TEINAPI void select_prev_active_group ();

TEINAPI void reset_selected_group ();

TEINAPI void increment_selected_tile     ();
TEINAPI void decrement_selected_tile     ();
TEINAPI void increment_selected_group    ();
TEINAPI void decrement_selected_group    ();
TEINAPI void increment_selected_category ();
TEINAPI void decrement_selected_category ();

TEINAPI Tile_ID get_tile_horizontal_flip (Tile_ID id);
TEINAPI Tile_ID get_tile_vertical_flip   (Tile_ID id);

TEINAPI void jump_to_category_basic   ();
TEINAPI void jump_to_category_tag     ();
TEINAPI void jump_to_category_overlay ();
TEINAPI void jump_to_category_entity  ();
TEINAPI void jump_to_category_back1   ();
TEINAPI void jump_to_category_back2   ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
