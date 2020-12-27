/*******************************************************************************
 * Editor GUI widget containing global editor actions/buttons.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#ifndef HOTBAR_HPP__ /*///////////////////////////////////////////////////////*/
#define HOTBAR_HPP__

#ifdef COMPILER_HAS_PRAGMA_ONCE
#pragma once
#endif

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

#endif /* HOTBAR_HPP__ ///////////////////////////////////////////////////////*/

/*******************************************************************************
 *
 * Copyright (c) 2020 Joshua Robertson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
*******************************************************************************/
