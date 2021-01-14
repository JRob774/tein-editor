#pragma once

#include <string>

namespace TEIN
{
    namespace Logger
    {
        // Controls the stack of current systems the logger reports to.
        void PushSystem (std::string system);
        void PopSystem ();

        void SingleSystemMessage (std::string system, const char* format, ...); // Logs a message for the specified system.
        void SystemMessage (const char* format, ...); // Logs a message for the current system.
        void Message (const char* format, ...); // Logs a message.
    }
}
