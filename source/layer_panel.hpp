/*******************************************************************************
 * Editor GUI widget allowing for level layers to be toggled on/off.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

FILDEF void init_layer_panel ();
FILDEF void do_layer_panel   (bool scrollbar);

FILDEF float get_layer_panel_height ();

FILDEF bool layer_panel_needs_scrollbar ();
FILDEF bool is_layer_panel_present      ();

FILDEF void toggle_layer_active  ();
FILDEF void toggle_layer_tag     ();
FILDEF void toggle_layer_overlay ();
FILDEF void toggle_layer_back1   ();
FILDEF void toggle_layer_back2   ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
