enum class ResizeDir { N, E, S, W, NE, SE, SW, NW, Center };

TEINAPI void OpenResize (int levelWidth = -1, int levelHeight = -1);
TEINAPI void DoResize ();
TEINAPI void CancelResize ();

TEINAPI void HandleResizeEvents ();

TEINAPI int GetResizeWidth ();
TEINAPI int GetResizeHeight ();

TEINAPI ResizeDir GetResizeDir ();

TEINAPI bool ResizeDirIsNorth (ResizeDir dir);
TEINAPI bool ResizeDirIsEast (ResizeDir dir);
TEINAPI bool ResizeDirIsSouth (ResizeDir dir);
TEINAPI bool ResizeDirIsWest (ResizeDir dir);
