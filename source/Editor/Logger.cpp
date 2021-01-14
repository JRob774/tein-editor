#include "Logger.hpp"

#include "Utility.hpp"

#include <cassert>
#include <cstdarg>

#include <iostream>
#include <fstream>
#include <stack>

namespace TEIN
{
    namespace Logger
    {
        namespace
        {
            static std::stack<std::string> s_Systems;
            static std::ofstream s_LogFile;

            static void PrintMessage (const char* format, va_list args)
            {
                std::string message = Utility::FormatString_V(format, args);
                // Open the log if it hasn't been opened already.
                if (!s_LogFile.is_open()) {
                    s_LogFile.open("debug.log");
                }
                if (s_LogFile.is_open()) s_LogFile << message << std::endl;
                std::cout << message << std::endl;
            }
        }

        void PushSystem (std::string system)
        {
            s_Systems.push(system);
        }
        void PopSystem ()
        {
            assert(!s_Systems.empty());
            s_Systems.pop();
        }

        void SingleSystemMessage (std::string system, const char* format, ...)
        {
            std::string finalFormat = "[" + system + "] " + format;
            va_list(args);
            va_start(args, format);
            PrintMessage(finalFormat.c_str(), args);
            va_end(args);
        }
        void SystemMessage (const char* format, ...)
        {
            assert(!s_Systems.empty());
            std::string finalFormat = "[" + s_Systems.top() + "] " + format;
            va_list(args);
            va_start(args, format);
            PrintMessage(finalFormat.c_str(), args);
            va_end(args);
        }
        void Message (const char* format, ...)
        {
            va_list(args);
            va_start(args, format);
            PrintMessage(format, args);
            va_end(args);
        }
    }
}
