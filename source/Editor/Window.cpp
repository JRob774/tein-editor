#include "Window.hpp"

#include "Utility.hpp"
#include "Logger.hpp"
#include "Settings.hpp"

#include <glad/glad.h>

namespace TEIN
{
    namespace Window
    {
        namespace
        {
            static constexpr const char* k_Title = "The End is Nigh: Editor";

            static SDL_Window* s_Window;
            static SDL_GLContext s_Context;
            static int s_CachedWidth; // The size of the non-fullscreen window.
            static int s_CachedHeight;
            static bool s_Fullscreen;
        }

        bool Init ()
        {
            Logger::PushSystem("window");
            Defer { Logger::PopSystem(); };

            // The SDL docs say that this should be done before creation of the window!
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

            s_CachedWidth = g_Settings.m_WindowWidth;
            s_CachedHeight = g_Settings.m_WindowHeight;

            s_Window = SDL_CreateWindow(k_Title, SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, s_CachedWidth,s_CachedHeight,
                SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIDDEN);
            if (!s_Window) {
                Logger::SystemMessage("Failed to create window: %s", SDL_GetError());
                return false;
            }

            s_Context = SDL_GL_CreateContext(s_Window);
            if (!s_Context) {
                Logger::SystemMessage("Failed to create OpenGL context: %s", SDL_GetError());
                return false;
            }

            if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
                Logger::SystemMessage("Failed to load OpenGL procedures!");
                return false;
            }

            SDL_SetWindowMinimumSize(s_Window, k_MinimumWidth,k_MinimumHeight);

            EnableFullscreen(false/*gSettings.fullscreen*/);

            if (SDL_GL_SetSwapInterval(-1) < 0) {
                if (SDL_GL_SetSwapInterval(1) < 0) {
                    Logger::SystemMessage("Failed to enable VSync!");
                }
            }

            return true;
        }

        void Quit ()
        {
            SDL_GL_DeleteContext(s_Context);
            SDL_DestroyWindow(s_Window);
        }

        void Refresh ()
        {
            SDL_GL_SwapWindow(s_Window);
        }

        void Show ()
        {
            SDL_ShowWindow(s_Window);
        }
        void Hide ()
        {
            SDL_HideWindow(s_Window);
        }

        void EnableFullscreen (bool enable)
        {
            if (enable) SDL_GetWindowSize(s_Window, &s_CachedWidth, &s_CachedHeight);
            s_Fullscreen = enable;
            SDL_SetWindowFullscreen(s_Window, (s_Fullscreen) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
        }
        bool IsFullscreen ()
        {
            return s_Fullscreen;
        }

        void SetSize (int width, int height)
        {
            SDL_SetWindowSize(s_Window, width, height);
        }
        void SetTitle (std::string title)
        {
            SDL_SetWindowTitle(s_Window, title.c_str());
        }

        int GetWidth ()
        {
            int width;
            SDL_GetWindowSize(s_Window, &width,NULL);
            return width;
        }
        int GetHeight ()
        {
            int height;
            SDL_GetWindowSize(s_Window, NULL,&height);
            return height;
        }

        int GetCachedWidth ()
        {
            if (s_Fullscreen) return s_CachedWidth;
            return GetWidth();
        }
        int GetCachedHeight ()
        {
            if (s_Fullscreen) return s_CachedHeight;
            return GetHeight();
        }

        SDL_Window* GetInternalWindow ()
        {
            return s_Window;
        }
        SDL_GLContext GetInternalContext ()
        {
            return s_Context;
        }
    }

}
