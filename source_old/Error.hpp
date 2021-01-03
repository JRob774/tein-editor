#define LogError(...) Internal::LogErrorMessage(__FILE__, __LINE__, __VA_ARGS__)

// Set this callback and it will be called during fatal exception termination.
static void(*gErrorTerminateCallback)(void);
static void(*gErrorMaximumCallback)(void);

enum class ErrorLevel { Min, Med, Max };

TEINAPI bool InitErrorSystem ();
TEINAPI void QuitErrorSystem ();
