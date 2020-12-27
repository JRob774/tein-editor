/*******************************************************************************
 * Main application initalization/termination and main execution loop.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr int APP_VER_MAJOR = 1;
static constexpr int APP_VER_MINOR = 1;
static constexpr int APP_VER_PATCH = 0;

/* -------------------------------------------------------------------------- */

static SDL_Event main_event;
static bool      main_running;

/* -------------------------------------------------------------------------- */

TEINAPI void init_application (int argc, char** argv);
TEINAPI void quit_application ();

TEINAPI void do_application ();

TEINAPI bool handle_application_events ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
