/*******************************************************************************
 * System that checks for updated versions and displays the changes/features.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

FILDEF void check_for_updates        ();
FILDEF void open_update_window_timed ();
FILDEF void open_update_window       ();
FILDEF void do_update                ();
FILDEF void handle_update_events     ();
FILDEF bool are_there_updates        ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
