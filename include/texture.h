#ifndef _TEXTURE_H
#define _TEXTURE_H

#include <vector>

#include "common.h"

struct TextureData
{
    u32 width, height, num_channels;
    std::vector<u8> data;

    TextureData(u32 width, u32 height, u32 num_channels, u8 *data);
};

#endif
