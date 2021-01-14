#include "AssetManager.hpp"

#include "Utility.hpp"

#include <fstream>

namespace TEIN
{
    void AssetData::Load (std::string fileName)
    {
        data = GonObject::Load(fileName);
    }
    void AssetData::Free ()
    {
        // Nothing...
    }

    void AssetShader::Load (std::string fileName)
    {
        data.Load(fileName);
    }
    void AssetShader::Free ()
    {
        data.Free();
    }

    void AssetTexture::Load (std::string fileName)
    {
        data.Load(fileName);
    }
    void AssetTexture::Free ()
    {
        data.Free();
    }

    namespace AssetManager
    {
        std::map<std::string,Asset*> g_Assets;
        std::filesystem::path g_AssetBasePath;

        bool Init ()
        {
            g_AssetBasePath = Utility::GetExecutablePath();

            // If there is a file in the current working directory specifying the assets relative path
            // then we append that to the executable path, otherwise we just expect the exe directory.
            std::ifstream file(g_AssetBasePath / "asset_path.txt", std::ios::in);
            if (file.is_open()) {
                std::string assetPath;
                std::getline(file, assetPath);
                g_AssetBasePath.append(assetPath);
            } else {
                g_AssetBasePath.append("assets");
            }
            return true;
        }
        void Quit ()
        {
            for (auto& [name,asset]: g_Assets) {
                if (asset) {
                    asset->Free();
                }
                delete asset;
            }
            g_Assets.clear();
        }
    }
}
