#pragma once

#include <string>

#include <imgui/imgui.h>
#include <SDL2/SDL.h>

namespace TEIN
{
    namespace Ui
    {
        bool Init ();
        void Quit ();
        void HandleEvents (const SDL_Event& event);
        void BeginFrame ();
        void EndFrame ();

        // Wrap ImGui::ImageButton with getting a texture from the asset manager and using it.
        // If a texture cannot be received from the asset manager then NULL is passed to ImGui.
        bool DoImageButton (std::string imageName, ImVec2 size);
    }
}
