/*******************************************************************************
 * Debug facilities for logging messages and timing sections of code.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr const char* DEBUG_LOG_NAME = "logs/debug_editor.log";

static int   current_debug_section = 0;
static FILE* debug_log;

struct Debug_Timer
{
    std::string  name;
    U64 start_counter;
};

static std::stack<Debug_Timer> debug_timers;
static std::vector<std::string> debug_timer_results;

/* -------------------------------------------------------------------------- */

TEINAPI void internal__log_debug (const char* format, ...)
{
    // We only open the debug log once the first debug occurs.
    if (!debug_log)
    {
        std::string debug_log_name(build_resource_string(DEBUG_LOG_NAME));
        create_path(strip_file_name(debug_log_name));
        debug_log = fopen(debug_log_name.c_str(), "w");
        if (debug_log)
        {
            LOG_DEBUG("DEBUG LOG [%s]", format_time("%m/%d/%Y %H:%M:%S").c_str());
        }
    }

    va_list args;

    #if defined(BUILD_DEBUG)
    va_start(args, format);
    for (int i=0; i<current_debug_section; ++i)
    {
        fprintf(stdout, "  ");
    }
    vfprintf(stdout, format, args);
    fprintf(stdout, "\n");
    va_end(args);
    fflush(stdout);
    #endif

    if (debug_log)
    {
        va_start(args, format);
        for (int i=0; i<current_debug_section; ++i)
        {
            fprintf(debug_log, "  ");
        }
        vfprintf(debug_log, format, args);
        fprintf(debug_log, "\n");
        va_end(args);
        fflush(debug_log);
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void begin_debug_section (const char* name)
{
    if (name) LOG_DEBUG("%s", name);
    current_debug_section++;
}

TEINAPI void end_debug_section ()
{
    if (current_debug_section > 0) current_debug_section--;
}

/* -------------------------------------------------------------------------- */

TEINAPI void quit_debug_system ()
{
    // This condition is important because, for some reason, calling
    // fclose(NULL) results in a long hang-time during program exit.
    if (debug_log)
    {
        fclose(debug_log);
        debug_log = NULL;
    }
}

/* -------------------------------------------------------------------------- */

#if defined(BUILD_DEBUG) /*****************************************************/

TEINAPI void begin_debug_timer (const char* name)
{
    Debug_Timer timer = {};
    timer.start_counter = SDL_GetPerformanceCounter();
    timer.name = name;
    debug_timers.push(timer);
}

TEINAPI void end_debug_timer ()
{
    Debug_Timer timer = debug_timers.top();
    debug_timers.pop();

    U64   start_counter = timer.start_counter;
    U64   end_counter   = SDL_GetPerformanceCounter();
    float frequency     = static_cast<float>(SDL_GetPerformanceFrequency());
    float elapsed       = static_cast<float>(end_counter-start_counter);
    float seconds       = elapsed / frequency;

    std::string str(format_string("%s took %fs.", timer.name.c_str(), seconds));
    debug_timer_results.push_back(str);
}

TEINAPI void clear_debug_timer_results ()
{
    debug_timer_results.clear();
}

TEINAPI void dump_debug_timer_results  ()
{
    if (!debug_timer_results.empty())
    {
        begin_debug_section("Debug Timer Results:");
        for (auto str: debug_timer_results) LOG_DEBUG(str.c_str());
        end_debug_section();
    }
}

#endif /* BUILD_DEBUG *********************************************************/

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
