static const Vec4 gGPAKProgressBarMinColor = { .2f,.5f,.2f, 1 };
static const Vec4 gGPAKProgressBarMaxColor = { .2f,.9f,.2f, 1 };

static SDL_Thread* gGPAKUnpackThread;
static SDL_Thread* gGPAKPackThread;

struct GPAKUnpackData
{
    std::string fileName;
    bool overwrite;

    std::atomic<float> progress; // In percent.
    std::atomic<bool>  complete = true; // Default to true so we can press the unpack button.
    std::atomic<bool>  cancel;

    std::atomic<GPAKError> error;
};

struct GPAKPackData
{
    std::string fileName;
    std::vector<std::string> paths;

    std::atomic<float> progress; // In percent.
    std::atomic<bool>  complete = true; // Default to true so we can press the pack button.
    std::atomic<bool>  cancel;

    std::atomic<GPAKError> error;
};

static GPAKUnpackData gGPAKUnpackData;
static GPAKPackData gGPAKPackData;

namespace Internal
{
    TEINAPI void HandleGPAKError (GPAKError error)
    {
        switch (error)
        {
            case(GPAKError::Write): LogError(ErrorLevel::Med, "Failed to write GPAK data!"); break;
            case(GPAKError::Read): LogError(ErrorLevel::Med, "Failed to read GPAK data!"); break;
            case(GPAKError::Empty): LogError(ErrorLevel::Med, "No files found to pack!"); break;
        }
    }

    TEINAPI int GPAKUnpackThreadMain (void* userData)
    {
        GPAKUnpackData* args = reinterpret_cast<GPAKUnpackData*>(userData);

        std::string fileName = args->fileName;
        bool overwrite = args->overwrite;

        FILE* file = fopen(fileName.c_str(), "rb");
        if (!file)
        {
            args->error.store(GPAKError::Read);
            return EXIT_FAILURE;
        }
        Defer { fclose(file); };

        std::vector<GPAKEntry> entries;
        U32 entryCount;

        fread(&entryCount, sizeof(32), 1, file);
        entries.resize(entryCount);

        for (auto& e: entries)
        {
            fread(&e.nameLength, sizeof(U16), 1, file);
            e.name.resize(e.nameLength);
            fread(&e.name[0], sizeof(char), e.nameLength, file);
            fread(&e.fileSize, sizeof(U32), 1, file);

            if (args->cancel.load()) // Cancel!
            {
                return EXIT_SUCCESS;
            }
        }

        size_t maxSize = std::max_element(entries.begin(), entries.end(), [](const GPAKEntry& a, const GPAKEntry& b) { return (a.fileSize < b.fileSize); })->fileSize;

        std::vector<U8> fileBuffer;
        fileBuffer.resize(maxSize);

        U32 entriesUnpacked = 0;

        std::string basePath(StripFileName(fileName));
        for (auto& e: entries)
        {
            fread(&fileBuffer[0], sizeof(U8), e.fileSize, file);

            std::string fullFileName(basePath + e.name);
            std::string fullFilePath(StripFileName(fullFileName));

            if (!DoesPathExist(fullFilePath))
            {
                CreatePath(fullFilePath);
            }
            if (!DoesFileExist(fullFileName) || overwrite)
            {
                FILE* output = fopen(fullFileName.c_str(), "wb");
                if (output)
                {
                    fwrite(&fileBuffer[0], sizeof(U8), e.fileSize, output);
                    fclose(output);
                }
            }

            ++entriesUnpacked;
            args->progress.store(static_cast<float>(entriesUnpacked) / static_cast<float>(entryCount));
            PushEditorEvent(EditorEvent::GPAKProgress, NULL, NULL);

            if (args->cancel.load()) // Cancel!
            {
                return EXIT_SUCCESS;
            }
        }

        args->complete.store(true);
        PushEditorEvent(EditorEvent::GPAKProgress, NULL, NULL);

        return EXIT_SUCCESS;
    }

    TEINAPI int GPAKPackThreadMain (void* userData)
    {
        GPAKPackData* args = reinterpret_cast<GPAKPackData*>(userData);

        std::string fileName = args->fileName;
        std::vector<std::string> paths = args->paths;

        // We say approximate because this isn't true if you are loading the loader
        // GPAK or if it is a modified GPAK with extra stuff in (added mod files).
        constexpr size_t ApproxGPAKEntries = 1340;

        FILE* file = fopen(fileName.c_str(), "wb");
        if (!file)
        {
            args->error.store(GPAKError::Write);
            return EXIT_FAILURE;
        }
        Defer { fclose(file); };

        std::vector<std::string> files;
        std::vector<std::string> strippedFiles;

        files.reserve(ApproxGPAKEntries);
        strippedFiles.reserve(ApproxGPAKEntries);

        for (auto path: paths)
        {
            std::vector<std::string> tempFiles;
            ListPathFiles(path, tempFiles, true);

            files.insert(files.end(), tempFiles.begin(), tempFiles.end());

            std::string strippedPath(path.substr(0, path.find_last_of("/\\")+1));
            for (auto& f: tempFiles) f = f.substr(strippedPath.length());
            strippedFiles.insert(strippedFiles.end(), tempFiles.begin(), tempFiles.end());
        }

        if (files.empty())
        {
            args->error.store(GPAKError::Empty);
            return EXIT_FAILURE;
        }

        assert(files.size() == strippedFiles.size());

        U32 entryCount = static_cast<U32>(files.size());
        fwrite(&entryCount, sizeof(U32), 1, file);

        U32 entriesPacked = 0;

        size_t maxSize = 0;
        for (size_t i=0; i<files.size(); ++i)
        {
            std::string name = strippedFiles.at(i);
            U16 nameLength = static_cast<U16>(name.length());
            U32 fileSize = static_cast<U32>(GetSizeOfFile(files.at(i)));

            maxSize = std::max(maxSize, static_cast<size_t>(fileSize));

            fwrite(&nameLength, sizeof(U16), 1, file);
            fwrite(name.c_str(), sizeof(char), nameLength, file);
            fwrite(&fileSize, sizeof(U32), 1, file);

            ++entriesPacked;
            args->progress.store(((static_cast<float>(entriesPacked) / static_cast<float>(entryCount)) / 2));
            PushEditorEvent(EditorEvent::GPAKProgress, NULL, NULL);

            if (args->cancel.load()) // Cancel!
            {
                return EXIT_SUCCESS;
            }
        }

        std::vector<U8> fileBuffer;
        fileBuffer.resize(maxSize);

        U32 fileCount = static_cast<U32>(files.size());
        U32 filesPacked = 0;

        for (auto& f: files)
        {
            FILE* input = fopen(f.c_str(), "rb");
            if (input)
            {
                size_t fileSize = GetSizeOfFile(input);
                fread(&fileBuffer[0], sizeof(U8), fileSize, input);
                fwrite(&fileBuffer[0], sizeof(U8), fileSize, file);
                fclose(input);
            }

            ++filesPacked;
            args->progress.store(.5f + ((static_cast<float>(filesPacked) / static_cast<float>(fileCount)) / 2));
            PushEditorEvent(EditorEvent::GPAKProgress, NULL, NULL);

            if (args->cancel.load()) // Cancel!
            {
                return EXIT_SUCCESS;
            }
        }

        args->complete.store(true);
        PushEditorEvent(EditorEvent::GPAKProgress, NULL, NULL);

        return EXIT_SUCCESS;
    }
}

TEINAPI void GPAKUnpack (std::string fileName, bool overwrite)
{
    gGPAKUnpackData.fileName = fileName;
    gGPAKUnpackData.overwrite = overwrite;
    gGPAKUnpackData.progress = 0;
    gGPAKUnpackData.complete = false;
    gGPAKUnpackData.cancel = false;
    gGPAKUnpackData.error = GPAKError::None;

    gGPAKUnpackThread = SDL_CreateThread(Internal::GPAKUnpackThreadMain, "UnpackGPAK", &gGPAKUnpackData);
    if (!gGPAKUnpackThread)
    {
        LogError(ErrorLevel::Med, "Failed to perform GPAK unpack operation! (%s)", SDL_GetError());
        return;
    }
    SDL_DetachThread(gGPAKUnpackThread);

    ShowWindow("WINUNPACK");
}

TEINAPI void GPAKPack (std::string fileName, std::vector<std::string> paths)
{
    gGPAKPackData.fileName = fileName;
    gGPAKPackData.paths = paths;
    gGPAKPackData.progress = 0;
    gGPAKPackData.complete = false;
    gGPAKPackData.cancel = false;
    gGPAKPackData.error = GPAKError::None;

    gGPAKPackThread = SDL_CreateThread(Internal::GPAKPackThreadMain, "PackGPAK", &gGPAKPackData);
    if (!gGPAKPackThread)
    {
        LogError(ErrorLevel::Med, "Failed to perform GPAK pack operation! (%s)", SDL_GetError());
        return;
    }
    SDL_DetachThread(gGPAKPackThread);

    ShowWindow("WINPACK");
}

TEINAPI float GPAKUnpackProgress ()
{
    return gGPAKUnpackData.progress.load();
}
TEINAPI float GPAKPackProgress ()
{
    return gGPAKPackData.progress.load();
}

TEINAPI bool IsGPAKUnpackComplete ()
{
    return gGPAKUnpackData.complete.load();
}
TEINAPI bool IsGPAKPackComplete ()
{
    return gGPAKPackData.complete.load();
}

TEINAPI void DoUnpack ()
{
    if (IsWindowHidden("WINUNPACK")) return;

    Quad p1, p2;

    p1.x = gWindowBorder;
    p1.y = gWindowBorder;
    p1.w = GetViewport().w - (gWindowBorder * 2);
    p1.h = GetViewport().h - (gWindowBorder * 2);

    SetUiFont(&GetEditorRegularFont());

    BeginPanel(p1, UiFlag::None, gUiColorExDark);

    float vw = GetViewport().w;
    float vh = GetViewport().h;

    p2.x =             1;
    p2.y =             1;
    p2.w = vw        - 2;
    p2.h = vh - p2.y - 1;

    BeginPanel(p2, UiFlag::None, gUiColorMedium);

    constexpr float XPad = 8;
    constexpr float YPad = 4;

    Vec2 cursor(XPad, YPad);

    SetPanelCursorDir(UiDir::Down);
    SetPanelCursor(&cursor);

    constexpr float LabelHeight = 24;
    constexpr float BarHeight = 20;

    DoLabel(UiAlign::Left,UiAlign::Center, LabelHeight, "Unpacking GPAK...");

    cursor.y += (YPad*2);

    float totalWidth = GetViewport().w - (XPad*2);
    float currentWidth = totalWidth * GPAKUnpackProgress();

    float x1 = cursor.x;
    float y1 = cursor.y;
    float x2 = cursor.x + currentWidth;
    float y2 = cursor.y + BarHeight;

    SetDrawColor(gUiColorLight);
    FillQuad(x1-2, y1-2, cursor.x+totalWidth+2, y2+2);
    SetDrawColor(gUiColorExDark);
    FillQuad(x1-1, y1-1, cursor.x+totalWidth+1, y2+1);

    BeginDraw(BufferMode::TriangleStrip);
    PutVertex(x1, y2, gGPAKProgressBarMinColor); // BL
    PutVertex(x1, y1, gGPAKProgressBarMinColor); // TL
    PutVertex(x2, y2, gGPAKProgressBarMaxColor); // BR
    PutVertex(x2, y1, gGPAKProgressBarMaxColor); // TR
    EndDraw();

    EndPanel();
    EndPanel();

    if (IsGPAKUnpackComplete())
    {
        Internal::HandleGPAKError(gGPAKUnpackData.error.load());
        HideWindow("WINUNPACK");
    }
}

TEINAPI void DoPack ()
{
    if (IsWindowHidden("WINPACK")) return;

    Quad p1, p2;

    p1.x = gWindowBorder;
    p1.y = gWindowBorder;
    p1.w = GetViewport().w - (gWindowBorder * 2);
    p1.h = GetViewport().h - (gWindowBorder * 2);

    SetUiFont(&GetEditorRegularFont());

    BeginPanel(p1, UiFlag::None, gUiColorExDark);

    float vw = GetViewport().w;
    float vh = GetViewport().h;

    p2.x =             1;
    p2.y =             1;
    p2.w = vw        - 2;
    p2.h = vh - p2.y - 1;

    BeginPanel(p2, UiFlag::None, gUiColorMedium);

    constexpr float XPad = 8;
    constexpr float YPad = 4;

    Vec2 cursor(XPad, YPad);

    SetPanelCursorDir(UiDir::Down);
    SetPanelCursor(&cursor);

    constexpr float LabelHeight = 24;
    constexpr float BarHeight = 20;

    DoLabel(UiAlign::Left,UiAlign::Center, LabelHeight, "Packing GPAK...");

    cursor.y += (YPad*2);

    float totalWidth = GetViewport().w - (XPad*2);
    float currentWidth = totalWidth * GPAKPackProgress();

    float x1 = cursor.x;
    float y1 = cursor.y;
    float x2 = cursor.x + currentWidth;
    float y2 = cursor.y + BarHeight;

    SetDrawColor(gUiColorLight);
    FillQuad(x1-2, y1-2, cursor.x+totalWidth+2, y2+2);
    SetDrawColor(gUiColorExDark);
    FillQuad(x1-1, y1-1, cursor.x+totalWidth+1, y2+1);

    BeginDraw(BufferMode::TriangleStrip);
    PutVertex(x1, y2, gGPAKProgressBarMinColor); // BL
    PutVertex(x1, y1, gGPAKProgressBarMinColor); // TL
    PutVertex(x2, y2, gGPAKProgressBarMaxColor); // BR
    PutVertex(x2, y1, gGPAKProgressBarMaxColor); // TR
    EndDraw();

    EndPanel();
    EndPanel();

    if (IsGPAKPackComplete())
    {
        Internal::HandleGPAKError(gGPAKPackData.error.load());
        HideWindow("WINPACK");
    }
}

TEINAPI void CancelUnpack ()
{
    gGPAKUnpackData.complete.store(true);
    gGPAKUnpackData.cancel.store(true);
    HideWindow("WINUNPACK");
}
TEINAPI void CancelPack ()
{
    gGPAKPackData.complete.store(true);
    gGPAKPackData.cancel.store(true);
    HideWindow("WINPACK");
}
