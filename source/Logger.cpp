Internal void Internal_PrintLogMessage (const char* format, va_list args)
{
    std::string message = FormatString_V(format, args);
    // Open the log if it hasn't been opened already.
    if (!gLogger.logFile.is_open()) {
        gLogger.logFile.open(gLogger.LogFileName);
    }
    if (gLogger.logFile.is_open()) gLogger.logFile << message << std::endl;
    std::cout << message << std::endl;
}

EditorAPI void PushLogSystem (std::string system)
{
    gLogger.systems.push(system);
}
EditorAPI void PopLogSystem ()
{
    assert(!gLogger.systems.empty());
    gLogger.systems.pop();
}

EditorAPI void LogSingleSystemMessage (std::string system, const char* format, ...)
{
    std::string finalFormat = "[" + system + "] " + format;
    va_list(args);
    va_start(args, format);
    Internal_PrintLogMessage(finalFormat.c_str(), args);
    va_end(args);
}
EditorAPI void LogSystemMessage (const char* format, ...)
{
    assert(!gLogger.systems.empty());
    std::string finalFormat = "[" + gLogger.systems.top() + "] " + format;
    va_list(args);
    va_start(args, format);
    Internal_PrintLogMessage(finalFormat.c_str(), args);
    va_end(args);
}
EditorAPI void LogMessage (const char* format, ...)
{
    va_list(args);
    va_start(args, format);
    Internal_PrintLogMessage(format, args);
    va_end(args);
}
