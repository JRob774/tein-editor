/*******************************************************************************
 * Interface for loading custom application mouse cursors for the editor.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

enum class Cursor { ARROW, BEAM, POINTER, BRUSH, FILL, SELECT, Total };

FILDEF bool load_editor_cursors ();
FILDEF void free_editor_cursors ();

FILDEF void   set_cursor (Cursor cursor);
FILDEF Cursor get_cursor ();

FILDEF bool custom_cursors_enabled ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
