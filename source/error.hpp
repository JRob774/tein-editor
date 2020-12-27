#pragma once

#define LogError(...) Internal::LogErrorMessage(__FILE__, __LINE__, __VA_ARGS__)

// Set this callback and it will be called during fatal exception termination.
static void(*gErrorTerminateCallback)(void);
static void(*gErrorMaximumCallback)(void);

enum ErrorLevel { ERR_MIN, ERR_MED, ERR_MAX };

TEINAPI bool InitErrorSystem ();
TEINAPI void QuitErrorSystem ();
