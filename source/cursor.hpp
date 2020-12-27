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

TEINAPI bool load_editor_cursors ();
TEINAPI void free_editor_cursors ();

TEINAPI void   set_cursor (Cursor cursor);
TEINAPI Cursor get_cursor ();

TEINAPI bool custom_cursors_enabled ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
