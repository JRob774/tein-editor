/*******************************************************************************
 * Editor GUI widget that allows for picking a color using various sliders.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

TEINAPI void   init_color_picker ();
TEINAPI void   open_color_picker (vec4* color);
TEINAPI void     do_color_picker ();
TEINAPI void cancel_color_picker ();

TEINAPI void handle_color_picker_events ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
