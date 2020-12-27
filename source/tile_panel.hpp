/*******************************************************************************
 * Editor GUI widget containing a palette of all the game's tiles.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#ifndef TILE_PANEL_HPP__ /*///////////////////////////////////////////////////*/
#define TILE_PANEL_HPP__

#ifdef COMPILER_HAS_PRAGMA_ONCE
#pragma once
#endif

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

FILDEF bool init_tile_panel ();
FILDEF void do_tile_panel   (bool scrollbar);

FILDEF bool tile_panel_needs_scrollbar ();

FILDEF void handle_tile_panel_events ();

FILDEF float get_tile_panel_height ();

FILDEF void reload_tile_graphics ();

FILDEF Tile_Category get_selected_category ();
FILDEF Tile_ID       get_selected_tile     ();
FILDEF Level_Layer   get_selected_layer    ();

FILDEF Level_Layer category_to_layer (Tile_Category category);

STDDEF void select_next_active_group ();
STDDEF void select_prev_active_group ();

FILDEF void reset_selected_group ();

FILDEF void increment_selected_tile     ();
FILDEF void decrement_selected_tile     ();
FILDEF void increment_selected_group    ();
FILDEF void decrement_selected_group    ();
FILDEF void increment_selected_category ();
FILDEF void decrement_selected_category ();

FILDEF Tile_ID get_tile_horizontal_flip (Tile_ID id);
FILDEF Tile_ID get_tile_vertical_flip   (Tile_ID id);

FILDEF void jump_to_category_basic   ();
FILDEF void jump_to_category_tag     ();
FILDEF void jump_to_category_overlay ();
FILDEF void jump_to_category_entity  ();
FILDEF void jump_to_category_back1   ();
FILDEF void jump_to_category_back2   ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

#endif /* TILE_PANEL_HPP__ ///////////////////////////////////////////////////*/

/*******************************************************************************
 *
 * Copyright (c) 2020 Joshua Robertson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
*******************************************************************************/
