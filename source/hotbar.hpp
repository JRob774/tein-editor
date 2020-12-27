/*******************************************************************************
 * Editor GUI widget containing global editor actions/buttons.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float HOTBAR_HEIGHT = 26;

FILDEF void do_hotbar ();

// Functionality for the various commands/actions located on the hotbar.

FILDEF void hb_new           ();
FILDEF void hb_load          ();
FILDEF void hb_save          ();
FILDEF void hb_save_as       ();
FILDEF void hb_undo_action   ();
FILDEF void hb_redo_action   ();
FILDEF void hb_history_begin ();
FILDEF void hb_history_end   ();
FILDEF void hb_zoom_out      ();
FILDEF void hb_zoom_in       ();
FILDEF void hb_gpak_unpack   ();
FILDEF void hb_gpak_pack     ();
FILDEF void hb_run_game      ();
FILDEF void hb_preferences   ();
FILDEF void hb_about         ();
FILDEF void hb_bug_report    ();
FILDEF void hb_help          ();
FILDEF void hb_update        ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
