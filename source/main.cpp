#include "main.hpp"

int main (int argc, char** argv)
{
    gErrorTerminateCallback = quit_application;
    gErrorMaximumCallback = save_restore_files;

    // We defer so that this always gets scalled on scope exit no matter what.
    Defer { quit_application(); };

    init_application(argc, argv);
    while (main_running && handle_application_events())
    {
        do_application();
    }

    return 0;
}
