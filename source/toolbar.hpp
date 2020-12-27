/*******************************************************************************
 * Editor GUI widget containing level/map editor specific actions and tools.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#ifndef TOOLBAR_HPP__ /*//////////////////////////////////////////////////////*/
#define TOOLBAR_HPP__

#ifdef COMPILER_HAS_PRAGMA_ONCE
#pragma once
#endif

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float TOOLBAR_INNER_PAD =  1;
static constexpr float TOOLBAR_WIDTH     = 30;

FILDEF void  do_toolbar    ();
FILDEF float get_toolbar_w ();

// Functionality for the various buttons/tools located on the toolbar.

FILDEF void tb_set_tool_to_brush  ();
FILDEF void tb_set_tool_to_fill   ();
FILDEF void tb_set_tool_to_select ();
FILDEF void tb_toggle_grid        ();
FILDEF void tb_toggle_bounds      ();
FILDEF void tb_toggle_layer_trans ();
FILDEF void tb_reset_camera       ();
FILDEF void tb_flip_level_h       ();
FILDEF void tb_flip_level_v       ();
FILDEF void tb_toggle_mirror_h    ();
FILDEF void tb_toggle_mirror_v    ();
FILDEF void tb_cut                ();
FILDEF void tb_copy               ();
FILDEF void tb_deselect           ();
FILDEF void tb_clear_select       ();
FILDEF void tb_resize             ();
FILDEF void tb_toggle_entity      ();
FILDEF void tb_toggle_guides      ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

#endif /* TOOLBAR_HPP__ //////////////////////////////////////////////////////*/

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
