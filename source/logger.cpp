Internal struct
{
    const char* LogFileName = "debug.log";
    std::ofstream logFile;
    std::stack<std::string> systems;

} iLogger;

Internal void Internal_PrintLogMessage (const char* format, va_list args)
{
    std::string message = FormatString_V(format, args);
    // Open the log if it hasn't been opened already.
    if (!iLogger.logFile.is_open()) {
        iLogger.logFile.open(iLogger.LogFileName);
    }
    if (iLogger.logFile.is_open()) iLogger.logFile << message << std::endl;
    std::cout << message << std::endl;
}

EditorAPI void PushLogSystem (std::string system)
{
    iLogger.systems.push(system);
}
EditorAPI void PopLogSystem ()
{
    assert(!iLogger.systems.empty());
    iLogger.systems.pop();
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
    assert(!iLogger.systems.empty());
    std::string finalFormat = "[" + iLogger.systems.top() + "] " + format;
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
