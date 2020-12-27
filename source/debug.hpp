/*******************************************************************************
 * Debug facilities for logging messages and timing sections of code.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

#define LOG_DEBUG(format, ...) internal__log_debug(format, __VA_ARGS__)

/* -------------------------------------------------------------------------- */

// Used to handle indentation of debug messages to the log.
TEINAPI void begin_debug_section (const char* name = NULL);
TEINAPI void end_debug_section   ();

/* -------------------------------------------------------------------------- */

TEINAPI void quit_debug_system ();

/* -------------------------------------------------------------------------- */

#if defined(BUILD_DEBUG) /*****************************************************/

TEINAPI void begin_debug_timer         (const char* name);
TEINAPI void end_debug_timer           ();
TEINAPI void clear_debug_timer_results ();
TEINAPI void dump_debug_timer_results  ();

#else /************************************************************************/

TEINAPI void begin_debug_timer         (const char* name) { /* Nothing */ }
TEINAPI void end_debug_timer           ()                 { /* Nothing */ }
TEINAPI void clear_debug_timer_results ()                 { /* Nothing */ }
TEINAPI void dump_debug_timer_results  ()                 { /* Nothing */ }

#endif /* BUILD_DEBUG *********************************************************/

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
