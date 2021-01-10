// Nothing...
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

Internal bool Internal_LoadLevel (FILE* file, Level& level)
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
        LogSingleSystemMessage("level", "Invalid level file header version: %d", level.header.version);
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

Internal void Internal_SaveLevel (FILE* file, const Level& level)
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

EditorAPI bool CreateBlankLevel (Level& level, int w, int h)
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

EditorAPI bool LoadLevel (Level& level, std::string fileName)
{
    // We don't make the path absolute or anything becuase if that is needed
    // then it should be handled by a higher-level than this internal system.

    FILE* file = fopen(fileName.c_str(), "rb");
    if (!file) {
        LogSingleSystemMessage("level", "Failed to load level file: %s", fileName.c_str());
        return false;
    }
    Defer { fclose(file); };

    // If the level is empty/blank we just create a blank default level.
    if (std::filesystem::file_size(fileName) == 0) return CreateBlankLevel(level);
    else return Internal_LoadLevel(file, level);
}

EditorAPI bool SaveLevel (const Level& level, std::string fileName)
{
    // We don't make the path absolute or anything becuase if that is needed
    // then it should be handled by a higher-level than this internal system.

    FILE* file = fopen(fileName.c_str(), "wb");
    if (!file) {
        LogSingleSystemMessage("level", "Failed to save level file: %s", fileName.c_str());
        return false;
    }
    Defer { fclose(file); };

    Internal_SaveLevel(file, level);
    return true;
}
