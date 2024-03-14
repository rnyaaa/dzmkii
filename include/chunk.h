#include <vector>

#include "PerlinNoise.hpp"
#include "tile.h"
#include "common.h"

#ifndef _CHUNK_H
#define _CHUNK_H

struct Chunk
{
    static const u32 TILES_PER_SIDE = 64;

    std::vector<Tile> tiles;
    std::vector<u8> material_indices;

    void setMatIndexByPos(glm::vec3 pos, u8 mat);

    Chunk(glm::vec2 chunk_start);
};

#endif
