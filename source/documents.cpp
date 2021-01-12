//
// Base Document
//

// ...

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

EditorAPI void UpdateDocuments ()
{
    for (auto& document: gDocumentManager.documents) {
        if (!document->mOpen) continue;
        std::string label = document->mName + "###" + document->mId;
        bool visible = ImGui::Begin(label.c_str(), &document->mOpen, ImGuiWindowFlags_NoSavedSettings); // @Improve: Do we want to save documents to settings???
        if (visible) document->Update();
        ImGui::End();
    }
}
EditorAPI void FreeDocuments ()
{
    for (auto& document: gDocumentManager.documents) {
        delete document;
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
