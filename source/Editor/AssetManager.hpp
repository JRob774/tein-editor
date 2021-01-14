#pragma once

#include "Renderer.hpp"

#include <filesystem>
#include <string>
#include <map>

#include <iostream>

#include <gon/gon.h>

namespace TEIN
{
    class Asset // Base type for assets, all custom asset types should inherit from this!
    {
    public:
        virtual ~Asset    () = default;
        virtual void Load (std::string fileName) = 0;
        virtual void Free () = 0;
    };

    class AssetData: public Asset
    {
    public:
        static inline const std::filesystem::path Path = "data";
        static inline const std::string Ext = ".gon";
        void Load (std::string fileName) override;
        void Free () override;
        GonObject data;
    };
    class AssetShader: public Asset
    {
    public:
        static inline const std::filesystem::path Path = "shaders";
        static inline const std::string Ext = ".shader";
        void Load (std::string fileName) override;
        void Free () override;
        Shader data;
    };
    class AssetTexture: public Asset
    {
    public:
        static inline const std::filesystem::path Path = "textures";
        static inline const std::string Ext = ".png";
        void Load (std::string fileName) override;
        void Free () override;
        Texture data;
    };

    namespace AssetManager
    {
        extern std::map<std::string,Asset*> g_Assets;
        extern std::filesystem::path g_AssetBasePath;

        bool Init ();
        void Quit ();

        template<typename T>
        auto* Get (std::string name)
        {
            // @Improve: We can probably clean up this path stuff quite a bit as its pretty messy...
            std::filesystem::path pathName = std::filesystem::path(g_AssetBasePath / T::Path).make_preferred();
            std::filesystem::path fileName = std::filesystem::path(name + T::Ext).make_preferred();

            T* asset = dynamic_cast<T*>(g_Assets[fileName.string()]); // Will create the asset if it doesn't exist.
            if (!asset) {
                // Allocate and load the asset if it hasn't been loaded.
                asset = new T; // @Improve: Change to std::nothrow and handle NULL case?
                asset->Load((pathName / fileName).string());
                g_Assets[fileName.string()] = asset;
            }
            if (name.empty()) {
                return static_cast<decltype(asset->data)*>(NULL);
            }
            return &asset->data;
        }
    }
}
