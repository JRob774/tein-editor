#include "Documents.hpp"

#include "Logger.hpp"
#include "Renderer.hpp"

#include <imgui/imgui_internal.h>

namespace TEIN
{
    ImVec2 Document::GetWindowSize () const
    {
        ImVec2 winMin = ImGui::GetWindowContentRegionMin();
        ImVec2 winMax = ImGui::GetWindowContentRegionMax();

        winMin.x += ImGui::GetWindowPos().x;
        winMin.y += ImGui::GetWindowPos().y;
        winMax.x += ImGui::GetWindowPos().x;
        winMax.y += ImGui::GetWindowPos().y;

        return ImVec2(winMax.x-winMin.x, winMax.y-winMin.y);
    }

    bool LevelDocument::Create (int width, int height)
    {
        m_Name = "Untitled";
        m_Open = true;
        m_LevelWidth = width;
        m_LevelHeight = height;
        return true;
    }
    void LevelDocument::Update ()
    {
        glClearColor(1,0,1,1);
        glClear(GL_COLOR_BUFFER_BIT);
        // @Incomplete: ...
    }
    void LevelDocument::Open ()
    {
        // @Incomplete: ...
    }
    void LevelDocument::Save ()
    {
        // @Incomplete: ...
    }

    namespace DocumentManager
    {
        std::vector<Document*> g_Documents;
        Framebuffer g_DocumentFramebuffer;

        bool Init ()
        {
            // The size doesn't matter as we resize this per document that is visible.
            g_DocumentFramebuffer.Create(1280,720);
            return true;
        }
        void Quit ()
        {
            g_DocumentFramebuffer.Free();
            for (auto& document: g_Documents) {
                delete document;
            }
        }

        void Update ()
        {
            for (auto& document: g_Documents) {
                if (!document->m_Open) continue;
                std::string label = document->m_Name + "###Document" + document->m_ID;
                bool visible = ImGui::Begin(label.c_str(), &document->m_Open);
                if (visible) {
                    ImVec2 windowSize = document->GetWindowSize();

                    int windowWidth = static_cast<int>(windowSize.x);
                    int windowHeight = static_cast<int>(windowSize.y);

                    if (windowWidth > 0 && windowHeight > 0) { // These can report as negative, in which case we draw nothing!
                        // Only resize the framebuffer when it's necessary.
                        if (windowWidth != g_DocumentFramebuffer.m_Width || windowHeight != g_DocumentFramebuffer.m_Height) {
                            g_DocumentFramebuffer.Resize(windowWidth, windowHeight);
                        }
                        // Do the actual update and render.
                        Renderer::BindFramebuffer(&g_DocumentFramebuffer);
                        document->Update();
                        Renderer::BindFramebuffer(NULL);
                        // Draw the framebuffer content to the document window.
                        ImVec2 imagePos; // Round to avoid sub-pixeling.
                        imagePos.x = roundf((ImGui::GetWindowSize().x - windowSize.x) * 0.5f);
                        imagePos.y = roundf((ImGui::GetWindowSize().y + ImGui::GetCurrentWindow()->TitleBarHeight() - windowSize.y) * 0.5f);
                        ImGui::SetCursorPos(imagePos);
                        ImTextureID textureId = reinterpret_cast<ImTextureID>(static_cast<intptr_t>(g_DocumentFramebuffer.m_Texture));
                        ImGui::Image(textureId, windowSize, ImVec2(0,1), ImVec2(1,0)); // Invert UVs because the framebuffer will be upside down!
                    }
                }
                ImGui::End();
            }
        }

        void CreateNewLevelDocument (int width, int height)
        {
            static int id = 0;
            g_Documents.push_back(new LevelDocument);
            LevelDocument* levelDocument = dynamic_cast<LevelDocument*>(g_Documents.back());
            levelDocument->m_ID = std::to_string(id++);
            if (!levelDocument->Create(width, height)) {
                Logger::SingleSystemMessage("document", "Failed to create new level document!");
            }
        }
    }
}
