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

TEINAPI void open_resize   (int lw = -1, int lh = -1);
TEINAPI void do_resize     ();
TEINAPI void cancel_resize ();

TEINAPI void handle_resize_events ();

TEINAPI int get_resize_w ();
TEINAPI int get_resize_h ();

TEINAPI Resize_Dir get_resize_dir ();

TEINAPI bool resize_dir_is_north (Resize_Dir dir);
TEINAPI bool resize_dir_is_east  (Resize_Dir dir);
TEINAPI bool resize_dir_is_south (Resize_Dir dir);
TEINAPI bool resize_dir_is_west  (Resize_Dir dir);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
