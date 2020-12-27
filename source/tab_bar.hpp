/*******************************************************************************
 * GUI widget that showcases the currently opened level/map tabs in the editor.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float TAB_BAR_HEIGHT = 25;

FILDEF void handle_tab_bar_events ();

FILDEF void do_tab_bar ();

FILDEF void maybe_scroll_tab_bar ();

FILDEF void move_tab_left  ();
FILDEF void move_tab_right ();

FILDEF void need_to_scroll_next_update ();

FILDEF bool mouse_is_over_tab_bar ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
