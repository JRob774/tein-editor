//
// Base Document
//

class Document
{
public:
    virtual void Update () = 0;
    virtual void Open   () = 0;
    virtual void Save   () = 0;

    Vec2 GetWindowSize () const; // Note: Only safe to be called inside Update()!

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
    Framebuffer documentFramebuffer;
};

Global DocumentManager gDocumentManager;

EditorAPI void CreateNewLevelDocument (int width, int height);

EditorAPI bool InitDocumentManager ();
EditorAPI void QuitDocumentManager ();

EditorAPI void UpdateDocuments ();

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
