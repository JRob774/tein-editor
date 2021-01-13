struct Logger
{
    const char* LogFileName = "debug.log";
    std::ofstream logFile;
    std::stack<std::string> systems;
};

Global Logger gLogger;

// Controls the stack of current systems the logger reports to.
EditorAPI void PushLogSystem (std::string system);
EditorAPI void PopLogSystem ();

EditorAPI void LogSingleSystemMessage (std::string system, const char* format, ...); // Logs a message for the specified system.
EditorAPI void LogSystemMessage (const char* format, ...); // Logs a message for the current system.
EditorAPI void LogMessage (const char* format, ...); // Logs a message.
