#include "Editor/Utility.hpp"
#include "Editor/Window.hpp"
#include "Editor/Settings.hpp"
#include "Editor/UserInterface.hpp"
#include "Editor/Logger.hpp"
#include "Editor/Renderer.hpp"
#include "Editor/Editor.hpp"

using namespace TEIN;

int main (int argc, char** argv)
{
    // We defer the termination code so that it is always called no matter how the program exits.
    Defer {
        Editor::Quit();
        Ui::Quit();
        Window::Quit();
        SDL_Quit();
    };

    g_Settings.Load();

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        Logger::SingleSystemMessage("SDL", "Failed to initialize SDL: ", SDL_GetError());
    } else {
        if (Window::Init()) {
            if (Ui::Init()) {
                if (Editor::Init()) {
                    Window::Show(); // We show window after everything is setup and ready.

                    bool running = true;
                    while (running) {
                        SDL_Event event;
                        while (SDL_PollEvent(&event)) {
                            switch (event.type) {
                                case (SDL_KEYDOWN): {
                                    switch (event.key.keysym.sym) {
                                        case (SDLK_RETURN): if (!(SDL_GetModState()&KMOD_ALT)) break;
                                        case (SDLK_F11): {
                                            Window::EnableFullscreen(!Window::IsFullscreen()); // Toggle fullscreen.
                                        } break;
                                    }
                                } break;
                                case (SDL_QUIT): {
                                    running = false;
                                } break;
                                // Special case we need to handle for quitting, because of ImGui, which can have multiple windows/viewports.
                                case (SDL_WINDOWEVENT): {
                                    if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                                        if (event.window.windowID == SDL_GetWindowID(Window::GetInternalWindow())) {
                                            running = false;
                                        }
                                    }
                                } break;
                            }
                            Ui::HandleEvents(event);
                        }

                        Renderer::Clear(0,0,0);

                        Ui::BeginFrame();
                        Editor::Update();
                        Ui::EndFrame();

                        Window::Refresh();
                    }
                }
            }
        }
    }

    return 0;
}
