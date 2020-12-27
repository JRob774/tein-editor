/*******************************************************************************
 * Editor GUI widget that allows for picking a color using various sliders.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

FILDEF void   init_color_picker ();
FILDEF void   open_color_picker (vec4* color);
FILDEF void     do_color_picker ();
FILDEF void cancel_color_picker ();

FILDEF void handle_color_picker_events ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
