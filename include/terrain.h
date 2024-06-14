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

#define VORONOI_BIOMES (500)

#define NUM_TEXTURES_PER_BIOME 7

#define BIOME_DEFAULT    (0)
#define BIOME_RAINFOREST (1)
#define BIOME_COLDLANDS  (2)
#define BIOME_SANDLANDS  (3)
#define BIOME_GRAVELANDS (4)
#define BIOME_MEATLANDS  (5)
#define BIOME_BADLANDS   (6)

#define NUM_BIOMES (7)

#define START_AREA (80)
#define WORLDSIZE (2000)

struct BiomePoint
{
    v2f position;
    u8 biome;
};

struct BNode
{
    BNode *parent;
    BNode *left_child;
    BNode *right_child;
    BiomePoint p;

    float x_bound_left, x_bound_right, y_bound_bottom, y_bound_top;

    void add_recur_x(const BiomePoint &bp);
    void add_recur_y(const BiomePoint &bp);
    BNode(BiomePoint bp)
        : p {bp}
        , left_child{nullptr}
        , right_child{nullptr}
        , parent{nullptr}
    { }
};

struct KDTree 
{
    // TODO: Could use an allocator for the std::array 
    //       for faster memory lookup (contiguous memory)
    BNode *root;

    KDTree();
    void add(const std::array<BiomePoint, VORONOI_BIOMES> &bpoints);
    std::vector<BiomePoint> find_n_closest(v2f pos, s32 n);
};

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

    Chunk(v2f chunk_start, u32 seed, f32 chunk_size, const std::array<BiomePoint, VORONOI_BIOMES> &bps);//KDTree *kd);

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

    std::array<BiomePoint, VORONOI_BIOMES> bps;
    //KDTree kd;

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

