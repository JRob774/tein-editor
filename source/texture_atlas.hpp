/*******************************************************************************
 * Texture atlas system for handling multiple images within a single texture.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr s32 ALT_OFFSET = 60000;

struct Texture_Atlas
{
    std::map<s32, quad> clips;
    Texture texture;
};

/* -------------------------------------------------------------------------- */

FILDEF bool load_texture_atlas_from_file (Texture_Atlas& atlas, std::string            file_name);
FILDEF bool load_texture_atlas_from_data (Texture_Atlas& atlas, const std::vector<u8>& file_data);
FILDEF void free_texture_atlas           (Texture_Atlas& atlas);

FILDEF quad& get_atlas_clip (Texture_Atlas& atlas, s32 key);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
