#ifndef _ASSET_H
#define _ASSET_H

#include <string>
#include <optional>
#include <filesystem>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include <chrono>

#include "texture.h"
namespace fs = std::filesystem;

template <typename T>
struct AssetHandle
{
    u32 identifier;
    u32 generation;
};

template <typename T>
struct FreeList
{
public:
    AssetHandle<T> insert(T a)
    {
        if (!free.empty())
        {
            u32 id = free.back();
            free.pop_back();
            generation[id] += 1;
            elements[id] = a;

            return {id, generation[id]};
        }

        u32 id = elements.size();
        elements.push_back(a);
        generation.push_back(0);

        return {id, 0};
    }

    std::optional<T> get(AssetHandle<T> handle)
    {
        if (handle.identifier < generation.size() && handle.generation == generation[handle.identifier])
        {
            return elements[handle.identifier];
        }
        return std::nullopt;
    }

private:
    std::vector<u32>  free;
    std::vector<T>    elements;
    std::vector<u32>  generation;
};

struct AssetManager
{
    std::vector<std::string> search_dirs;

    // TODO(jklmn): Will this possibly cause to load the same asset twice?
    std::map<fs::path, std::chrono::time_point<std::chrono::system_clock>> loaded;

    FreeList<TextureData>   textures;
    FreeList<std::string>   text_data;

    void addSearchDirectory(const fs::path &dir, bool recursive=false);

    std::vector<std::string> findMatchingFiles(
            std::string filename, 
            bool multiple = false
        );

#define GET(A, F)\
        std::optional<A> F(AssetHandle<A>)

    GET(TextureData, getTextureData);
    GET(std::string, getText);

#undef GET

#define LOAD(A, F)\
    std::optional<AssetHandle<A>> F(const fs::path &path);

    LOAD(TextureData, loadTexture);
    LOAD(std::string, loadText);

#undef LOAD
};

#endif // _ASSET_H
