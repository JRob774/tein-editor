/*******************************************************************************
 * System for tooltips that appear when hovering on certain GUI elements.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

TEINAPI void set_current_tooltip   (std::string name, std::string desc = "");
TEINAPI void do_tooltip            ();
TEINAPI void handle_tooltip_events ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
