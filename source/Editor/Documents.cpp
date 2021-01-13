//
// Base Document
//

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

//
// Document Manager
//

EditorAPI void CreateNewLevelDocument (int width, int height)
{
    static int id = 0;
    gDocumentManager.documents.push_back(new LevelDocument);
    LevelDocument* levelDocument = dynamic_cast<LevelDocument*>(gDocumentManager.documents.back());
    levelDocument->mId = std::to_string(id++);
    if (!levelDocument->Create(width, height)) {
        LogSingleSystemMessage("document", "Failed to create new level document!");
    }
}

EditorAPI bool InitDocumentManager ()
{
    // The size doesn't matter as we resize this per document that is visible.
    CreateFramebuffer(gDocumentManager.documentFramebuffer, 1280,720);
    return true;
}
EditorAPI void QuitDocumentManager ()
{
    FreeFramebuffer(gDocumentManager.documentFramebuffer);
    for (auto& document: gDocumentManager.documents) {
        delete document;
    }
}

EditorAPI void UpdateDocuments ()
{
    for (auto& document: gDocumentManager.documents) {
        if (!document->mOpen) continue;
        std::string label = document->mName + "###Document" + document->mId;
        bool visible = ImGui::Begin(label.c_str(), &document->mOpen);
        if (visible) {
            ImVec2 windowSize = document->GetWindowSize();

            int windowWidth = static_cast<int>(windowSize.x);
            int windowHeight = static_cast<int>(windowSize.y);

            if (windowWidth > 0 && windowHeight > 0) { // These can report as negative, in which case we draw nothing!
                // Only resize the framebuffer when it's necessary.
                if (windowWidth != gDocumentManager.documentFramebuffer.width || windowHeight != gDocumentManager.documentFramebuffer.height) {
                    ResizeFramebuffer(gDocumentManager.documentFramebuffer, windowWidth, windowHeight);
                }
                // Do the actual update and render.
                BindFramebuffer(&gDocumentManager.documentFramebuffer);
                document->Update();
                BindFramebuffer(NULL);
                // Draw the framebuffer content to the document window.
                ImVec2 imagePos = (ImGui::GetWindowSize() + ImVec2(0,ImGui::GetCurrentWindow()->TitleBarHeight()) - windowSize) * 0.5f;
                imagePos.x = roundf(imagePos.x); // Round to avoid sub-pixeling.
                imagePos.y = roundf(imagePos.y);
                ImGui::SetCursorPos(imagePos);
                ImTextureID textureId = reinterpret_cast<ImTextureID>(static_cast<intptr_t>(gDocumentManager.documentFramebuffer.texture));
                ImGui::Image(textureId, windowSize, ImVec2(0,1), ImVec2(1,0)); // Invert UVs because the framebuffer will be upside down!
            }
        }
        ImGui::End();
    }
}

//
// Level Document
//

bool LevelDocument::Create (int width, int height)
{
    mName = "Untitled";
    mOpen = true;
    mLevelWidth = width;
    mLevelHeight = height;
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
