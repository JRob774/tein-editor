/*******************************************************************************
 * System for retrieving the game's palette colors for the map editor to use.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr const char* PALETTE_FILE = "textures/palette.png";
static constexpr const char* TILESET_FILE = "data/tilesets.txt";
static constexpr const char* GAME_GPAK    = "game.gpak";

// The columns in the palette file to pull the colors from.
static constexpr int PALETTE_MAIN_COLUMN = 5;

static std::map<std::string, vec4> palette_main_lookup;

/* -------------------------------------------------------------------------- */

TEINAPI void init_palette_lookup ()
{
    constexpr const char* PATH_STEAM_X86 = "C:/Program Files (x86)/Steam/steamapps/common/theendisnigh/";
    constexpr const char* PATH_STEAM_X64 = "C:/Program Files/Steam/steamapps/common/theendisnigh/";

    constexpr const char* PATH_EPIC_X86  = "C:/Program Files (x86)/Epic Games/theendisnigh/";
    constexpr const char* PATH_EPIC_X64  = "C:/Program Files)/Epic Games/theendisnigh/";

    // In order of priority of where we want to search.
    const std::vector<std::string> PATHS
    {
        make_path_absolute(""),
        PATH_STEAM_X64, PATH_STEAM_X86,
        PATH_EPIC_X64, PATH_EPIC_X86,
    };

    std::vector<u8> palette_data;
    std::vector<u8> tileset_data;

    for (auto& path: PATHS)
    {
        std::string pname(path + PALETTE_FILE);
        if (palette_data.empty() && does_file_exist(pname))
        {
            palette_data = read_binary_file(pname);
        }
        std::string tname(path + TILESET_FILE);
        if (tileset_data.empty() && does_file_exist(tname))
        {
            tileset_data = read_binary_file(tname);
        }

        // If either data does not exist then we will attempt to load from the GPAK.
        if (palette_data.empty() || tileset_data.empty())
        {
            std::string file_name(path + "game.gpak");
            std::vector<GPAK_Entry> entries;
            if (does_file_exist(file_name))
            {
                FILE* file = fopen(file_name.c_str(), "rb");
                if (file)
                {
                    Defer { fclose(file); };

                    u32 entry_count;
                    fread(&entry_count, sizeof(u32), 1, file);
                    entries.resize(entry_count);

                    for (auto& e: entries)
                    {
                        fread(&e.name_length, sizeof(u16),  1,             file);
                        e.name.resize(e.name_length);
                        fread(&e.name[0],     sizeof(char), e.name_length, file);
                        fread(&e.file_size,   sizeof(u32),  1,             file);
                    }

                    std::vector<u8> file_buffer;
                    for (auto& e: entries)
                    {
                        file_buffer.resize(e.file_size);
                        fread(&file_buffer[0], sizeof(u8), e.file_size, file);
                        if (palette_data.empty() && e.name == PALETTE_FILE)
                        {
                            palette_data = file_buffer;
                        }
                        if (tileset_data.empty() && e.name == TILESET_FILE)
                        {
                            tileset_data = file_buffer;
                        }
                    }
                }
            }
        }
        // We can leave early because we have found both files we want!
        if (!palette_data.empty() && !tileset_data.empty())
        {
            break;
        }
    }

    // If they still aren't present then we can't load the palette.
    if (palette_data.empty() || tileset_data.empty()) return;

    constexpr int BPP = 4;

    int w, h, bpp;
    u8* palette = stbi_load_from_memory(&palette_data[0], static_cast<int>(palette_data.size()), &w, &h, &bpp, BPP);
    if (!palette)
    {
        LOG_ERROR(ERR_MIN, "Failed to load palette data for the map editor!");
        return;
    }
    Defer { stbi_image_free(palette); };

    std::string buffer(tileset_data.begin(), tileset_data.end());
    GonObject gon = GonObject::LoadFromBuffer(buffer);
    for (auto it: gon.children_map)
    {
        std::string name = it.first;
        int palette_row = gon.children_array[it.second]["palette"].Int(0);

        int index = (palette_row * (w*BPP) + (PALETTE_MAIN_COLUMN   * BPP));

        float r = static_cast<float>(palette[index+0]) / 255;
        float g = static_cast<float>(palette[index+1]) / 255;
        float b = static_cast<float>(palette[index+2]) / 255;
        float a = static_cast<float>(palette[index+3]) / 255;

        palette_main_lookup.insert(std::pair<std::string, vec4>(name, vec4(r,g,b,a)));
    }
}

TEINAPI vec4 get_tileset_main_color (std::string tileset)
{
    if (tileset != "..")
    {
        if (palette_main_lookup.find(tileset) != palette_main_lookup.end())
        {
            return palette_main_lookup[tileset];
        }
        return ((is_ui_light()) ? ui_color_ex_dark : ui_color_ex_dark);
    }
    return vec4(0,0,0,1);
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
