#include "chunk.h"

#include <cstdlib>
#include <random>

void Chunk::setMatIndexByPos(glm::vec3 pos, u8 mat)
{
    // GET CHUNK LOC, NORMALIZE
    glm::vec2 indexpos;
    indexpos = glm::vec2(floor(pos.x/100.0f), floor(pos.y/100.0f));
    u32 index = floor(indexpos.x / Chunk::TILES_PER_SIDE) + (int(indexpos.y) % Chunk::TILES_PER_SIDE);
        
    this->material_indices[index] = mat;
}

Chunk::Chunk(glm::vec2 chunk_start)
{
    
    std::random_device rd;
    std::uniform_real_distribution<float> dis(0.0f, 1.0f); 
    float randomFloat = 0.0;

    const f32 chunk_width = 100.0f;
    const f32 tile_width = chunk_width / Chunk::TILES_PER_SIDE;
    const f32 startx = chunk_start.x - chunk_width/2.0f;
    const f32 starty = chunk_start.y - chunk_width/2.0f;

    f32 perlin_scale = 0.025f;
    f32 noise_scale = 4.0f;
    u32 octaves = 9;

    const siv::PerlinNoise::seed_type seed = 1236u;
    const siv::PerlinNoise perlin{ seed };

    std::vector<Tile> tiles;

    std::vector<u8> material_indices((Chunk::TILES_PER_SIDE + 1) * (Chunk::TILES_PER_SIDE + 1));

    glm::vec3 normals[(Chunk::TILES_PER_SIDE + 1) * (Chunk::TILES_PER_SIDE + 1)];

    memset(normals, 0, sizeof(glm::vec3) * (Chunk::TILES_PER_SIDE + 1) * (Chunk::TILES_PER_SIDE + 1));

    for (u32 i = 0; i < Chunk::TILES_PER_SIDE; i++)
    {
        for (u32 j = 0; j < Chunk::TILES_PER_SIDE; j++)
        {
            const f32 x_pos = startx + i * tile_width;
            const f32 y_pos = starty + j * tile_width;
            const f32 x_pos2 = startx + (i + 1) * tile_width;
            const f32 y_pos2 = starty + (j + 1) * tile_width;
                         
            glm::vec3 corners[4] = {
                 glm::vec3(x_pos,  y_pos,  0.0f),
                 glm::vec3(x_pos,  y_pos2, 0.0f),
                 glm::vec3(x_pos2, y_pos2, 0.0f),
                 glm::vec3(x_pos2, y_pos,  0.0f)
            };

            for (u32 i = 0; i < 4; i++)
            {
                f32 noise = noise_scale * perlin.octave2D(corners[i].x * perlin_scale, corners[i].y * perlin_scale, octaves);
                corners[i].z = noise;
            }

            Tile tile;
                
            glm::vec3 normal1 = glm::normalize(glm::cross(corners[2] - corners[0], corners[1] - corners[0]));

            normals[i * Chunk::TILES_PER_SIDE + j] += normal1;
            normals[i * Chunk::TILES_PER_SIDE + (j+1)] += normal1;
            normals[(i+1) * Chunk::TILES_PER_SIDE + j] += normal1;

            glm::vec3 normal2 = glm::normalize(glm::cross(corners[3] - corners[0], corners[2] - corners[0]));

            normals[i * Chunk::TILES_PER_SIDE + j] += normal2;
            normals[(i+1) * Chunk::TILES_PER_SIDE + j] += normal2;
            normals[(i+1) * Chunk::TILES_PER_SIDE + (j+1)] += normal2;

            tile.vertices[0] = {corners[0], glm::vec4(0.0f), glm::vec4(1.0)};
            tile.vertices[1] = {corners[1], glm::vec4(0.0f), glm::vec4(1.0)};
            tile.vertices[2] = {corners[2], glm::vec4(0.0f), glm::vec4(1.0)};
            tile.vertices[3] = {corners[3], glm::vec4(0.0f), glm::vec4(1.0)};
            
            tiles.push_back(tile);
        }
    }


    for (u32 i = 0; i < Chunk::TILES_PER_SIDE; i++)
    {
        for (u32 j = 0; j < Chunk::TILES_PER_SIDE; j++)
        {
             tiles[i * Chunk::TILES_PER_SIDE + j].vertices[0].normal = glm::vec4(glm::normalize(normals[i     * Chunk::TILES_PER_SIDE + j    ]), 0.0f);
             tiles[i * Chunk::TILES_PER_SIDE + j].vertices[1].normal = glm::vec4(glm::normalize(normals[i     * Chunk::TILES_PER_SIDE + (j+1)]), 0.0f);
             tiles[i * Chunk::TILES_PER_SIDE + j].vertices[2].normal = glm::vec4(glm::normalize(normals[(i+1) * Chunk::TILES_PER_SIDE + (j+1)]), 0.0f);
             tiles[i * Chunk::TILES_PER_SIDE + j].vertices[3].normal = glm::vec4(glm::normalize(normals[(i+1) * Chunk::TILES_PER_SIDE + j    ]), 0.0f);
        }
    }

    // Teture
    for(auto tile : this->tiles)
    {

        for (u32 i = 0; i < 4; i++)
        {
            f32 noise = noise_scale * perlin.octave2D(tile.vertices[i].pos.x * perlin_scale, tile.vertices[i].pos.y * perlin_scale, octaves);
            if(noise > 0.5)
            {
                setMatIndexByPos(tile.vertices[i].pos, 1);
            } else {
                setMatIndexByPos(tile.vertices[i].pos, 0);
            }
        }

    }
    this->tiles = tiles;
    this->material_indices = material_indices;
}


