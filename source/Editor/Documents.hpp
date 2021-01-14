#pragma once

#include "Renderer.hpp"

#include <string>
#include <vector>

#include <imgui/imgui.h>

namespace TEIN
{
    class Document // Base type for documents, all custom document types should inherit from this!
    {
    public:
        virtual ~Document   () = default;
        virtual void Update () = 0;
        virtual void Open   () = 0;
        virtual void Save   () = 0;

        ImVec2 GetWindowSize () const; // Note: Only safe to be called inside Update()!

        std::string m_Name;
        std::string m_ID;
        bool        m_Open;
    };

    class LevelDocument: public Document
    {
    public:
        bool Create (int width, int height);
        void Update () override;
        void Open   () override;
        void Save   () override;
    private:
        int m_LevelWidth;
        int m_LevelHeight;
    };

    namespace DocumentManager
    {
        extern std::vector<Document*> g_Documents;
        extern Framebuffer g_DocumentFramebuffer;

        bool Init   ();
        void Quit   ();
        void Update ();

        void CreateNewLevelDocument (int width, int height);
    }
}
