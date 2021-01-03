static constexpr const char* gCrashDumpName = "TheEndEditor-Crash.dmp";
static constexpr const char* gErrorLogName = "logs/error_editor.log";

static FILE* gErrorLog;

namespace Internal
{
    // Unhandled exception dump taken from here <https://stackoverflow.com/a/700108>
    #if defined(PLATFORM_WINDOWS)
    TEINAPI LONG WINAPI UnhandledExceptionFilter (struct _EXCEPTION_POINTERS* info)
    {
        ShowAlert("Error", "Fatal exception occurred!\nCreating crash dump!", AlertType::Error, AlertButton::Ok);

        std::string fileName(MakePathAbsolute(gCrashDumpName));
        HANDLE file = CreateFileA(fileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file != INVALID_HANDLE_VALUE) {
            Defer { CloseHandle(file); };

            MINIDUMP_EXCEPTION_INFORMATION miniDumpInfo = {};
            miniDumpInfo.ThreadId = GetCurrentThreadId();
            miniDumpInfo.ExceptionPointers = info;
            miniDumpInfo.ClientPointers = TRUE;

            MINIDUMP_TYPE type = static_cast<MINIDUMP_TYPE>(MiniDumpWithFullMemory|MiniDumpWithHandleData);
            MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, type, &miniDumpInfo, NULL, NULL);
        }

        if (gErrorTerminateCallback) gErrorTerminateCallback();

        return EXCEPTION_EXECUTE_HANDLER;
    }
    #endif // PLATFORM_WINDOWS

    TEINAPI void LogErrorMessage (const char* file, int line, ErrorLevel level, const char* format, ...)
    {
        // We only open the error log once the first error occurs.
        if (!gErrorLog) {
            std::string errorLogName(BuildResourceString(gErrorLogName));
            CreatePath(StripFileName(errorLogName));
            gErrorLog = fopen(errorLogName.c_str(), "a");
        }

        va_list args;

        std::string fileStr = FixPathSlashes(file);
        std::string timeStr = FormatTime("%m/%d/%Y %H:%M:%S");

        const char* errFormat = "[%s] Error in %s at line %d: ";

        #if defined(BUILD_DEBUG)
        va_start(args, format);
        fprintf(stderr, format, timeStr.c_str(), fileStr.c_str(), line);
        vfprintf(stderr, errFormat, args);
        fprintf(stderr, "\n");
        va_end(args);
        fflush(stderr);
        #endif // BUILD_DEBUG

        if (gErrorLog) {
            va_start(args, format);
            fprintf(gErrorLog, format, timeStr.c_str(), fileStr.c_str(), line);
            vfprintf(gErrorLog, errFormat, args);
            fprintf(gErrorLog, "\n");
            va_end(args);
            fflush(gErrorLog);
        }

        if (level == ErrorLevel::Min) return;

        va_start(args, format);
        std::string msg = FormatStringV(format, args);
        ShowAlert("Error!", msg, AlertType::Error, AlertButton::Ok, "WINMAIN");
        va_end(args);

        if (level == ErrorLevel::Med) return;

        gMainRunning = false;

        if (gErrorMaximumCallback) gErrorMaximumCallback();
    }
}

#if defined(PLATFORM_WINDOWS)
TEINAPI bool InitErrorSystem ()
{
    SetUnhandledExceptionFilter(&Internal::UnhandledExceptionFilter);
    return true;
}
#endif // PLATFORM_WINDOWS

TEINAPI void QuitErrorSystem ()
{
    // This condition is important because, for some reason, calling
    // fclose(NULL) results in a long hang-time during program exit.
    if (gErrorLog) {
        fclose(gErrorLog);
        gErrorLog = NULL;
    }
}
