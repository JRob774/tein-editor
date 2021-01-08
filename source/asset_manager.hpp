// Base type for assets, all custom asset types should inherit from this!
struct AssetBase
{
             AssetBase () = default;
    virtual ~AssetBase () = default;
    virtual void Load (std::string fileName) = 0;
    virtual void Free () = 0;
};

Global struct AssetManager
{
    AssetManager ();
    std::map<std::string,AssetBase*> assets;
    std::filesystem::path assetBasePath;

} gAssetManager;

EditorAPI void FreeAssets ();

template<typename T>
EditorAPI auto* GetAsset (std::string name)
{
    // @Improve: We can probably clean up this path stuff quite a bit as its pretty messy...
    std::filesystem::path pathName = std::filesystem::canonical(std::filesystem::path(gAssetManager.assetBasePath / T::Path).make_preferred());
    std::filesystem::path fileName = std::filesystem::path(name + T::Ext).make_preferred();

    T* asset = dynamic_cast<T*>(gAssetManager.assets[fileName.string()]); // Will create the asset if it doesn't exist.
    if (!asset) {
        // Allocate and load the asset if it hasn't been loaded.
        asset = new T; // @Improve: Change to std::nothrow and handle NULL case?
        asset->Load((pathName / fileName).string());
        gAssetManager.assets[fileName.string()] = asset;
    }
    if (name.empty()) {
        return static_cast<decltype(asset->data)*>(NULL);
    }
    return &asset->data;
}

// Asset Types

struct AssetData: public AssetBase
{
    static inline const std::filesystem::path Path = "data";
    static inline const std::string Ext = ".gon";
    void Load (std::string fileName) override;
    void Free () override;
    GonObject data;
};

struct AssetShader: public AssetBase
{
    static inline const std::filesystem::path Path = "shaders";
    static inline const std::string Ext = ".shader";
    void Load (std::string fileName) override;
    void Free () override;
    Shader data;
};

struct AssetTexture: public AssetBase
{
    static inline const std::filesystem::path Path = "textures";
    static inline const std::string Ext = ".png";
    void Load (std::string fileName) override;
    void Free () override;
    Texture data;
};
