/*******************************************************************************
 * Dialog box that opens up when the user wants to create a new level/map.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

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
