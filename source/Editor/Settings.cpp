#include "Settings.hpp"

#include "Logger.hpp"
#include "Window.hpp"

#include <fstream>

#include <gon/gon.h>

namespace TEIN
{
    namespace
    {
        static constexpr const char* k_SettingsFile = "settings.data";
    }

    void Settings::Save ()
    {
        std::ofstream settings(k_SettingsFile, std::ios::trunc);
        if (!settings.is_open()) {
            Logger::SingleSystemMessage("settings", "Failed to save settings!");
        } else {
            settings << std::boolalpha;
            settings << "window_width "  << Window::GetCachedWidth()  << std::endl;
            settings << "window_height " << Window::GetCachedHeight() << std::endl;
            settings << "fullscreen "    << Window::IsFullscreen()    << std::endl;
        }
    }

    void Settings::Load ()
    {
        GonObject settings = GonObject::Load(k_SettingsFile);
        g_Settings.m_WindowWidth  = settings["window_width" ].Int (Settings::k_DefaultWindowWidth);
        g_Settings.m_WindowHeight = settings["window_height"].Int (Settings::k_DefaultWindowHeight);
        g_Settings.m_Fullscreen   = settings["fullscreen"   ].Bool(Settings::k_DefaultFullscreen);
    }

    void Settings::Reset ()
    {
        Window::SetSize(Settings::k_DefaultWindowWidth, Settings::k_DefaultWindowHeight);
        Window::EnableFullscreen(Settings::k_DefaultFullscreen);
    }

    Settings g_Settings;
}
