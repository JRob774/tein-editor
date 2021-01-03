static constexpr const char* gResourceLocationFileName = "resource_location.txt";
static constexpr const char* gResourceGpakFileName = "editor.gpak";

static std::string gResourceLocation;
static std::string gCurrentEditorFont;

// Maps file names to the data that was stored within the editor GPAK.
static std::map<std::string,std::vector<U8>> gGpakResourceLookup;

TEINAPI bool InitResourceManager ()
{
    std::string gpakFileName(MakePathAbsolute(gResourceGpakFileName));
    if (!DoesFileExist(gpakFileName)) return true;

    // Return true because we may still function.
    FILE* gpak = fopen(gpakFileName.c_str(), "rb");
    if (!gpak) {
        LogError(ErrorLevel::Med, "Failed to load editor GPAK!");
        return true;
    }
    Defer { fclose(gpak); };

    std::vector<GPAKEntry> entries;
    U32 entryCount;

    fread(&entryCount, sizeof(U32), 1, gpak);
    entries.resize(entryCount);

    for (auto& e: entries) {
        fread(&e.nameLength, sizeof(U16), 1, gpak);
        e.name.resize(e.nameLength);
        fread(&e.name[0], sizeof(char), e.nameLength, gpak);
        fread(&e.fileSize, sizeof(U32), 1, gpak);
    }

    std::vector<U8> fileBuffer;
    for (auto& e: entries) {
        fileBuffer.resize(e.fileSize);
        fread(&fileBuffer[0], sizeof(U8), e.fileSize, gpak);
        gGpakResourceLookup.insert({ e.name, fileBuffer });
    }

    LogDebug("Loaded Editor GPAK");
    return true;
}

TEINAPI void GetResourceLocation ()
{
    std::string resourceLocationFile(GetExecutablePath() + gResourceLocationFileName);
    if (DoesFileExist(resourceLocationFile)) {
        std::string relativePath(ReadEntireFile(resourceLocationFile));
        gResourceLocation = GetExecutablePath() + relativePath;
        // Remove trailing whitespace, if there is any.
        gResourceLocation.erase(gResourceLocation.find_last_not_of(" \t\n\r\f\v") + 1);
    }
}

// We attempt to load resources from file first, but if they don't exist
// then we fall-back to loading them from the editor's GPAK file instead.

TEINAPI bool LoadTextureResource (std::string fileName, Texture& texture, TextureWrap wrap)
{
    std::string absoluteFileName(BuildResourceString(fileName));
    if (DoesFileExist(absoluteFileName)) return LoadTextureFromFile(texture, absoluteFileName, wrap);
    else return LoadTextureFromData(texture, gGpakResourceLookup[fileName], wrap);
}

TEINAPI bool LoadAtlasResource (std::string fileName, TextureAtlas& atlas)
{
    std::string absoluteFileName(BuildResourceString(fileName));
    if (DoesFileExist(absoluteFileName)) return LoadTextureAtlasFromFile(atlas, absoluteFileName);
    else return LoadTextureAtlasFromData(atlas, gGpakResourceLookup[fileName]);
}

TEINAPI bool LoadFontResource (std::string fileName, Font& font, std::vector<int> pointSizes, float cacheSize)
{
    std::string absoluteFileName(BuildResourceString(fileName));
    if (DoesFileExist(absoluteFileName)) return LoadFontFromFile(font, absoluteFileName, pointSizes, cacheSize);
    else return LoadFontFromData(font, gGpakResourceLookup[fileName], pointSizes, cacheSize);
}

TEINAPI Shader LoadShaderResource (std::string fileName)
{
    std::string absoluteFileName(BuildResourceString(fileName));
    if (DoesFileExist(absoluteFileName)) return LoadShaderFromFile(absoluteFileName);
    else return LoadShaderFromData(gGpakResourceLookup[fileName]);
}

TEINAPI std::vector<U8> LoadBinaryResource (std::string fileName)
{
    std::string absoluteFileName(BuildResourceString(fileName));
    if (DoesFileExist(absoluteFileName)) return ReadBinaryFile(absoluteFileName);
    else return gGpakResourceLookup[fileName];
}

TEINAPI SDL_Surface* LoadSurfaceResource (std::string fileName)
{
    std::string absoluteFileName(BuildResourceString(fileName));
    if (DoesFileExist(absoluteFileName)) return SDL_LoadBMP(absoluteFileName.c_str());
    else return SDL_LoadBMP_RW(SDL_RWFromConstMem(&gGpakResourceLookup[fileName][0],
        static_cast<int>(gGpakResourceLookup[fileName].size())), true);
}

TEINAPI std::string LoadStringResource (std::string fileName)
{
    std::string absoluteFileName(BuildResourceString(fileName));
    if (DoesFileExist(absoluteFileName)) return ReadEntireFile(absoluteFileName);
    else return std::string(gGpakResourceLookup[fileName].begin(),
        gGpakResourceLookup[fileName].end());
}

TEINAPI bool LoadEditorResources ()
{
    if (!LoadTextureResource("textures/editor_ui/tools.png", gResourceIcons)) {
        LogError(ErrorLevel::Max, "Failed to load editor icons!");
        return false;
    }
    if (!LoadTextureResource("textures/editor_ui/checker_x14.png", gResourceChecker14, TextureWrap::Repeat)) {
        LogError(ErrorLevel::Max, "Failed to load the checker-x14 image!");
        return false;
    }
    if (!LoadTextureResource("textures/editor_ui/checker_x16.png", gResourceChecker16, TextureWrap::Repeat)) {
        LogError(ErrorLevel::Max, "Failed to load the checker-x16 image!");
        return false;
    }
    if (!LoadTextureResource("textures/editor_ui/checker_x20.png", gResourceChecker20, TextureWrap::Repeat)) {
        LogError(ErrorLevel::Max, "Failed to load the checker-x20 image!");
        return false;
    }
    if (!LoadFontResource("fonts/opensans-regular.ttf", gResourceFontRegularSans, { gSmallFontPointSize, gLargeFontPointSize })) {
        LogError(ErrorLevel::Max, "Failed to load OpenSans regular font!");
        return false;
    }
    if (!LoadFontResource("fonts/opensans-bold.ttf", gResourceFontBoldSans, { gSmallFontPointSize, gLargeFontPointSize })) {
        LogError(ErrorLevel::Max, "Failed to load OpenSans bold font!");
        return false;
    }
    if (!LoadFontResource("fonts/liberationmono-regular.ttf", gResourceFontRegularLibMono)) {
        LogError(ErrorLevel::Max, "Failed to load LiberationMono regular font!");
        return false;
    }
    if (!LoadFontResource("fonts/opendyslexic-regular.ttf", gResourceFontRegularDyslexic, { gSmallFontPointSize, gLargeFontPointSize })) {
        LogError(ErrorLevel::Max, "Failed to load OpenDyslexic regular font!");
        return false;
    }
    if (!LoadFontResource("fonts/opendyslexic-bold.ttf", gResourceFontBoldDyslexic, { gSmallFontPointSize, gLargeFontPointSize })) {
        LogError(ErrorLevel::Max, "Failed to load OpenDyslexic bold font!");
        return false;
    }
    if (!LoadFontResource("fonts/opendyslexic-mono.ttf", gResourceFontMonoDyslexic)) {
        LogError(ErrorLevel::Max, "Failed to load OpenDyslexic mono font!");
        return false;
    }

    UpdateEditorFont();

    LogDebug("Loaded Editor Resources");
    return true;
}

TEINAPI void FreeEditorResources ()
{
    FreeFont(gResourceFontMonoDyslexic);
    FreeFont(gResourceFontBoldSans);
    FreeFont(gResourceFontBoldDyslexic);
    FreeFont(gResourceFontRegularLibMono);
    FreeFont(gResourceFontRegularSans);
    FreeFont(gResourceFontRegularDyslexic);
    FreeTexture(gResourceIcons);
    FreeTexture(gResourceChecker14);
    FreeTexture(gResourceChecker16);
    FreeTexture(gResourceChecker20);
    FreeTextureAtlas(gResourceLarge);
    FreeTextureAtlas(gResourceSmall);
}

TEINAPI std::string BuildResourceString (std::string pathName)
{
    return (IsPathAbsolute(pathName)) ? pathName : (gResourceLocation + pathName);
}

TEINAPI void UpdateEditorFont ()
{
    gCurrentEditorFont = gEditorSettings.fontFace;
}

TEINAPI bool IsEditorFontOpenSans ()
{
    return (gCurrentEditorFont != "OpenDyslexic");
}

TEINAPI Font& GetEditorRegularFont ()
{
    return (IsEditorFontOpenSans()) ? gResourceFontRegularSans : gResourceFontRegularDyslexic;
}
TEINAPI Font& GetEditorBoldFont ()
{
    return (IsEditorFontOpenSans()) ? gResourceFontBoldSans : gResourceFontBoldDyslexic;
}

TEINAPI TextureAtlas& GetEditorAtlasLarge ()
{
    return gResourceLarge;
}
TEINAPI TextureAtlas& GetEditorAtlasSmall ()
{
    return gResourceSmall;
}
