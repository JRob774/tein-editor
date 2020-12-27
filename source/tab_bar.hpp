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

TEINAPI void handle_tab_bar_events ();

TEINAPI void do_tab_bar ();

TEINAPI void maybe_scroll_tab_bar ();

TEINAPI void move_tab_left  ();
TEINAPI void move_tab_right ();

TEINAPI void need_to_scroll_next_update ();

TEINAPI bool mouse_is_over_tab_bar ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
