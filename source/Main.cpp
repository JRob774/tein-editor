#include "Main.hpp"

int main (int argc, char** argv)
{
    // We defer the termination code so that it is always called no matter how the program exits.
    Defer {
        SaveSettings();
        FreeAssets();
        QuitEditor();
        QuitUi();
        QuitWindow();
        SDL_Quit();
    };

    LoadSettings();

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        LogSingleSystemMessage("SDL", "Failed to initialize SDL: ", SDL_GetError());
    } else {
        if (InitWindow()) {
            if (InitUi()) {
                if (InitEditor()) {
                    ShowWindow(); // We show window after everything is setup and ready.

                    bool running = true;
                    while (running) {
                        SDL_Event event;
                        while (SDL_PollEvent(&event)) {
                            switch (event.type) {
                                case (SDL_KEYDOWN): {
                                    switch (event.key.keysym.sym) {
                                        case (SDLK_RETURN): if (!(SDL_GetModState()&KMOD_ALT)) break;
                                        case (SDLK_F11): {
                                            EnableWindowFullscreen(!IsWindowFullscreen()); // Toggle fullscreen.
                                        } break;
                                    }
                                } break;
                                case (SDL_QUIT): {
                                    running = false;
                                } break;
                                // Special case we need to handle for quitting, because of ImGui, which can have multiple windows/viewports.
                                case (SDL_WINDOWEVENT): {
                                    if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                                        if (event.window.windowID == SDL_GetWindowID(GetInternalWindow())) {
                                            running = false;
                                        }
                                    }
                                } break;
                            }
                            HandleUiEvents(event);
                            HandleEditorEvents(event);
                        }

                        glClearColor(0,0,0,1);
                        glClear(GL_COLOR_BUFFER_BIT);

                        BeginUiFrame();
                        DoEditor();
                        EndUiFrame();

                        RefreshWindow();
                    }
                }
            }
        }
    }

    return 0;
}
