/*******************************************************************************
 * Functionality for platform-specific open/save/path dialog operations.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

enum class Dialog_Type { LVL, CSV, LVL_CSV, GPAK, EXE };

STDDEF std::vector<std::string> open_dialog (Dialog_Type type, bool multiselect = true);
STDDEF std::string              save_dialog (Dialog_Type type);
STDDEF std::vector<std::string> path_dialog (                  bool multiselect = true);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
