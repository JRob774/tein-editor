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
    std::string assetBasePath;

} gAssetManager;

EditorAPI std::string BuildAssetPath (std::string pathName);

EditorAPI void FreeAssets ();

template<typename T>
EditorAPI auto* GetAsset (std::string name)
{
    std::string fileName = name + T::Ext;
    std::string fullFileName = BuildAssetPath(T::Path + fileName);
    T* asset = dynamic_cast<T*>(gAssetManager.assets[fileName]); // Will create the asset if it doesn't exist.
    if (!asset) {
        // Allocate and load the asset if it hasn't been loaded.
        asset = new T; // @Improve: Change to std::nothrow and handle NULL case?
        asset->Load(fullFileName);
        gAssetManager.assets[fileName] = asset;
    }
    if (name.empty()) {
        return static_cast<decltype(asset->data)*>(NULL);
    }
    return &asset->data;
}

// Asset Types

struct AssetShader: public AssetBase
{
    static inline const std::string Path = "shaders/";
    static inline const std::string Ext = ".shader";
    void Load (std::string fileName) override;
    void Free () override;
    Shader data;
};

struct AssetTexture: public AssetBase
{
    static inline const std::string Path = "textures/";
    static inline const std::string Ext = ".png";
    void Load (std::string fileName) override;
    void Free () override;
    Texture data;
};

struct AssetData: public AssetBase
{
    static inline const std::string Path = "data/";
    static inline const std::string Ext = ".data";
    void Load (std::string fileName) override;
    void Free () override;
    GonObject data;
};
