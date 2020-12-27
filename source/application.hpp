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

FILDEF void init_application (int argc, char** argv);
FILDEF void quit_application ();

FILDEF void do_application ();

FILDEF bool handle_application_events ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
