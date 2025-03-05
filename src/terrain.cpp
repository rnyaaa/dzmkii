#include "terrain.h"
#include "input.h"
#include "logger.h"
#include "renderer.h"
#include "geometry.h"
#include "asset.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>

#include <queue>
#include <time.h>    
#define CHUNKS_PER_SIDE 10 

bool Skyscraper::mesh_generated = false;
MeshData Skyscraper::mesh_data;
DZMesh Skyscraper::mesh = DZInvalid;
bool Skyscraper::mesh_registered = false;

Skyscraper::Skyscraper(glm::vec3 pos)
{
    this->transform.pos = pos;

    int base_scale = 6 + rand()%6;
    int height_scale = 60 + rand()%60;
    this->transform.scale = glm::vec3(base_scale, base_scale, height_scale);
    this->transform.rotation = glm::vec3(0.f, 0.f, 0.f);
    if (!Skyscraper::mesh_generated)
    {
        this->mesh_data = MeshData::UnitCube();
    }

    AArect2f bounding_rect {
        v2f{
            pos.x - base_scale / 2.f,
            pos.y - base_scale / 2.f,
        },
        v2f{
            (f32) base_scale,
            (f32) base_scale
        }
    };

    this->collision_bound = bounding_rect;

    this->texture = (u8) rand() % 5;
}

void Skyscraper::collision(glm::vec3 &velocity, bool &on_roof, glm::vec3 inpos)
{
    circ2f collide_bound{v2f{inpos.x, inpos.y}, 1.0f};
    const f32 roof = this->transform.pos.z + this->transform.scale.z/2.f;
    if (collide_bound.collidesWith(this->collision_bound))
    {
        // const v2f skscr_sw = this->collision_bound.pos;
        // const v2f skscr_nw = skscr_sw + this->collision_bound.rotatePoint({0.f, this->collision_bound.dim.y}, this->transform.rotation.z);
        // const v2f skscr_ne = skscr_sw + this->collision_bound.rotatePoint(this->collision_bound.dim, this->transform.rotation.z);
        // const v2f skscr_se = skscr_sw + this->collision_bound.rotatePoint({this->collision_bound.dim.x, 0.f}, this->transform.rotation.z);

        std::array<v2f, 4> vertices = this->collision_bound.getVertices();

        const LineSegment2D<f32> segments[4] = {
            {vertices[0], vertices[1]},
            {vertices[1], vertices[2]},
            {vertices[2], vertices[3]},
            {vertices[3], vertices[0]},
        };

        //Log::verbose("SKYSCRAPER: POS: (%f, %f) - DIM: (%f)", this->transform.pos.x, this->transform.pos.y, this->transform.scale.x);
        //Log::verbose("SKYSCRAPER SEGMENTS: ");
        //Log::verbose("\t (%f, %f) -> (%f, %f)", vertices[0].x, vertices[0].y, vertices[1].x, vertices[1].y);
        //Log::verbose("\t (%f, %f) -> (%f, %f)", vertices[1].x, vertices[1].y, vertices[2].x, vertices[2].y);
        //Log::verbose("\t (%f, %f) -> (%f, %f)", vertices[2].x, vertices[2].y, vertices[3].x, vertices[3].y);
        //Log::verbose("\t (%f, %f) -> (%f, %f)", vertices[3].x, vertices[3].y, vertices[0].x, vertices[0].y);

        if((inpos.z + 2) < roof)
        {
            const f32 cols[4] = {
                segments[0].collidesWith(collide_bound),
                segments[1].collidesWith(collide_bound),
                segments[2].collidesWith(collide_bound),
                segments[3].collidesWith(collide_bound)
            };

            size_t max_col = std::max_element(cols, cols + 4) - cols;

            if (max_col > 3) return;

            v2f skscr_vec = segments[max_col].p1 - segments[max_col].p2;
            v2f normal = v2f{-skscr_vec.y, skscr_vec.x}.normalized();
            v2f normvel = v2f{velocity.x, velocity.y}.normalized();
            f32 dot = std::max(normal.dot(normvel), 0.f);
            v2f vel = (normvel - (normal * dot)) * glm::length(velocity.xy());

            velocity.x = vel.x;
            velocity.y = vel.y;
        } 
        else 
        {
            if(velocity.z < 0.f && inpos.z <= roof + 2.5)
            {
                on_roof = true;
                velocity.z = 0.f;
            }
        }
    }
}

void Skyscraper::updateUniforms(DZRenderer &renderer)
{
    if (!Skyscraper::mesh_registered)
    {
        Log::verbose("\tRegistering mesh with renderer...");
        Skyscraper::mesh = renderer.createMesh(this->mesh_data);
        Skyscraper::mesh_registered = true;
        Log::verbose("\tMesh registered...");
        Skyscraper::mesh_registered = true;
    }

    if (!this->has_uniforms_buffer)
    {
        this->local_uniforms_buffer = 
            renderer.createBufferOfSize(sizeof(SkyscraperData), StorageMode::MANAGED);
        this->has_uniforms_buffer = true;
    }

    SkyscraperData skyscraper_data;

    skyscraper_data.foo = this->foo ? 1 : 0;
    skyscraper_data.texture = texture;
    skyscraper_data.model_matrix = this->transform.asMat4();

    this->foo = false;

    renderer.setBufferOfSize(
            local_uniforms_buffer, 
            &skyscraper_data, 
            sizeof(SkyscraperData)
        );
}

MeshData genMeshFromTiles(std::vector<Tile> tiles)
{
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
 
    for (auto tile : tiles)
    {
        indices.push_back(vertices.size());
        indices.push_back(vertices.size() + 1);
        indices.push_back(vertices.size() + 2);
        indices.push_back(vertices.size());
        indices.push_back(vertices.size() + 2);
        indices.push_back(vertices.size() + 3);

        vertices.push_back(tile.vertices[0]);
        vertices.push_back(tile.vertices[1]);
        vertices.push_back(tile.vertices[2]);
        vertices.push_back(tile.vertices[3]);
    }

    return MeshData { 
        vertices,
        indices,
        PrimitiveType::TRIANGLE
    };
}

Chunk::Chunk(
        v2f chunk_start, 
        u32 seed, 
        f32 chunk_size
    )
    : mesh_registered(false)
{
    Log::verbose("Setting up chunk...");

    const f32 tile_width = chunk_size / TILES_PER_SIDE;
    const f32 startx = (f32) chunk_start.x;
    const f32 starty = (f32) chunk_start.y;

    transform.pos = glm::vec3(chunk_start.x, chunk_start.y, 0.0);
    transform.scale = glm::vec3(1.0f);
    transform.rotation = glm::vec3(0.0);

    f32 perlin_scale = 0.005f;
    f32 noise_scale = 64.0f;
    u32 octaves = 9;

    const siv::PerlinNoise perlin(seed);

    std::vector<Tile> tiles(TILES_PER_SIDE * TILES_PER_SIDE);

    //glm::vec3 normals[(TILES_PER_SIDE + 1) * (TILES_PER_SIDE + 1)];

    //memset(normals, 0, sizeof(glm::vec3) * (TILES_PER_SIDE + 1) * (TILES_PER_SIDE + 1));

    Log::verbose("\tChunk construction started...");

    f32 height[TILES_PER_SIDE + 3][TILES_PER_SIDE + 3];
    glm::vec3 normal[TILES_PER_SIDE + 1][TILES_PER_SIDE + 1];
    glm::vec3 tangent[TILES_PER_SIDE + 1][TILES_PER_SIDE + 1];
    glm::vec3 bitangent[TILES_PER_SIDE + 1][TILES_PER_SIDE + 1];

    for (u32 i = 0; i < TILES_PER_SIDE + 3; i++)
    {
        for (u32 j = 0; j < TILES_PER_SIDE + 3; j++)
        {
            const f32 x_pos = i * tile_width;
            const f32 y_pos = j * tile_width;
            f32 noise = noise_scale * std::powf(perlin.octave2D((startx + x_pos) * perlin_scale, (starty + y_pos) * perlin_scale, octaves), 2);

            height[i][j] = noise;
        }
    }

    for (u32 i = 0; i < TILES_PER_SIDE + 1; i++)
    {
        u32 x = i + 1;
        for (u32 j = 0; j < TILES_PER_SIDE + 1; j++)
        {
            u32 y = j + 1;

            f32 dx = height[x+1][y] - height[x-1][y];
            f32 dy = height[x][y+1] - height[x][y-1];

            normal[i][j] = glm::normalize(glm::vec3(-dx, -dy, 2));
            tangent[i][j] = glm::normalize(glm::vec3(1,0,dx));

            // Reorthoganalize
            tangent[i][j] -= glm::dot(tangent[i][j], normal[i][j]) * normal[i][j];
            bitangent[i][j] = glm::cross(normal[i][j], tangent[i][j]);
        }
    }

    for (u32 i = 0; i < TILES_PER_SIDE; i++)
    {
        u32 x = i + 1;
        for (u32 j = 0; j < TILES_PER_SIDE; j++)
        {
            u32 tile_index = i * TILES_PER_SIDE + j;
            u32 y = j + 1;

#define SET_VERTEX(A, X, Y) \
            tiles[tile_index].vertices[A] = {\
                    glm::vec3((X) * tile_width, (Y) * tile_width, height[X][Y]), \
                    glm::vec4(normal[X][Y], 0.0f), \
                    glm::vec4(1.0), \
                    glm::vec4(tangent[X][Y], 0.0f), \
                    glm::vec4(bitangent[X][Y], 0.0f)\
                };

            SET_VERTEX(0, i, j);
            SET_VERTEX(1, i, j+1);
            SET_VERTEX(2, i+1, j+1);
            SET_VERTEX(3, i+1, j);
        }
    }

    //for (u32 i = 0; i < TILES_PER_SIDE; i++)
    //{
    //    for (u32 j = 0; j < TILES_PER_SIDE; j++)
    //    {
    //        const f32 x_pos = i * tile_width;
    //        const f32 y_pos = j * tile_width;
    //        const f32 x_pos2 = (i + 1) * tile_width;
    //        const f32 y_pos2 = (j + 1) * tile_width;
    //                     
    //        glm::vec3 corners[4] = {
    //             glm::vec3(x_pos,  y_pos,  0.0f),
    //             glm::vec3(x_pos,  y_pos2, 0.0f),
    //             glm::vec3(x_pos2, y_pos2, 0.0f),
    //             glm::vec3(x_pos2, y_pos,  0.0f)
    //        };

    //        for (u32 i = 0; i < 4; i++)
    //        {
    //            f32 noise = noise_scale * std::powf(perlin.octave2D((startx + corners[i].x) * perlin_scale, (starty + corners[i].y) * perlin_scale, octaves), 2);
    //            corners[i].z = noise;
    //        }

    //        Tile tile;
    //            
    //        glm::vec3 normal1 = glm::normalize(glm::cross(corners[2] - corners[0], corners[1] - corners[0]));

    //        normals[i * TILES_PER_SIDE + j]     += normal1;
    //        normals[i * TILES_PER_SIDE + (j+1)] += normal1;
    //        normals[(i+1) * TILES_PER_SIDE + j] += normal1;

    //        glm::vec3 normal2 = glm::normalize(glm::cross(corners[3] - corners[0], corners[2] - corners[0]));

    //        normals[i * TILES_PER_SIDE + j]         += normal2;
    //        normals[(i+1) * TILES_PER_SIDE + j]     += normal2;
    //        normals[(i+1) * TILES_PER_SIDE + (j+1)] += normal2;

    //        // Tangent: Horizontal edge (from vertex 0 to vertex 1 or vertex 3)
    //        glm::vec3 tangent = glm::normalize(corners[1] - corners[0]);

    //        // Bitangent: Vertical edge (from vertex 0 to vertex 3)
    //        glm::vec3 bitangent = glm::normalize(corners[3] - corners[0]);

    //        // Assign the vertices with normals, tangents, and bitangents
    //        tile.vertices[0] = {corners[0], glm::vec4(normals[i * TILES_PER_SIDE + j], 0.0f), glm::vec4(1.0), glm::vec4(tangent, 0.0f), glm::vec4(bitangent, 0.0f)};
    //        tile.vertices[1] = {corners[1], glm::vec4(normals[i * TILES_PER_SIDE + (j + 1)], 0.0f), glm::vec4(1.0), glm::vec4(tangent, 0.0f), glm::vec4(bitangent, 0.0f)};
    //        tile.vertices[2] = {corners[2], glm::vec4(normals[(i + 1) * TILES_PER_SIDE + (j + 1)], 0.0f), glm::vec4(1.0), glm::vec4(tangent, 0.0f), glm::vec4(bitangent, 0.0f)};
    //        tile.vertices[3] = {corners[3], glm::vec4(normals[(i + 1) * TILES_PER_SIDE + j], 0.0f), glm::vec4(1.0), glm::vec4(tangent, 0.0f), glm::vec4(bitangent, 0.0f)};

    //        tiles.push_back(tile);
    //    }
    //}

    //Log::verbose("\tGenerating normals...");

    //for (u32 i = 0; i < TILES_PER_SIDE; i++)
    //{
    //    for (u32 j = 0; j < TILES_PER_SIDE; j++)
    //    {
    //         tiles[i * TILES_PER_SIDE + j].vertices[0].normal = glm::vec4(glm::normalize(normals[i     * TILES_PER_SIDE + j    ]), 0.0f);
    //         tiles[i * TILES_PER_SIDE + j].vertices[1].normal = glm::vec4(glm::normalize(normals[i     * TILES_PER_SIDE + (j+1)]), 0.0f);
    //         tiles[i * TILES_PER_SIDE + j].vertices[2].normal = glm::vec4(glm::normalize(normals[(i+1) * TILES_PER_SIDE + (j+1)]), 0.0f);
    //         tiles[i * TILES_PER_SIDE + j].vertices[3].normal = glm::vec4(glm::normalize(normals[(i+1) * TILES_PER_SIDE + j    ]), 0.0f);
    //    }
    //}

    Log::verbose("\tGenerating material indices...");

    // Texture
    for(u32 i = 0; i < TILES_PER_SIDE; i++)
    {
        for (u32 j = 0; j < TILES_PER_SIDE; j++)
        {
            Tile tile = tiles[i * TILES_PER_SIDE + j];
            for (u32 k = 0; k < 4; k++)
            {
                // TODO: Fix this one weird thing doctors (jklmn, ronja) hate
                f32 _perlin_scale = perlin_scale;
                f32 _noise_scale = noise_scale;
                u32 _octaves = 200;
                int otherseed = 1010620;
                const siv::PerlinNoise otherperlin(otherseed);
                u32 index = i * TILES_PER_SIDE + j;
                f32 height = (tiles[index].vertices[0].pos.z 
                           + tiles[index].vertices[1].pos.z 
                           + tiles[index].vertices[2].pos.z 
                           + tiles[index].vertices[3].pos.z) / 4;

                f32 noise = _noise_scale * otherperlin.octave2D((startx + tile.vertices[k].pos.x) * _perlin_scale, (starty + tile.vertices[k].pos.y) * perlin_scale, _octaves) + height/4;
                //Log::verbose("noise: %f", noise);
                if(noise <= -4.5)
                {
                    // TODO: make perlin, not seeded noise
                    u32 chance = rand() % 10;
                    if(chance >= 5)
                        this->material_indices[i * TILES_PER_SIDE + j] = 0;
                    else
                        this->material_indices[i * TILES_PER_SIDE + j] = 1;
                } 
                else if (noise <= -3.5) 
                {
                    this->material_indices[i * TILES_PER_SIDE + j] = 1;
                }
                else if (noise <= -2.0) 
                {
                    this->material_indices[i * TILES_PER_SIDE + j] = 2;
                }

                else if (noise <= 2.0) 
                {
                    this->material_indices[i * TILES_PER_SIDE + j] = 3;
                }

                else if (noise <= 4.0) 
                {
                    this->material_indices[i * TILES_PER_SIDE + j] = 4;
                }

                else if (noise <= 6.5) 
                {
                    this->material_indices[i * TILES_PER_SIDE + j] = 5;
                }

                else if (noise > 8.5) 
                {
                    u32 chance = rand() % 10;
                    if(chance >= 5)
                        this->material_indices[i * TILES_PER_SIDE + j] = 6;
                    else
                        this->material_indices[i * TILES_PER_SIDE + j] = 5;
                }
            }
        }
        for(int i = 0; i < SKYSCRAPERS_PER_CUNK; i++)
        {
            Skyscraper *skyscr = new Skyscraper(
                    glm::vec3(
                        chunk_start.x + rand()%TILES_PER_SIDE, 
                        chunk_start.y + rand()%TILES_PER_SIDE, 
                        0));
            this->skyscrapers[i] = skyscr;
        }
    }

    Log::verbose("\tGenerating mesh data...");

    MeshData mesh_data = genMeshFromTiles(tiles);
    this->mesh_data = mesh_data;
}

v2f Chunk::getPosFromTileIndex(u32 tile_index, f32 tile_width)
{
    u32 x = tile_index % TILES_PER_SIDE;
    u32 y = tile_index / TILES_PER_SIDE; 
    return v2f{transform.pos.x + x * tile_width, transform.pos.y + y * tile_width}; 
}

void Chunk::updateUniforms(DZRenderer &renderer, s32 chunk_index)
{
    if (!this->mesh_registered)
    {
        Log::verbose("\tRegistering mesh with renderer...");
        this->mesh = renderer.createMesh(this->mesh_data);
        this->local_uniforms_buffer = 
            renderer.createBufferOfSize(sizeof(ChunkData), StorageMode::MANAGED);
        this->mesh_registered = true;

        Log::verbose("\tMesh registered...");
    }

    ChunkData chunk_data;

    chunk_data.chunk_index = chunk_index;
    chunk_data.model_matrix = this->transform.asMat4();

    renderer.setBufferOfSize(
            local_uniforms_buffer, 
            &chunk_data, 
            sizeof(ChunkData)
        );
}

Terrain::Terrain(DZRenderer &renderer, f32 chunk_size, u32 seed)
    : chunk_size { chunk_size }
    , seed { seed }
{
    Log::verbose("Creating terrain..."); 
    srand(seed);
    this->terrain_uniform_buffer = renderer.createBufferOfSize(sizeof(MegaChunkData));
    Log::verbose("Terrain established"); 
}

v2f Terrain::getChunkOriginFromPos(v2f pos)
{
    int chunk_start_x, chunk_start_y;
    
    chunk_start_x = floor(pos.x / chunk_size) * chunk_size;
    chunk_start_y = floor(pos.y / chunk_size) * chunk_size;

    v2f chunk_start{
            (float) chunk_start_x,
            (float) chunk_start_y
    };

    return chunk_start;
}

void Terrain::createChunk(DZRenderer &renderer, glm::vec2 pos_in_chunk)
{
    v2f chunkpos {
        pos_in_chunk.x,
        pos_in_chunk.y
    };

    v2f origin = this->getChunkOriginFromPos(chunkpos);

    // Verify no chunk already contains pos
    if(this->chunks.contains(origin))
    {
        return;
    }

    Log::verbose("\tCreating chunk...");

    Chunk chunk(origin, seed, chunk_size);

    this->chunks.emplace(origin, chunk);
}

Chunk* Terrain::getChunkFromPos(v2f pos)
{
    v2f origin = this->getChunkOriginFromPos(pos);
    if(this->chunks.contains(origin))
    {
        // [] operator requires a default cosntructor for Chunk
        // so instead of &this->chunks[origin]
        return &this->chunks.find(origin)->second;
    }
    return nullptr;
}


void Terrain::getVisible(Camera &camera)
{
    std::array<Chunk*, 25> new_visible;
        for (int i = -2; i <= 2; i++)
        {
            for (int j = -2; j <= 2; j++)
            {
                visible[(j+2) * 5 + (i + 2)] 
                    = getChunkFromPos(
                            v2f
                            {
                                camera.position.x + chunk_size * i,
                                camera.position.y + chunk_size * j
                            }
                        );
            }
        }
    visible = new_visible;
}

float glsl_mod(float x, float y) 
{
    return x - y * floor(x / y);
}

int Terrain::getTileIndexFromPos(v2f pos)
{
    const f32 tile_width = chunk_size / TILES_PER_SIDE;
    int x = floor(glsl_mod(pos.x, chunk_size) / tile_width);
    int y = floor(glsl_mod(pos.y, chunk_size) / tile_width);
    return y * TILES_PER_SIDE + x;
}

f32 Terrain::getHeight(glm::vec2 pos)
{
    const siv::PerlinNoise perlin(seed);
    f32 perlin_scale = 0.005f;
    f32 noise_scale = 64.0f;
    u32 octaves = 9;

    f32 noise = noise_scale * std::powf(perlin.octave2D((pos.x) * perlin_scale, (pos.y) * perlin_scale, octaves), 2);
    return noise;
}

void Terrain::updateUniforms(DZRenderer &renderer, std::array<Chunk*, 25> visible) const
{
    MegaChunkData mega_chunk_data;
    for (int i = 0; i < 25; i++)
    {
        if (visible[i])
        {
            memcpy(
                    &mega_chunk_data.material_indices[i * TILES_PER_SIDE * TILES_PER_SIDE],
                    visible[i]->material_indices,
                    TILES_PER_SIDE * TILES_PER_SIDE
                );
        }
    }

    renderer.setBufferOfSize(
            terrain_uniform_buffer, 
            &mega_chunk_data, 
            sizeof(MegaChunkData)
        );
}
