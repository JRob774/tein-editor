/*******************************************************************************
 * Editor GUI widget allowing for level layers to be toggled on/off.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

TEINAPI void init_layer_panel ();
TEINAPI void do_layer_panel   (bool scrollbar);

TEINAPI float get_layer_panel_height ();

TEINAPI bool layer_panel_needs_scrollbar ();
TEINAPI bool is_layer_panel_present      ();

TEINAPI void toggle_layer_active  ();
TEINAPI void toggle_layer_tag     ();
TEINAPI void toggle_layer_overlay ();
TEINAPI void toggle_layer_back1   ();
TEINAPI void toggle_layer_back2   ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
