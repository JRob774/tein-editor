/*******************************************************************************
 * Editor GUI widget that shows information on the current hovered element.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float STATUS_BAR_INNER_PAD =  6;
static constexpr float STATUS_BAR_HEIGHT    = 20;

INLDEF void push_status_bar_message (const char* fmt, ...);
FILDEF void do_status_bar           ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
