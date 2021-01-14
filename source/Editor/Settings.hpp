#pragma once

#include "Window.hpp"

namespace TEIN
{
    class Settings
    {
    public:
        static inline constexpr int  k_DefaultWindowWidth = Window::k_StartWidth;
        static inline constexpr int  k_DefaultWindowHeight = Window::k_StartHeight;
        static inline constexpr bool k_DefaultFullscreen = true;

        int  m_WindowWidth;
        int  m_WindowHeight;
        bool m_Fullscreen;

        void Save  ();
        void Load  ();
        void Reset ();
    };

    extern Settings g_Settings;
}
