/*******************************************************************************
 * Menu that allows for the modification of application settings and hotkeys.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

TEINAPI void init_preferences_menu ();
TEINAPI void do_preferences_menu   ();

TEINAPI void handle_preferences_menu_events ();

TEINAPI void restore_preferences ();
TEINAPI void cancel_preferences  ();
TEINAPI void save_preferences    ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
