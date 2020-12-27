/*******************************************************************************
 * Loads and saves raw level data to and from the TEIN level file format.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

// The order in which layer data is stored within the .lvl files is different
// from the order it is actually represented in the editor so we use an array
// to get the correct order in which to read & write the layers to the files.

constexpr Level_Layer LEVEL_IO_ORDER[LEVEL_LAYER_TOTAL]
{
    LEVEL_LAYER_BACK1,
    LEVEL_LAYER_ACTIVE,
    LEVEL_LAYER_TAG,
    LEVEL_LAYER_OVERLAY,
    LEVEL_LAYER_BACK2
};

/* -------------------------------------------------------------------------- */

TEINAPI bool internal__load_level (FILE* file, Level& level)
{
    fread(&level.header.version, sizeof(s32), 1, file);
    fread(&level.header.width  , sizeof(s32), 1, file);
    fread(&level.header.height , sizeof(s32), 1, file);
    fread(&level.header.layers , sizeof(s32), 1, file);

    level.header.version = SDL_SwapBE32(level.header.version);
    level.header.width   = SDL_SwapBE32(level.header.width  );
    level.header.height  = SDL_SwapBE32(level.header.height );
    level.header.layers  = SDL_SwapBE32(level.header.layers );

    if (level.header.version != 1)
    {
        std::string msg(format_string("Invalid level file version '%d'!", level.header.version));
        show_alert("Error", msg, ALERT_TYPE_ERROR, ALERT_BUTTON_OK, "WINMAIN");
        return false;
    }

    s32 lw = level.header.width;
    s32 lh = level.header.height;

    for (int i=0; i<LEVEL_LAYER_TOTAL; ++i)
    {
        auto& layer = level.data[LEVEL_IO_ORDER[i]];
        layer.assign(lw*lh, 0);
        for (auto& tile: layer)
        {
            fread(&tile, sizeof(Tile_ID), 1, file);
            tile = SDL_SwapBE32(tile);
        }
    }

    return true;
}

TEINAPI void internal__save_level (FILE* file, const Level& level)
{
    s32 version = SDL_SwapBE32(level.header.version);
    s32 width   = SDL_SwapBE32(level.header.width  );
    s32 height  = SDL_SwapBE32(level.header.height );
    s32 layers  = SDL_SwapBE32(level.header.layers );

    fwrite(&version, sizeof(s32), 1, file);
    fwrite(&width  , sizeof(s32), 1, file);
    fwrite(&height , sizeof(s32), 1, file);
    fwrite(&layers , sizeof(s32), 1, file);

    for (int i=0; i<LEVEL_LAYER_TOTAL; ++i)
    {
        auto& layer = level.data[LEVEL_IO_ORDER[i]];
        for (auto& tile: layer)
        {
            Tile_ID id = SDL_SwapBE32(tile);
            fwrite(&id, sizeof(Tile_ID), 1, file);
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI bool load_level (Level& level, std::string file_name)
{
    // We don't make the path absolute or anything becuase if that is needed
    // then it should be handled by a higher-level than this internal system.

    FILE* file = fopen(file_name.c_str(), "rb");
    if (!file)
    {
        LOG_ERROR(ERR_MED, "Failed to load level file '%s'!", file_name.c_str());
        return false;
    }
    defer { fclose(file); };

    // If the level is empty/blank we just create a blank default level.
    if (get_size_of_file(file) == 0) return create_blank_level(level);
    else                             return internal__load_level(file, level);
}

TEINAPI bool save_level (const Level& level, std::string file_name)
{
    // We don't make the path absolute or anything becuase if that is needed
    // then it should be handled by a higher-level than this internal system.

    FILE* file = fopen(file_name.c_str(), "wb");
    if (!file)
    {
        LOG_ERROR(ERR_MED, "Failed to save level file '%s'!", file_name.c_str());
        return false;
    }
    defer { fclose(file); };

    internal__save_level(file, level);
    return true;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool load_restore_level (Tab& tab, std::string file_name)
{
    FILE* file = fopen(file_name.c_str(), "rb");
    if (!file)
    {
        LOG_ERROR(ERR_MED, "Failed to load restore file '%s'!", file_name.c_str());
        return false;
    }
    defer { fclose(file); };

    // Read until the null-terminator to get the name of the level.
    std::string level_name;
    char c = 0;
    do
    {
        fread(&c, sizeof(char), 1, file);
        if (c) { level_name.push_back(c); }
    }
    while (c);

    // Set the name of the level for the tab we are loading into.
    tab.name = level_name;

    internal__load_level(file, tab.level);
    return true;
}

TEINAPI bool save_restore_level (const Tab& tab, std::string file_name)
{
    FILE* file = fopen(file_name.c_str(), "wb");
    if (!file)
    {
        LOG_ERROR(ERR_MED, "Failed to save restore file '%s'!", file_name.c_str());
        return false;
    }
    defer { fclose(file); };

    // Write the name of the level + null-terminator for later restoration.
    if (tab.name.empty())
    {
        char null_terminator = '\0';
        fwrite(&null_terminator, sizeof(char), 1, file);
    }
    else
    {
        const char* name = tab.name.c_str();
        fwrite(name, sizeof(char), strlen(name)+1, file);
    }

    internal__save_level(file, tab.level);
    return true;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool create_blank_level (Level& level, int w, int h)
{
    level.header.version = 1;
    level.header.width   = w;
    level.header.height  = h;
    level.header.layers  = LEVEL_LAYER_TOTAL;

    s32 lw = level.header.width;
    s32 lh = level.header.height;

    for (auto& layer: level.data) layer.assign(lw*lh, 0);

    return true;
}

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

/*******************************************************************************
 *
 * Copyright (c) 2020 Joshua Robertson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
*******************************************************************************/
