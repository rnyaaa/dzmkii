#include "texture.h"

TextureData::TextureData(u32 width, u32 height, u32 num_channels, std::shared_ptr<u8> data)
    : width(width), 
      height(height), 
      num_channels(num_channels),
      data(data)
{ }
