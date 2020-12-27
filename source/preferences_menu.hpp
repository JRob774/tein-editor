/*******************************************************************************
 * Menu that allows for the modification of application settings and hotkeys.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

FILDEF void init_preferences_menu ();
FILDEF void do_preferences_menu   ();

FILDEF void handle_preferences_menu_events ();

FILDEF void restore_preferences ();
FILDEF void cancel_preferences  ();
FILDEF void save_preferences    ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
