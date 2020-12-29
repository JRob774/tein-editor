/*******************************************************************************
 * Loads and manages both packed and loose resources used by the application.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr const char* RESOURCE_LOCATION = "resource_location.txt";
static constexpr const char* RESOURCE_GPAK = "editor.gpak";

static std::string resource_location;
static std::string current_editor_font;

// Maps file names to the data that was stored within the editor GPAK.
static std::map<std::string, std::vector<U8>> gpak_resource_lookup;

/* -------------------------------------------------------------------------- */

TEINAPI bool init_resource_manager ()
{
    std::string gpak_file_name(MakePathAbsolute(RESOURCE_GPAK));
    if (!DoesFileExist(gpak_file_name)) return true;

    // Return true because we may still function.
    FILE* gpak = fopen(gpak_file_name.c_str(), "rb");
    if (!gpak)
    {
        LogError(ERR_MED, "Failed to load editor GPAK!");
        return true;
    }
    Defer { fclose(gpak); };

    std::vector<GPAK_Entry> entries;
    U32 entry_count;

    fread(&entry_count, sizeof(U32), 1, gpak);
    entries.resize(entry_count);

    for (auto& e: entries)
    {
        fread(&e.name_length, sizeof(U16),  1,             gpak);
        e.name.resize(e.name_length);
        fread(&e.name[0],     sizeof(char), e.name_length, gpak);
        fread(&e.file_size,   sizeof(U32),  1,             gpak);
    }

    std::vector<U8> file_buffer;
    for (auto& e: entries)
    {
        file_buffer.resize(e.file_size);
        fread(&file_buffer[0], sizeof(U8), e.file_size, gpak);
        gpak_resource_lookup.insert(std::pair<std::string, std::vector<U8>>(e.name, file_buffer));
    }

    LogDebug("Loaded Editor GPAK");
    return true;
}

TEINAPI void get_resource_location ()
{
    std::string resource_location_file(GetExecutablePath() + RESOURCE_LOCATION);
    if (DoesFileExist(resource_location_file))
    {
        std::string relative_path(ReadEntireFile(resource_location_file));
        resource_location = GetExecutablePath() + relative_path;
        // Remove trailing whitespace, if there is any.
        resource_location.erase(resource_location.find_last_not_of(" \t\n\r\f\v") + 1);
    }
}

/* -------------------------------------------------------------------------- */

// We attempt to load resources from file first, but if they don't exist
// then we fall-back to loading them from the editor's GPAK file instead.

TEINAPI bool load_texture_resource (std::string file_name, Texture& tex, TextureWrap wrap)
{
    std::string abs_file_name(build_resource_string(file_name));
    if (DoesFileExist(abs_file_name)) return LoadTextureFromFile(tex, abs_file_name, wrap);
    else return LoadTextureFromData(tex, gpak_resource_lookup[file_name], wrap);
}

TEINAPI bool load_atlas_resource (std::string file_name, Texture_Atlas& atlas)
{
    std::string abs_file_name(build_resource_string(file_name));
    if (DoesFileExist(abs_file_name)) return load_texture_atlas_from_file(atlas, abs_file_name);
    else return load_texture_atlas_from_data(atlas, gpak_resource_lookup[file_name]);
}

TEINAPI bool load_font_resource (std::string file_name, Font& fnt, std::vector<int> pt, float csz)
{
    std::string abs_file_name(build_resource_string(file_name));
    if (DoesFileExist(abs_file_name)) return load_font_from_file(fnt, abs_file_name, pt, csz);
    else return load_font_from_data(fnt, gpak_resource_lookup[file_name], pt, csz);
}

TEINAPI Shader load_shader_resource (std::string file_name)
{
    std::string abs_file_name(build_resource_string(file_name));
    if (DoesFileExist(abs_file_name)) return load_shader_from_file(abs_file_name);
    else return load_shader_from_data(gpak_resource_lookup[file_name]);
}

TEINAPI std::vector<U8> load_binary_resource (std::string file_name)
{
    std::string abs_file_name(build_resource_string(file_name));
    if (DoesFileExist(abs_file_name)) return ReadBinaryFile(abs_file_name);
    else return gpak_resource_lookup[file_name];
}

TEINAPI SDL_Surface* load_surface_resource (std::string file_name)
{
    std::string abs_file_name(build_resource_string(file_name));
    if (DoesFileExist(abs_file_name)) return SDL_LoadBMP(abs_file_name.c_str());
    else return SDL_LoadBMP_RW(SDL_RWFromConstMem(&gpak_resource_lookup[file_name][0],
        static_cast<int>(gpak_resource_lookup[file_name].size())), true);
}

TEINAPI std::string load_string_resource (std::string file_name)
{
    std::string abs_file_name(build_resource_string(file_name));
    if (DoesFileExist(abs_file_name)) return ReadEntireFile(abs_file_name);
    else return std::string(gpak_resource_lookup[file_name].begin(),
        gpak_resource_lookup[file_name].end());
}

/* -------------------------------------------------------------------------- */

TEINAPI bool load_editor_resources ()
{
    if (!load_texture_resource("textures/editor_ui/tools.png", resource_icons))
    {
        LogError(ERR_MAX, "Failed to load editor icons!");
        return false;
    }
    if (!load_texture_resource("textures/editor_ui/checker_x14.png", resource_checker_14, TEXTURE_WRAP_REPEAT))
    {
        LogError(ERR_MAX, "Failed to load the checker-x14 image!");
        return false;
    }
    if (!load_texture_resource("textures/editor_ui/checker_x16.png", resource_checker_16, TEXTURE_WRAP_REPEAT))
    {
        LogError(ERR_MAX, "Failed to load the checker-x16 image!");
        return false;
    }
    if (!load_texture_resource("textures/editor_ui/checker_x20.png", resource_checker_20, TEXTURE_WRAP_REPEAT))
    {
        LogError(ERR_MAX, "Failed to load the checker-x20 image!");
        return false;
    }
    if (!load_font_resource("fonts/opensans-regular.ttf", resource_font_regular_sans, { SMALL_FONT_POINT_SIZE, LARGE_FONT_POINT_SIZE }))
    {
        LogError(ERR_MAX, "Failed to load OpenSans regular font!");
        return false;
    }
    if (!load_font_resource("fonts/opensans-bold.ttf", resource_font_bold_sans, { SMALL_FONT_POINT_SIZE, LARGE_FONT_POINT_SIZE }))
    {
        LogError(ERR_MAX, "Failed to load OpenSans bold font!");
        return false;
    }
    if (!load_font_resource("fonts/liberationmono-regular.ttf", resource_font_regular_libmono))
    {
        LogError(ERR_MAX, "Failed to load LiberationMono regular font!");
        return false;
    }
    if (!load_font_resource("fonts/opendyslexic-regular.ttf", resource_font_regular_dyslexic, { SMALL_FONT_POINT_SIZE, LARGE_FONT_POINT_SIZE }))
    {
        LogError(ERR_MAX, "Failed to load OpenDyslexic regular font!");
        return false;
    }
    if (!load_font_resource("fonts/opendyslexic-bold.ttf", resource_font_bold_dyslexic, { SMALL_FONT_POINT_SIZE, LARGE_FONT_POINT_SIZE }))
    {
        LogError(ERR_MAX, "Failed to load OpenDyslexic bold font!");
        return false;
    }
    if (!load_font_resource("fonts/opendyslexic-mono.ttf", resource_font_mono_dyslexic))
    {
        LogError(ERR_MAX, "Failed to load OpenDyslexic mono font!");
        return false;
    }

    update_editor_font();

    LogDebug("Loaded Editor Resources");
    return true;
}

TEINAPI void free_editor_resources ()
{
    free_font         (resource_font_mono_dyslexic);
    free_font         (resource_font_bold_sans);
    free_font         (resource_font_bold_dyslexic);
    free_font         (resource_font_regular_libmono);
    free_font         (resource_font_regular_sans);
    free_font         (resource_font_regular_dyslexic);
    FreeTexture       (resource_icons);
    FreeTexture       (resource_checker_14);
    FreeTexture       (resource_checker_16);
    FreeTexture       (resource_checker_20);
    free_texture_atlas(resource_large);
    free_texture_atlas(resource_small);
}

/* -------------------------------------------------------------------------- */

TEINAPI std::string build_resource_string (std::string str)
{
    return (IsPathAbsolute(str)) ? str : (resource_location + str);
}

/* -------------------------------------------------------------------------- */

TEINAPI void update_editor_font ()
{
    current_editor_font = gEditorSettings.fontFace;
}

TEINAPI bool is_editor_font_opensans ()
{
    return (current_editor_font != "OpenDyslexic");
}

/* -------------------------------------------------------------------------- */

TEINAPI Font& get_editor_regular_font ()
{
    return (is_editor_font_opensans()) ? resource_font_regular_sans : resource_font_regular_dyslexic;
}

TEINAPI Font& get_editor_bold_font ()
{
    return (is_editor_font_opensans()) ? resource_font_bold_sans : resource_font_bold_dyslexic;
}

/* -------------------------------------------------------------------------- */

TEINAPI Texture_Atlas& get_editor_atlas_large ()
{
    return resource_large;
}

TEINAPI Texture_Atlas& get_editor_atlas_small ()
{
    return resource_small;
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
