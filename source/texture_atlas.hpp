/*******************************************************************************
 * Texture atlas system for handling multiple images within a single texture.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr S32 ALT_OFFSET = 60000;

struct Texture_Atlas
{
    std::map<S32, Quad> clips;
    Texture texture;
};

/* -------------------------------------------------------------------------- */

TEINAPI bool load_texture_atlas_from_file (Texture_Atlas& atlas, std::string            file_name);
TEINAPI bool load_texture_atlas_from_data (Texture_Atlas& atlas, const std::vector<U8>& file_data);
TEINAPI void free_texture_atlas           (Texture_Atlas& atlas);

TEINAPI Quad& get_atlas_clip (Texture_Atlas& atlas, S32 key);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
