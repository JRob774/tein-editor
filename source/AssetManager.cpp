AssetManager::AssetManager ()
{
    // @Incomplete: Sort out this path stuff...

    assetBasePath = GetExecutablePath();
    // assetBasePath += "/";

    // If there is a file in the current working directory specifying the assets relative path
    // then we append that to the executable path, otherwise we just expect the exe directory.
    std::ifstream file("asset_path.txt", std::ios::in);
    if (file.is_open()) {
        std::string assetPath;
        std::getline(file, assetPath);
        assetBasePath += assetPath;
    } else {
        assetBasePath += "assets/";
    }

    // Make sure the asset path always ends in a slash.
    if (assetBasePath.back() != '\\' && assetBasePath.back() != '/') {
        assetBasePath += "/";
    }
}

EditorAPI std::string BuildAssetPath (std::string pathName)
{
    return gAssetManager.assetBasePath + pathName;
}

EditorAPI void FreeAssets ()
{
    for (auto& [name,asset]: gAssetManager.assets) {
        asset->Free();
        delete asset;
    }
    gAssetManager.assets.clear();
}

// Asset Types

void AssetShader::Load (std::string fileName)
{
    LoadShader(data, fileName);
}
void AssetShader::Free ()
{
    FreeShader(data);
}

void AssetTexture::Load (std::string fileName)
{
    LoadTexture(data, fileName);
}
void AssetTexture::Free ()
{
    FreeTexture(data);
}

void AssetData::Load (std::string fileName)
{
    data = GonObject::Load(fileName);
}
void AssetData::Free ()
{
    // Nothing...
}
