enum class EditorEvent: U32 { UiRedraw, BackupTab, Cooldown, BlinkCursor, GPAKProgress, ShowTooltip, ShowUpdate };
TEINAPI void PushEditorEvent (EditorEvent id, void* data1, void* data2);
