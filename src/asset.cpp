#include <algorithm>
#include <optional>
#include <filesystem>
#include <string_view>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"

#include "asset.h"
#include "common.h"

namespace fs = std::filesystem;

void AssetManager::addSearchDirectory(std::string dir)
{
    // TODO: verify directory exists
    this->search_dirs.push_back(dir);
}

std::vector<std::string> AssetManager::findMatchingFiles(
        std::string filename, 
        bool multiple
    )
{
    std::vector<std::string> matches;

    for (const auto &dir : this->search_dirs)
    {
        for (const auto &entry : fs::directory_iterator(dir))
        {
            if (entry.path().filename().string() == filename)
            {
                matches.push_back(entry.path());
            }
        }
    }

    if (matches.empty())
    {
        Log::warning("No match found for file %s\n", filename.c_str());
    }

    if (!multiple && matches.size() > 1)
    {
        Log::warning(
                "Multiple matches found for file %s\n", filename.c_str());
    }

    return matches;
}

std::optional<TextureData> AssetManager::getTexture(const std::string &filename)
{
    Log::verbose("Getting Texture...");
    std::vector<std::string> matches = this->findMatchingFiles(filename);

    if (matches.size() == 0)
    {
        Log::verbose("\tNo matches found.");
        return std::nullopt;
    }

    s32 width, height, num_channels;

    Log::verbose("\tLoading texture from disk.");
    // TODO determine number of desired channels
    u8 *texture_data = stbi_load(
            matches[0].c_str(), 
            &width, 
            &height, 
            &num_channels, 
            STBI_rgb
        );

    Log::verbose("\twidth: %d", width);
    Log::verbose("\theight: %d", height);
    Log::verbose("\tnum_channels: %d", num_channels);

    // Convert from (RGB/RGBA) to (BGR/BGRA)
    if (num_channels == 3 || num_channels == 4)
    {
        for (int i = 0; i < width*height; i++)
        {
            std::reverse(
                    &texture_data[i * num_channels], 
                    &texture_data[i * num_channels + 3]
                );
        }
    }

    // Convert to BGRA if BGR
    if (num_channels == 3)
    {
        u8 *image_buffer = (u8 *) malloc(width * height * 4);
        for(int i = 0; i < width*height; i++)
        {
            memcpy(&image_buffer[i * 4], 
                   &texture_data[i * 3], 
                   num_channels);
            image_buffer[i * 4 + 3] = 255; 
        }
        num_channels = 4;
        free(texture_data);
        texture_data = image_buffer;
    }

    // Convert to BGRA if single channel
    if (num_channels == 1)
    {
        u8 *image_buffer = (u8 *) malloc(width * height * 4);
        for(int i = 0; i < width*height; i++)
        {
            memset(image_buffer + (i * 4), texture_data[i], 4);
            image_buffer[i * 4 + 3] = 255;
        }
        num_channels = 4;
        free(texture_data);
        texture_data = image_buffer;
    }

    if (texture_data == nullptr)
    {
        Log::error(
                "Failed to load texture from file %s\n", filename.c_str());
        return std::nullopt;
    }

    if (num_channels != STBI_rgb_alpha)
        Log::warning("Number of channels (%d) does not match expected: %d", 
                num_channels, STBI_rgb_alpha);

    TextureData td = TextureData(width, height, num_channels, texture_data);

    free(texture_data);

    return td;
}

std::optional<std::string> AssetManager::getTextFile(const std::string &path)
{
    // TODO: Do actual smart asset management stuff here
    std::ifstream file_stream(path);
    std::stringstream text_stream;

    text_stream << file_stream.rdbuf();

    return text_stream.str();
}
