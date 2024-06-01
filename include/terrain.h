#include <vector>
#include <map>
#include <PerlinNoise.hpp>

#include "common.h"
#include "vertex.h"
#include "transform.h"
#include "renderer.h"
#include "term_renderer.h"
#include "asset.h"

#pragma once

#ifndef _TERRAIN_H
#define _TERRAIN_H

#define TILES_PER_SIDE  (64)
#define TILES_PER_CHUNK (TILES_PER_SIDE * TILES_PER_SIDE)

#define LAYER_SIZE (150.0)

struct Tile
{
    // North, East, South, West
    Vertex vertices[4];
};

struct ChunkData
{
    glm::mat4 model_matrix;
    s32 chunk_index;
};

struct MegaChunkData
{
    u8 material_indices[TILES_PER_SIDE * TILES_PER_SIDE * 9];
    u8 los_indices[TILES_PER_SIDE * TILES_PER_SIDE * 9];
};

struct Chunk
{
    Transform transform;

    u8 material_indices[TILES_PER_SIDE * TILES_PER_SIDE];
    u8 los_indices[TILES_PER_SIDE * TILES_PER_SIDE];
    u8 navigable[TILES_PER_SIDE * TILES_PER_SIDE];
    
    MeshData mesh_data;

    bool mesh_registered;
    DZMesh mesh;
    DZBuffer local_uniforms_buffer;

    Chunk(v2f chunk_start, u32 seed, f32 chunk_size);

    void updateUniforms(DZRenderer &renderer, s32 chunk_index);

    v2f  getPosFromTileIndex(u32 tile_index, f32 tile_width);
};


struct Terrain
{
    const f32 chunk_size;
    const u32 seed;
    DZBuffer terrain_uniform_buffer;
    DZPipeline terrain_pipeline;

    std::map<v2f, Chunk> chunks;
    std::array<Chunk*, 9> visible;

    Terrain(DZRenderer &renderer, f32 chunk_size, u32 seed);

    void seedNoise(u32 seed);

    void createChunk(DZRenderer &renderer, glm::vec2 pos_in_chunk);
    void termRender(DZTermRenderer &term, glm::vec2 pos);
    void updateLOS(glm::vec2 pos, int LOS);

    // TODO(ronja): bad form to name a method getX() if it does not return anything
    void    getVisible(Camera &camera);
    v2f     getChunkOriginFromPos(v2f pos);
    Chunk*  getChunkFromPos(v2f pos);
    int     getTileIndexFromPos(v2f pos);

    void updateUniforms(DZRenderer &renderer, std::array<Chunk*, 9> visible) const;

};



#endif // _TERRAIN_H

