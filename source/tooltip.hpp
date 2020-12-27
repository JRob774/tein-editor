/*******************************************************************************
 * System for tooltips that appear when hovering on certain GUI elements.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

STDDEF void set_current_tooltip   (std::string name, std::string desc = "");
FILDEF void do_tooltip            ();
FILDEF void handle_tooltip_events ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
