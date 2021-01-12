//
// Base Document
//

class Document
{
public:
    virtual void Update () = 0;
    virtual void Open   () = 0;
    virtual void Save   () = 0;

    std::string mName;
    std::string mId;
    bool        mOpen;
};

//
// Document Manager
//

struct DocumentManager
{
    std::vector<Document*> documents;
};

Global DocumentManager gDocumentManager;

EditorAPI void CreateNewLevelDocument (int width, int height);

EditorAPI void UpdateDocuments ();
EditorAPI void FreeDocuments ();

//
// Level Document
//

class LevelDocument: public Document
{
public:
    bool Create (int width, int height);
    void Update () override;
    void Open   () override;
    void Save   () override;
private:
    int mLevelWidth;
    int mLevelHeight;
};
