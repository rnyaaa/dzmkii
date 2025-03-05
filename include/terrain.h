#include <vector>
#include <map>
#include <PerlinNoise.hpp>
#include "common.h"
#include "vertex.h"
#include "transform.h"
#include "renderer.h"
#include "term_renderer.h"

#pragma once
#ifndef _TERRAIN_H
#define _TERRAIN_H
#define TILES_PER_SIDE  (64)
#define TILES_PER_CHUNK (TILES_PER_SIDE * TILES_PER_SIDE)
#define SKYSCRAPERS_PER_CUNK (15)
struct Skyscraper
{
    Transform transform;
    u8 texture;
    AArect2f collision_bound;

    static bool mesh_generated;
    static MeshData mesh_data;
    static DZMesh mesh;
    static bool mesh_registered;

    bool has_uniforms_buffer = false;
    DZBuffer local_uniforms_buffer;

    bool foo = false;

    Skyscraper();
    Skyscraper(glm::vec3 pos);
    void collision(glm::vec3 &velocity, bool &on_roof, glm::vec3 pos);
    void updateUniforms(DZRenderer &renderer);
};

struct SkyscraperData
{
    glm::mat4 model_matrix;
    u32 foo;
    u8 texture;
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
    u8 material_indices[TILES_PER_SIDE * TILES_PER_SIDE * 25];
};

struct Chunk
{
    Transform transform;

    u8 material_indices[TILES_PER_SIDE * TILES_PER_SIDE];

    std::array<Skyscraper*, SKYSCRAPERS_PER_CUNK> skyscrapers;

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

    std::map<v2f, Chunk> chunks;
    std::array<Chunk*, 25> visible;

    Terrain(DZRenderer &renderer, f32 chunk_size, u32 seed);

    void seedNoise(u32 seed);

    void createChunk(DZRenderer &renderer, glm::vec2 pos_in_chunk);
    void getVisible(Camera &camera);
    f32 getHeight(glm::vec2 pos);

    v2f     getChunkOriginFromPos(v2f pos);
    Chunk*  getChunkFromPos(v2f pos);
    int     getTileIndexFromPos(v2f pos);

    void updateUniforms(DZRenderer &renderer, std::array<Chunk*, 25> visible) const;

};
#endif // _TERRAIN_H
