enum class Cursor { ARROW, BEAM, POINTER, BRUSH, FILL, SELECT, Total };

TEINAPI bool LoadEditorCursors ();
TEINAPI void FreeEditorCursors ();

TEINAPI void   SetCursorType (Cursor cursor);
TEINAPI Cursor GetCursorType ();

TEINAPI bool CustomCursorsEnabled ();
