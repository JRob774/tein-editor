/*******************************************************************************
 * Editor GUI widget containing level/map editor specific actions and tools.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float TOOLBAR_INNER_PAD =  1;
static constexpr float TOOLBAR_WIDTH     = 30;

FILDEF void  do_toolbar    ();
FILDEF float get_toolbar_w ();

// Functionality for the various buttons/tools located on the toolbar.

FILDEF void tb_set_tool_to_brush  ();
FILDEF void tb_set_tool_to_fill   ();
FILDEF void tb_set_tool_to_select ();
FILDEF void tb_toggle_grid        ();
FILDEF void tb_toggle_bounds      ();
FILDEF void tb_toggle_layer_trans ();
FILDEF void tb_reset_camera       ();
FILDEF void tb_flip_level_h       ();
FILDEF void tb_flip_level_v       ();
FILDEF void tb_toggle_mirror_h    ();
FILDEF void tb_toggle_mirror_v    ();
FILDEF void tb_cut                ();
FILDEF void tb_copy               ();
FILDEF void tb_deselect           ();
FILDEF void tb_clear_select       ();
FILDEF void tb_resize             ();
FILDEF void tb_toggle_entity      ();
FILDEF void tb_toggle_guides      ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
