#pragma once

#define LogDebug(format, ...) Internal::LogDebugMessage(format, __VA_ARGS__)

// Used to handle indentation of debug messages to the log.
TEINAPI void BeginDebugSection (const char* name = NULL);
TEINAPI void EndDebugSection   ();

TEINAPI void QuitDebugSystem ();

// These functions will do nothing if not in BUILD_DEBUG mode.
TEINAPI void BeginDebugTimer       (const char* name);
TEINAPI void EndDebugTimer         ();
TEINAPI void ClearDebugTimerResult ();
TEINAPI void DumpDebugTimerResult  ();
