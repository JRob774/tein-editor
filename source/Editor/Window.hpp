#pragma once

#include <string>

#include <SDL2/SDL.h>

namespace TEIN
{
    namespace Window
    {
        constexpr int k_StartWidth = 1280;
        constexpr int k_StartHeight = 720;
        constexpr int k_MinimumWidth = 320;
        constexpr int k_MinimumHeight = 240;

        bool Init ();
        void Quit ();
        void Refresh ();
        void Show ();
        void Hide ();
        void EnableFullscreen (bool enable);
        bool IsFullscreen ();
        void SetSize (int width, int height);
        void SetTitle (std::string title);
        int GetWidth ();
        int GetHeight ();
        int GetCachedWidth ();
        int GetCachedHeight ();
        SDL_Window* GetInternalWindow ();
        SDL_GLContext GetInternalContext ();
    }
}
