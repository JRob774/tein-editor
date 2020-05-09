/*******************************************************************************
 * Dialog box that opens up when the user wants to create a new level/map.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#ifndef RESIZE_DIALOG_HPP__ /*////////////////////////////////////////////////*/
#define RESIZE_DIALOG_HPP__

#ifdef COMPILER_HAS_PRAGMA_ONCE
#pragma once
#endif

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

enum class Resize_Dir { N, E, S, W, NE, SE, SW, NW, CENTER };

/* -------------------------------------------------------------------------- */

FILDEF void open_resize   (int lw = -1, int lh = -1);
FILDEF void do_resize     ();
FILDEF void cancel_resize ();

FILDEF void handle_resize_events ();

FILDEF int get_resize_w ();
FILDEF int get_resize_h ();

FILDEF Resize_Dir get_resize_dir ();

FILDEF bool resize_dir_is_north (Resize_Dir dir);
FILDEF bool resize_dir_is_east  (Resize_Dir dir);
FILDEF bool resize_dir_is_south (Resize_Dir dir);
FILDEF bool resize_dir_is_west  (Resize_Dir dir);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

#endif /* RESIZE_DIALOG_HPP__ ////////////////////////////////////////////////*/

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