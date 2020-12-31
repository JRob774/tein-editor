#include "main.hpp"

int main (int argc, char** argv)
{
    gErrorTerminateCallback = QuitApplication;
    gErrorMaximumCallback = SaveRestoreFiles;

    // We defer so that this always gets scalled on scope exit no matter what.
    Defer { QuitApplication(); };

    InitApplication(argc, argv);
    while (gMainRunning && HandleApplicationEvents())
    {
        DoApplication();
    }

    return 0;
}
