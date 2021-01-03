// The order in which layer data is stored within the .lvl files is different
// from the order it is actually represented in the editor so we use an array
// to get the correct order in which to read & write the layers to the files.
constexpr LevelLayer gLevelIOOrder [static_cast<int>(LevelLayer::Total)]
{
    LevelLayer::Back1,
    LevelLayer::Active,
    LevelLayer::Tag,
    LevelLayer::Overlay,
    LevelLayer::Back2
};

namespace Internal
{
    TEINAPI bool LoadLevel (FILE* file, Level& level)
    {
        fread(&level.header.version, sizeof(S32), 1, file);
        fread(&level.header.width,   sizeof(S32), 1, file);
        fread(&level.header.height,  sizeof(S32), 1, file);
        fread(&level.header.layers,  sizeof(S32), 1, file);

        level.header.version = SDL_SwapBE32(level.header.version);
        level.header.width   = SDL_SwapBE32(level.header.width  );
        level.header.height  = SDL_SwapBE32(level.header.height );
        level.header.layers  = SDL_SwapBE32(level.header.layers );

        if (level.header.version != 1) {
            std::string msg(FormatString("Invalid level file version '%d'!", level.header.version));
            ShowAlert("Error", msg, AlertType::Error, AlertButton::Ok, "WINMAIN");
            return false;
        }

        S32 lw = level.header.width;
        S32 lh = level.header.height;

        for (int i=0; i<static_cast<int>(LevelLayer::Total); ++i) {
            auto& layer = level.data[static_cast<int>(gLevelIOOrder[i])];
            layer.assign(lw*lh, 0);
            for (auto& tile: layer) {
                fread(&tile, sizeof(TileID), 1, file);
                tile = SDL_SwapBE32(tile);
            }
        }

        return true;
    }

    TEINAPI void SaveLevel (FILE* file, const Level& level)
    {
        S32 version = SDL_SwapBE32(level.header.version);
        S32 width   = SDL_SwapBE32(level.header.width  );
        S32 height  = SDL_SwapBE32(level.header.height );
        S32 layers  = SDL_SwapBE32(level.header.layers );

        fwrite(&version, sizeof(S32), 1, file);
        fwrite(&width,   sizeof(S32), 1, file);
        fwrite(&height,  sizeof(S32), 1, file);
        fwrite(&layers,  sizeof(S32), 1, file);

        for (int i=0; i<static_cast<int>(LevelLayer::Total); ++i) {
            auto& layer = level.data[static_cast<int>(gLevelIOOrder[i])];
            for (auto& tile: layer) {
                TileID id = SDL_SwapBE32(tile);
                fwrite(&id, sizeof(TileID), 1, file);
            }
        }
    }
}

TEINAPI bool CreateBlankLevel (Level& level, int w, int h)
{
    level.header.version = 1;
    level.header.width = w;
    level.header.height = h;
    level.header.layers = static_cast<S32>(LevelLayer::Total);

    S32 lw = level.header.width;
    S32 lh = level.header.height;

    for (auto& layer: level.data) layer.assign(lw*lh, 0);

    return true;
}

TEINAPI bool LoadLevel (Level& level, std::string fileName)
{
    // We don't make the path absolute or anything becuase if that is needed
    // then it should be handled by a higher-level than this internal system.

    FILE* file = fopen(fileName.c_str(), "rb");
    if (!file) {
        LogError(ErrorLevel::Med, "Failed to load level file '%s'!", fileName.c_str());
        return false;
    }
    Defer { fclose(file); };

    // If the level is empty/blank we just create a blank default level.
    if (GetSizeOfFile(file) == 0) return CreateBlankLevel(level);
    else return Internal::LoadLevel(file, level);
}

TEINAPI bool SaveLevel (const Level& level, std::string fileName)
{
    // We don't make the path absolute or anything becuase if that is needed
    // then it should be handled by a higher-level than this internal system.

    FILE* file = fopen(fileName.c_str(), "wb");
    if (!file) {
        LogError(ErrorLevel::Med, "Failed to save level file '%s'!", fileName.c_str());
        return false;
    }
    Defer { fclose(file); };

    Internal::SaveLevel(file, level);
    return true;
}

TEINAPI bool LoadRestoreLevel (Tab& tab, std::string fileName)
{
    FILE* file = fopen(fileName.c_str(), "rb");
    if (!file) {
        LogError(ErrorLevel::Med, "Failed to load restore file '%s'!", fileName.c_str());
        return false;
    }
    Defer { fclose(file); };

    // Read until the null-terminator to get the name of the level.
    std::string levelName;
    char c = 0;
    do {
        fread(&c, sizeof(char), 1, file);
        if (c) { levelName.push_back(c); }
    } while (c);

    // Set the name of the level for the tab we are loading into.
    tab.name = levelName;

    Internal::LoadLevel(file, tab.level);
    return true;
}

TEINAPI bool SaveRestoreLevel (const Tab& tab, std::string fileName)
{
    FILE* file = fopen(fileName.c_str(), "wb");
    if (!file) {
        LogError(ErrorLevel::Med, "Failed to save restore file '%s'!", fileName.c_str());
        return false;
    }
    Defer { fclose(file); };

    // Write the name of the level + null-terminator for later restoration.
    if (tab.name.empty()) {
        char nullTerminator = '\0';
        fwrite(&nullTerminator, sizeof(char), 1, file);
    } else {
        const char* name = tab.name.c_str();
        fwrite(name, sizeof(char), strlen(name)+1, file);
    }

    Internal::SaveLevel(file, tab.level);
    return true;
}
