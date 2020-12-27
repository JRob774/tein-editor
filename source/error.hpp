/*******************************************************************************
 * Facilities for logging error messages and producing crash dumps on failure.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

#define LOG_ERROR(...) internal__log_error(__FILE__, __LINE__, __VA_ARGS__)

/* -------------------------------------------------------------------------- */

// Set this callback and it will be called during fatal exception termination.
static void(*error_terminate_callback)(void);
static void(*error_maximum_callback)(void);

enum Error_Level { ERR_MIN, ERR_MED, ERR_MAX };

FILDEF bool init_error_system ();
FILDEF void quit_error_system ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
