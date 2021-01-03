enum class Cursor { Arrow, Beam, Pointer, Brush, Fill, Select, Total };

TEINAPI bool LoadEditorCursors ();
TEINAPI void FreeEditorCursors ();
TEINAPI void SetCursorType (Cursor cursor);
TEINAPI Cursor GetCursorType ();
TEINAPI bool CustomCursorsEnabled ();
