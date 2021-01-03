static constexpr const char* gPaletteFile = "textures/palette.png";
static constexpr const char* gTilesetFile = "data/tilesets.txt";

static constexpr const char* gGameGPAK = "game.gpak";

// The columns in the palette file to pull the colors from.
static constexpr int gPaletteMainColumn = 5;

static std::map<std::string,Vec4> gPaletteMainLookup;

TEINAPI void InitPaletteLookup ()
{
    constexpr const char* PathSteamX86 = "C:/Program Files (x86)/Steam/steamapps/common/theendisnigh/";
    constexpr const char* PathSteamX64 = "C:/Program Files/Steam/steamapps/common/theendisnigh/";

    constexpr const char* PathEpicX86 = "C:/Program Files (x86)/Epic Games/theendisnigh/";
    constexpr const char* PathEpicX64 = "C:/Program Files)/Epic Games/theendisnigh/";

    // In order of priority of where we want to search.
    const std::vector<std::string> Paths = { MakePathAbsolute(""), PathSteamX64, PathSteamX86, PathEpicX64, PathEpicX86 };

    std::vector<U8> paletteData;
    std::vector<U8> tilesetData;

    for (auto& path: Paths) {
        std::string paletteName(path + gPaletteFile);
        if (paletteData.empty() && DoesFileExist(paletteName)) paletteData = ReadBinaryFile(paletteName);
        std::string tilesetName(path + gTilesetFile);
        if (tilesetData.empty() && DoesFileExist(tilesetName)) tilesetData = ReadBinaryFile(tilesetName);

        // If either data does not exist then we will attempt to load from the GPAK.
        if (paletteData.empty() || tilesetData.empty()) {
            std::string fileName(path + gGameGPAK);
            std::vector<GPAKEntry> entries;
            if (DoesFileExist(fileName)) {
                FILE* file = fopen(fileName.c_str(), "rb");
                if (file) {
                    Defer { fclose(file); };

                    U32 entryCount;
                    fread(&entryCount, sizeof(U32), 1, file);
                    entries.resize(entryCount);

                    for (auto& e: entries) {
                        fread(&e.nameLength, sizeof(U16), 1, file);
                        e.name.resize(e.nameLength);
                        fread(&e.name[0], sizeof(char), e.nameLength, file);
                        fread(&e.fileSize, sizeof(U32), 1, file);
                    }

                    std::vector<U8> fileBuffer;
                    for (auto& e: entries) {
                        fileBuffer.resize(e.fileSize);
                        fread(&fileBuffer[0], sizeof(U8), e.fileSize, file);

                        if (paletteData.empty() && e.name == gPaletteFile) paletteData = fileBuffer;
                        if (tilesetData.empty() && e.name == gTilesetFile) tilesetData = fileBuffer;
                    }
                }
            }
        }
        // We can leave early because we have found both files we want!
        if (!paletteData.empty() && !tilesetData.empty()) break;
    }

    // If they still aren't present then we can't load the palette.
    if (paletteData.empty() || tilesetData.empty()) return;

    constexpr int Bpp = 4;

    int w,h,bpp;
    U8* palette = stbi_load_from_memory(&paletteData[0], static_cast<int>(paletteData.size()), &w,&h,&bpp,Bpp);
    if (!palette) {
        LogError(ErrorLevel::Min, "Failed to load palette data for the map editor!");
        return;
    }
    Defer { stbi_image_free(palette); };

    std::string buffer(tilesetData.begin(), tilesetData.end());
    GonObject gon = GonObject::LoadFromBuffer(buffer);
    for (auto [name,index]: gon.children_map) {
        int paletteRow = gon.children_array[index]["palette"].Int(0);
        int paletteIndex = (paletteRow * (w*Bpp) + (gPaletteMainColumn * Bpp));

        float r = static_cast<float>(palette[paletteIndex+0]) / 255;
        float g = static_cast<float>(palette[paletteIndex+1]) / 255;
        float b = static_cast<float>(palette[paletteIndex+2]) / 255;
        float a = static_cast<float>(palette[paletteIndex+3]) / 255;

        gPaletteMainLookup.insert({ name, Vec4(r,g,b,a) });
    }
}

TEINAPI Vec4 GetTilesetMainColor (std::string tileset)
{
    if (tileset != "..") {
        if (gPaletteMainLookup.find(tileset) != gPaletteMainLookup.end()) {
            return gPaletteMainLookup[tileset];
        }
        return gUiColorExDark;
    }
    return Vec4(0,0,0,1);
}
