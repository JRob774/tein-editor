/*******************************************************************************
 * Facilities for logging error messages and producing crash dumps on failure.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr const char* CRASH_DUMP_NAME = "TheEndEditor-Crash.dmp";
static constexpr const char* DEBUG_DUMP_NAME = "TheEndEditor-Debug.dmp";
static constexpr const char*  ERROR_LOG_NAME =  "logs/error_editor.log";

static FILE* error_log;

/* -------------------------------------------------------------------------- */

// Unhandled exception dump taken from here <https://stackoverflow.com/a/700108>
#if defined(PLATFORM_WIN32)
TEINAPI LONG WINAPI internal__unhandled_exception_filter (struct _EXCEPTION_POINTERS* info)
{
    show_alert("Error", "Fatal exception occurred!\nCreating crash dump!",
        ALERT_TYPE_ERROR, ALERT_BUTTON_OK);

    std::string file_name(make_path_absolute(CRASH_DUMP_NAME));
    HANDLE file = CreateFileA(file_name.c_str(), GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file != INVALID_HANDLE_VALUE)
    {
        Defer { CloseHandle(file); };

        MINIDUMP_EXCEPTION_INFORMATION mini_dump_info = {};
        mini_dump_info.ThreadId          = GetCurrentThreadId();
        mini_dump_info.ExceptionPointers = info;
        mini_dump_info.ClientPointers    = TRUE;

        MINIDUMP_TYPE type = static_cast<MINIDUMP_TYPE>(MiniDumpWithFullMemory|MiniDumpWithHandleData);
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
            file, type, &mini_dump_info, NULL, NULL);
    }

    if (error_terminate_callback)
    {
        error_terminate_callback();
    }

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

TEINAPI void internal__log_error (const char* file, int line, Error_Level level, const char* format, ...)
{
    // We only open the error log once the first error occurs.
    if (!error_log)
    {
        std::string error_log_name(build_resource_string(ERROR_LOG_NAME));
        create_path(strip_file_name(error_log_name));
        error_log = fopen(error_log_name.c_str(), "a");
    }

    va_list args;

    std::string file_str = fix_path_slashes(file);
    std::string time_str = format_time("%m/%d/%Y %H:%M:%S");

    const char* err_format = "[%s] Error in %s at line %d: ";

    #if defined(BUILD_DEBUG)
    va_start(args, format);
    fprintf(stderr, format, time_str.c_str(), file_str.c_str(), line);
    vfprintf(stderr, err_format, args);
    fprintf(stderr, "\n");
    va_end(args);
    fflush(stderr);
    #endif

    if (error_log)
    {
        va_start(args, format);
        fprintf(error_log, format, time_str.c_str(), file_str.c_str(), line);
        vfprintf(error_log, err_format, args);
        fprintf(error_log, "\n");
        va_end(args);
        fflush(error_log);
    }

    if (level == ERR_MIN) return;

    va_start(args, format);
    std::string msg = format_string_v(format, args);
    show_alert("Error!", msg, ALERT_TYPE_ERROR, ALERT_BUTTON_OK, "WINMAIN");
    va_end(args);

    if (level == ERR_MED) return;

    main_running = false;

    if (error_maximum_callback)
    {
        error_maximum_callback();
    }
}

/* -------------------------------------------------------------------------- */

#if defined(PLATFORM_WIN32)
TEINAPI bool init_error_system ()
{
    SetUnhandledExceptionFilter(&internal__unhandled_exception_filter);
    return true;
}
#else
#error init_error_system not implemented on the current platform!
#endif

TEINAPI void quit_error_system ()
{
    // This condition is important because, for some reason, calling
    // fclose(NULL) results in a long hang-time during program exit.
    if (error_log)
    {
        fclose(error_log);
        error_log = NULL;
    }
}

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

/*******************************************************************************
 *
 * Copyright (c) 2020 Joshua Robertson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
*******************************************************************************/
