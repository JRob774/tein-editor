namespace Internal
{
    TEINAPI bool CreateTextureAtlas (TextureAtlas& atlas, GonObject gon)
    {
        std::string textureFile(gon["texture"].String());
        if (!LoadTextureResource(textureFile, atlas.texture))
        {
            LogError(ERR_MIN, "Failed to load texture atlas image!");
            return false;
        }

        for (const auto& obj: gon["clips"].children_array)
        {
            S32 id = std::stoi(obj["id"].String());
            auto& clipData = obj["clip"].children_array;
            Quad clip
            {
            static_cast<float>(clipData[0].Number()),
            static_cast<float>(clipData[1].Number()),
            static_cast<float>(clipData[2].Number()),
            static_cast<float>(clipData[3].Number())
            };
            atlas.clips.insert({ id, clip });
        }

        return true;
    }
}

TEINAPI bool LoadTextureAtlasFromData (TextureAtlas& atlas, const std::vector<U8>& fileData)
{
    std::string buffer(fileData.begin(), fileData.end());
    return Internal::CreateTextureAtlas(atlas, GonObject::LoadFromBuffer(buffer));
}
TEINAPI bool LoadTextureAtlasFromFile (TextureAtlas& atlas, std::string fileName)
{
    fileName = MakePathAbsolute(fileName);
    return Internal::CreateTextureAtlas(atlas, GonObject::Load(fileName));
}

TEINAPI void FreeTextureAtlas (TextureAtlas& atlas)
{
    FreeTexture(atlas.texture);
    atlas.clips.clear();
}

TEINAPI Quad& GetAtlasClip (TextureAtlas& atlas, S32 key)
{
    return atlas.clips[key];
}
