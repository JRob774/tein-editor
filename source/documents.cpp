//
// Base Document
//

Vec2 Document::GetWindowSize () const
{
    ImVec2 winMin = ImGui::GetWindowContentRegionMin();
    ImVec2 winMax = ImGui::GetWindowContentRegionMax();

    winMin.x += ImGui::GetWindowPos().x;
    winMin.y += ImGui::GetWindowPos().y;
    winMax.x += ImGui::GetWindowPos().x;
    winMax.y += ImGui::GetWindowPos().y;

    return { winMax.x-winMin.x, winMax.y-winMin.y };
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
            Vec2 windowSize = document->GetWindowSize();
            int windowWidth = static_cast<int>(windowSize.x);
            int windowHeight = static_cast<int>(windowSize.y);
            if (windowWidth > 0 && windowHeight > 0) { // These can report as negative, in which case we draw nothing!
                // Only resize the framebuffer when it's necessary.
                if (windowWidth != gDocumentManager.documentFramebuffer.width || windowHeight != gDocumentManager.documentFramebuffer.height) {
                    ResizeFramebuffer(gDocumentManager.documentFramebuffer, windowWidth, windowHeight);
                }
                document->Update();
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
