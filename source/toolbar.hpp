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

TEINAPI void  do_toolbar    ();
TEINAPI float get_toolbar_w ();

// Functionality for the various buttons/tools located on the toolbar.

TEINAPI void tb_set_tool_to_brush  ();
TEINAPI void tb_set_tool_to_fill   ();
TEINAPI void tb_set_tool_to_select ();
TEINAPI void tb_toggle_grid        ();
TEINAPI void tb_toggle_bounds      ();
TEINAPI void tb_toggle_layer_trans ();
TEINAPI void tb_reset_camera       ();
TEINAPI void tb_flip_level_h       ();
TEINAPI void tb_flip_level_v       ();
TEINAPI void tb_toggle_mirror_h    ();
TEINAPI void tb_toggle_mirror_v    ();
TEINAPI void tb_cut                ();
TEINAPI void tb_copy               ();
TEINAPI void tb_deselect           ();
TEINAPI void tb_clear_select       ();
TEINAPI void tb_resize             ();
TEINAPI void tb_toggle_entity      ();
TEINAPI void tb_toggle_guides      ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
