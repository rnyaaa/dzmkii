#include "terrain.h"
#include "logger.h"
#include "renderer.h"
#include "geometry.h"
#include <cstring>
#include <map>

MeshData genMeshFromTiles(std::vector<Tile> tiles)
{
    std::vector<Vertex> vertices;
 
    for (auto tile : tiles)
    {
        // TODO(jklmn): Do this using elements instead to reduce vertex count
        vertices.push_back(tile.vertices[0]);
        vertices.push_back(tile.vertices[1]);
        vertices.push_back(tile.vertices[2]);

        vertices.push_back(tile.vertices[0]);
        vertices.push_back(tile.vertices[2]);
        vertices.push_back(tile.vertices[3]);
    }

    return MeshData { 
        vertices,
        {}
    };
}

Chunk::Chunk() {}

Chunk::Chunk(
        DZRenderer &renderer, 
        v2f chunk_start, 
        u32 seed, 
        f32 chunk_size
    )
{
    Log::verbose("Setting up chunk...");

    const f32 tile_width = chunk_size / TILES_PER_SIDE;
    const f32 startx = (f32) chunk_start.x;
    const f32 starty = (f32) chunk_start.y;

    f32 perlin_scale = 0.005f;
    f32 noise_scale = 16.0f;
    u32 octaves = 9;

    const siv::PerlinNoise perlin(seed);

    std::vector<Tile> tiles;

    glm::vec3 normals[(TILES_PER_SIDE + 1) * (TILES_PER_SIDE + 1)];

    memset(normals, 0, sizeof(glm::vec3) * (TILES_PER_SIDE + 1) * (TILES_PER_SIDE + 1));

    Log::verbose("\tChunk construction started...");

    for (u32 i = 0; i < TILES_PER_SIDE; i++)
    {
        for (u32 j = 0; j < TILES_PER_SIDE; j++)
        {
            const f32 x_pos = i * tile_width;
            const f32 y_pos = j * tile_width;
            const f32 x_pos2 = (i + 1) * tile_width;
            const f32 y_pos2 = (j + 1) * tile_width;
                         
            glm::vec3 corners[4] = {
                 glm::vec3(x_pos,  y_pos,  0.0f),
                 glm::vec3(x_pos,  y_pos2, 0.0f),
                 glm::vec3(x_pos2, y_pos2, 0.0f),
                 glm::vec3(x_pos2, y_pos,  0.0f)
            };

            for (u32 i = 0; i < 4; i++)
            {
                f32 noise = noise_scale * perlin.octave2D((startx + corners[i].x) * perlin_scale, (starty + corners[i].y) * perlin_scale, octaves);
                corners[i].z = noise;
            }

            Tile tile;
                
            glm::vec3 normal1 = glm::normalize(glm::cross(corners[2] - corners[0], corners[1] - corners[0]));

            normals[i * TILES_PER_SIDE + j]     += normal1;
            normals[i * TILES_PER_SIDE + (j+1)] += normal1;
            normals[(i+1) * TILES_PER_SIDE + j] += normal1;

            glm::vec3 normal2 = glm::normalize(glm::cross(corners[3] - corners[0], corners[2] - corners[0]));

            normals[i * TILES_PER_SIDE + j]         += normal2;
            normals[(i+1) * TILES_PER_SIDE + j]     += normal2;
            normals[(i+1) * TILES_PER_SIDE + (j+1)] += normal2;

            tile.vertices[0] = {corners[0], glm::vec4(0.0f), glm::vec4(1.0), glm::vec4(0.0f), glm::vec4(0.0f)};
            tile.vertices[1] = {corners[1], glm::vec4(0.0f), glm::vec4(1.0), glm::vec4(0.0f), glm::vec4(0.0f)};
            tile.vertices[2] = {corners[2], glm::vec4(0.0f), glm::vec4(1.0), glm::vec4(0.0f), glm::vec4(0.0f)};
            tile.vertices[3] = {corners[3], glm::vec4(0.0f), glm::vec4(1.0), glm::vec4(0.0f), glm::vec4(0.0f)};
            
            tiles.push_back(tile);
        }
    }

    Log::verbose("\tGenerating normals...");

    for (u32 i = 0; i < TILES_PER_SIDE; i++)
    {
        for (u32 j = 0; j < TILES_PER_SIDE; j++)
        {
             tiles[i * TILES_PER_SIDE + j].vertices[0].normal = glm::vec4(glm::normalize(normals[i     * TILES_PER_SIDE + j    ]), 0.0f);
             tiles[i * TILES_PER_SIDE + j].vertices[1].normal = glm::vec4(glm::normalize(normals[i     * TILES_PER_SIDE + (j+1)]), 0.0f);
             tiles[i * TILES_PER_SIDE + j].vertices[2].normal = glm::vec4(glm::normalize(normals[(i+1) * TILES_PER_SIDE + (j+1)]), 0.0f);
             tiles[i * TILES_PER_SIDE + j].vertices[3].normal = glm::vec4(glm::normalize(normals[(i+1) * TILES_PER_SIDE + j    ]), 0.0f);

             tiles[i * TILES_PER_SIDE + j].vertices[0].calculateTangentAndBitangent();
             tiles[i * TILES_PER_SIDE + j].vertices[1].calculateTangentAndBitangent();
             tiles[i * TILES_PER_SIDE + j].vertices[2].calculateTangentAndBitangent();
             tiles[i * TILES_PER_SIDE + j].vertices[3].calculateTangentAndBitangent();
        }
    }

    Log::verbose("\tSetting LOS indices...");

    memset(this->los_indices, 0, TILES_PER_SIDE * TILES_PER_SIDE);

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
                f32 _perlin_scale = perlin_scale * 1.5;
                f32 _noise_scale = noise_scale;
                u32 _octaves = 10;
                int otherseed = 1010620;
                const siv::PerlinNoise otherperlin(otherseed);
                f32 noise = _noise_scale * otherperlin.octave2D((startx + tile.vertices[k].pos.x) * _perlin_scale, (starty + tile.vertices[k].pos.y) * perlin_scale, _octaves);
                if(noise <= -2.25)
                {
                    this->material_indices[j * TILES_PER_SIDE + i] = 0;
                } 
                else if (noise > -2.25 && noise <= -1.5) 
                {
                    this->material_indices[j * TILES_PER_SIDE + i] = 1;
                }
                else if (noise > -1.5 && noise <= -0.75) 
                {
                    this->material_indices[j * TILES_PER_SIDE + i] = 2;
                }

                else if (noise > -0.75 && noise <= 1.0) 
                {
                    this->material_indices[j * TILES_PER_SIDE + i] = 3;
                }

                else if (noise > 1.0 && noise <= 1.75) 
                {
                    this->material_indices[j * TILES_PER_SIDE + i] = 4;
                }

                else if (noise > 1.75 && noise <= 2.5) 
                {
                    this->material_indices[j * TILES_PER_SIDE + i] = 5;
                }

                else if (noise > 2.5) 
                {
                    this->material_indices[j * TILES_PER_SIDE + i] = 6;
                }
            }
        }
    }

    Log::verbose("\tGenerating mesh data...");

    MeshData mesh_data = genMeshFromTiles(tiles);

    transform.pos = glm::vec3(chunk_start.x, chunk_start.y, 0.0);
    transform.scale = glm::vec3(1.0f);
    transform.rotation = glm::vec3(0.0);

    Log::verbose("\tRegistering mesh with renderer...");

    this->mesh = renderer.createMesh(mesh_data);
    this->local_uniforms_buffer = renderer.createBufferOfSize(sizeof(ChunkData), StorageMode::MANAGED);

    Log::verbose("\tMesh registered...");
}

void Chunk::updateUniforms(DZRenderer &renderer, s32 chunk_index)
{
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
    Chunk chunk(renderer, origin, seed, chunk_size);

    this->chunks[origin] = chunk;
}

Chunk* Terrain::getChunkFromPos(v2f pos)
{
    v2f origin = this->getChunkOriginFromPos(pos);
    if(this->chunks.contains(origin))
    {
        return &this->chunks[origin];
    }
    return nullptr;
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

void Terrain::updateLOS(glm::vec2 pos, int LOS)
{
    circ2f circle {
        v2f {
            pos.x,
            pos.y
        },
        (float) LOS
    };

    AArect2f los_rect {
        v2f{
            circle.pos.x - circle.radius,
            circle.pos.y - circle.radius
        },
        v2f {
            circle.radius * 2,
            circle.radius * 2
        }
    };

    f32 tile_width = chunk_size / TILES_PER_SIDE;

    v2f origin = getChunkOriginFromPos(v2f{pos.x, pos.y});

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            v2f nbor_origin = origin + v2f { i * chunk_size, j * chunk_size };
            auto chunk_rect = AArect2f {
                nbor_origin,
                v2f { chunk_size, chunk_size}
            };

            if (chunk_rect.collidesWith(circle))
            {
                if (auto intersection = los_rect.getIntersectingRect(chunk_rect))
                {
                    Chunk *chunk = this->getChunkFromPos(intersection->pos);

                    if (!chunk) continue;

                    v2f origin {
                        chunk->transform.pos.x,
                        chunk->transform.pos.y
                    };

                    int n_tiles_x = intersection->dim.x / tile_width;
                    int n_tiles_y = intersection->dim.y / tile_width;
                    for(int i = 0; i < n_tiles_x; i++)
                    {
                        for(int j = 0; j < n_tiles_y; j++)
                        {
                            v2f tile_center = {
                                intersection->pos.x + tile_width * (i + 0.5f),
                                intersection->pos.y + tile_width * (j + 0.5f)
                            };

                            if (tile_center.distanceFrom(circle.pos) <= circle.radius)
                            {
                                int index = this->getTileIndexFromPos(tile_center);
                                chunk->los_indices[index] = 2;
                            }
                        }
                    }
                }
            }
        }
    }
}

void Terrain::termRender(DZTermRenderer &term, glm::vec2 pos)
{
    f32 tile_width = chunk_size / TILES_PER_SIDE;

    std::vector<char> charmap = { ' ', '.', ',', ':', ';', '#' };

    v2i center;
    center.x = floor(pos.x / tile_width);
    center.y = floor(pos.y / tile_width);
    
    v2i termdim = term.getDimensions();
    v2i term_origin;
    term_origin.x = center.x - termdim.x/2;
    term_origin.y = center.y - termdim.y/2;

    for (auto &map_entry : this->chunks)
    {
        Chunk &chunk = map_entry.second;
        v2i termcoord;
        termcoord.x = floor(chunk.transform.pos.x / tile_width);
        termcoord.y = floor(chunk.transform.pos.y / tile_width);
        
        int termboundx = termcoord.x + TILES_PER_SIDE;
        int termboundy = termcoord.y + TILES_PER_SIDE;

        if (termcoord.x <= center.x && center.x <= termboundx && 
            termcoord.y <= center.y && center.y <= termboundy)
        {
            for(int i = 0; i < TILES_PER_SIDE; i++)
            {
                for(int j = 0; j < TILES_PER_SIDE; j++)
                {
                    v2i fragcoord;
                    fragcoord.x = termcoord.x + i - term_origin.x;
                    fragcoord.y = termcoord.y + j - term_origin.y;
                    int mat = charmap[chunk.material_indices[j * TILES_PER_SIDE + i]];
                    term.putChar(mat, fragcoord.x, fragcoord.y);
                }
            }
        }
    }
}

void Terrain::updateUniforms(DZRenderer &renderer, std::array<Chunk*, 9> visible)
{
        MegaChunkData mega_chunk_data;
        for (int i = 0; i < 9; i++)
        {
            memcpy(
                    &mega_chunk_data.material_indices[i * TILES_PER_SIDE * TILES_PER_SIDE],
                    visible[i]->material_indices,
                    TILES_PER_SIDE * TILES_PER_SIDE
                );
            memcpy(
                    &mega_chunk_data.los_indices[i * TILES_PER_SIDE * TILES_PER_SIDE],
                    visible[i]->los_indices,
                    TILES_PER_SIDE * TILES_PER_SIDE
                );
        }

        renderer.setBufferOfSize(
                terrain_uniform_buffer, 
                &mega_chunk_data, 
                sizeof(MegaChunkData)
            );
    }