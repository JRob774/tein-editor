#pragma once

// @Improve: This is kind of weird and should probably be changed?
// The offset in the atlas for the large tile clips.
static constexpr S32 gAltOffset = 60000;

struct TextureAtlas
{
    std::map<S32,Quad> clips;
    Texture texture;
};

TEINAPI bool LoadTextureAtlasFromData (TextureAtlas& atlas, const std::vector<U8>& fileData);
TEINAPI bool LoadTextureAtlasFromFile (TextureAtlas& atlas, std::string            fileName);
TEINAPI void FreeTextureAtlas         (TextureAtlas& atlas);

TEINAPI Quad& GetAtlasClip (TextureAtlas& atlas, S32 key);
