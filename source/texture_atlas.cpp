/*******************************************************************************
 * Texture atlas system for handling multiple images within a single texture.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

TEINAPI bool internal__create_texture_atlas (Texture_Atlas& atlas, GonObject gon)
{
    std::string texture_file(gon["texture"].String());
    if (!load_texture_resource(texture_file, atlas.texture))
    {
        LOG_ERROR(ERR_MIN, "Failed to load texture atlas image!");
        return false;
    }

    for (const auto& obj: gon["clips"].children_array)
    {
        s32   id        = std::stoi(obj["id"].String());
        auto& clip_data = obj["clip"].children_array;
        quad clip
        {
        static_cast<float>(clip_data[0].Number()),
        static_cast<float>(clip_data[1].Number()),
        static_cast<float>(clip_data[2].Number()),
        static_cast<float>(clip_data[3].Number())
        };
        atlas.clips.insert(std::pair<s32, quad>(id, clip));
    }

    return true;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool load_texture_atlas_from_file (Texture_Atlas& atlas, std::string file_name)
{
    file_name = make_path_absolute(file_name);
    return internal__create_texture_atlas(atlas, GonObject::Load(file_name));
}
TEINAPI bool load_texture_atlas_from_data (Texture_Atlas& atlas, const std::vector<u8>& file_data)
{
    std::string buffer(file_data.begin(), file_data.end());
    return internal__create_texture_atlas(atlas, GonObject::LoadFromBuffer(buffer));
}

/* -------------------------------------------------------------------------- */

TEINAPI void free_texture_atlas (Texture_Atlas& atlas)
{
    free_texture(atlas.texture);
    atlas.clips.clear();
}

/* -------------------------------------------------------------------------- */

TEINAPI quad& get_atlas_clip (Texture_Atlas& atlas, s32 key)
{
    return atlas.clips[key];
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
