static constexpr const char* gDebugLogName = "logs/debug_editor.log";

static int gCurrentDebugSection = 0;
static FILE* gDebugLog;

struct DebugTimer
{
    std::string name;
    U64 startCounter;
};

static std::stack<DebugTimer> gDebugTimers;
static std::vector<std::string> gDebugTimerResults;

namespace Internal
{
    TEINAPI void LogDebugMessage (const char* format, ...)
    {
        // We only open the debug log once the first debug occurs.
        if (!gDebugLog)
        {
            std::string debugLogName(BuildResourceString(gDebugLogName));
            CreatePath(StripFileName(debugLogName));
            gDebugLog = fopen(debugLogName.c_str(), "w");
            if (gDebugLog)
            {
                LogDebug("DEBUG LOG [%s]", FormatTime("%m/%d/%Y %H:%M:%S").c_str());
            }
        }

        va_list args;

        #if defined(BuildDebug)
        va_start(args, format);
        for (int i=0; i<gCurrentDebugSection; ++i)
        {
            fprintf(stdout, "  ");
        }
        vfprintf(stdout, format, args);
        fprintf(stdout, "\n");
        va_end(args);
        fflush(stdout);
        #endif // BuildDebug

        if (gDebugLog)
        {
            va_start(args, format);
            for (int i=0; i<gCurrentDebugSection; ++i)
            {
                fprintf(gDebugLog, "  ");
            }
            vfprintf(gDebugLog, format, args);
            fprintf(gDebugLog, "\n");
            va_end(args);
            fflush(gDebugLog);
        }
    }
}

TEINAPI void BeginDebugSection (const char* name)
{
    if (name) LogDebug("%s", name);
    gCurrentDebugSection++;
}

TEINAPI void EndDebugSection ()
{
    if (gCurrentDebugSection > 0) gCurrentDebugSection--;
}

TEINAPI void QuitDebugSystem ()
{
    // This condition is important because, for some reason, calling
    // fclose(NULL) results in a long hang-time during program exit.
    if (gDebugLog)
    {
        fclose(gDebugLog);
        gDebugLog = NULL;
    }
}

#if defined(BuildDebug)

TEINAPI void BeginDebugTimer (const char* name)
{
    DebugTimer timer;
    timer.startCounter = SDL_GetPerformanceCounter();
    timer.name = name;
    gDebugTimers.push(std::move(timer));
}
TEINAPI void EndDebugTimer ()
{
    DebugTimer timer = gDebugTimers.top();
    gDebugTimers.pop();

    U64 startCounter = timer.startCounter;
    U64 endCounter = SDL_GetPerformanceCounter();

    float frequency = static_cast<float>(SDL_GetPerformanceFrequency());
    float elapsed = static_cast<float>(endCounter-startCounter);
    float seconds = elapsed / frequency;

    std::string results(FormatString("%s took %fs.", timer.name.c_str(), seconds));
    gDebugTimerResults.push_back(results);
}
TEINAPI void ClearDebugTimerResult ()
{
    gDebugTimerResults.clear();
}
TEINAPI void DumpDebugTimerResult  ()
{
    if (!gDebugTimerResults.empty())
    {
        BeginDebugSection("Debug Timer Results:");
        for (auto str: gDebugTimerResults) LogDebug(str.c_str());
        EndDebugSection();
    }
}

#else

TEINAPI void BeginDebugTimer (const char* name)
{
    // Nothing...
}
TEINAPI void EndDebugTimer ()
{
    // Nothing...
}
TEINAPI void ClearDebugTimerResult ()
{
    // Nothing...
}
TEINAPI void DumpDebugTimerResult  ()
{
    // Nothing...
}

#endif // BuildDebug
