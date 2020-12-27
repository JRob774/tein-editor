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

TEINAPI void do_hotbar ();

// Functionality for the various commands/actions located on the hotbar.

TEINAPI void hb_new           ();
TEINAPI void hb_load          ();
TEINAPI void hb_save          ();
TEINAPI void hb_save_as       ();
TEINAPI void hb_undo_action   ();
TEINAPI void hb_redo_action   ();
TEINAPI void hb_history_begin ();
TEINAPI void hb_history_end   ();
TEINAPI void hb_zoom_out      ();
TEINAPI void hb_zoom_in       ();
TEINAPI void hb_gpak_unpack   ();
TEINAPI void hb_gpak_pack     ();
TEINAPI void hb_run_game      ();
TEINAPI void hb_preferences   ();
TEINAPI void hb_about         ();
TEINAPI void hb_bug_report    ();
TEINAPI void hb_help          ();
TEINAPI void hb_update        ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
