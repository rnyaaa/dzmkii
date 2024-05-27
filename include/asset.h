#ifndef _ASSET_H
#define _ASSET_H

#include <string>
#include <optional>

#include "texture.h"

struct AssetManager
{
    std::vector<std::string> search_dirs;

    void addSearchDirectory(std::string dir);

    std::vector<std::string> findMatchingFiles(
            std::string filename, 
            bool multiple = false
        );

    std::optional<TextureData> getTexture(const std::string &path);
    std::optional<std::string> getTextFile(const std::string &path);

};

#endif // _ASSET_H
