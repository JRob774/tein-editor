static constexpr float gToolbarInnerPad = 1;
static constexpr float gToolbarDefaultWidth = 30;

TEINAPI void DoToolbar ();
TEINAPI float GetToolbarWidth ();

// Functionality for the various buttons/tools located on the toolbar.
TEINAPI void ToolbarSetToolToBrush ();
TEINAPI void ToolbarSetToolToFill ();
TEINAPI void ToolbarSetToolToSelect ();
TEINAPI void ToolbarToggleGrid ();
TEINAPI void ToolbarToggleBounds ();
TEINAPI void ToolbarToggleLayerTransparency ();
TEINAPI void ToolbarResetCamera ();
TEINAPI void ToolbarFlipLevelH ();
TEINAPI void ToolbarFlipLevelV ();
TEINAPI void ToolbarToggleMirrorH ();
TEINAPI void ToolbarToggleMirrorV ();
TEINAPI void ToolbarCut ();
TEINAPI void ToolbarCopy ();
TEINAPI void ToolbarDeselect ();
TEINAPI void ToolbarClearSelect ();
TEINAPI void ToolbarResize ();
TEINAPI void ToolbarToggleEntity ();
TEINAPI void ToolbarToggleGuides ();
