#include "terrain.h"
#include "input.h"
#include "logger.h"
#include "renderer.h"
#include "geometry.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>

#include <queue>
#include <time.h>    

KDTree::KDTree(){}


void KDTree::add(const std::array<BiomePoint, VORONOI_BIOMES> &bpoints)
{
    this->root = new BNode(bpoints[0]);

    root->x_bound_left   = INFINITY;
    root->x_bound_right  = INFINITY;
    root->y_bound_bottom = INFINITY;
    root->y_bound_top    = INFINITY;

    for(int i = 1; i < VORONOI_BIOMES; i++)
    {
            root->add_recur_x(bpoints[i]);
    }

    return;
}

void BNode::add_recur_x(const BiomePoint &bp)
{
    Log::verbose("IN KD: %d", bp.biome);
    if(bp.position.x > p.position.x)
    {
        if(right_child == nullptr)
        {
            right_child = new BNode(bp);

            right_child->x_bound_left   = p.position.x;
            right_child->x_bound_right  = this->x_bound_right;
            right_child->y_bound_bottom = this->y_bound_bottom;
            right_child->y_bound_top    = this->y_bound_top;

            right_child->parent = this;

            return;
        }
        right_child->add_recur_y(bp);
    }
    else
    {
        if(left_child == nullptr)
        {
            left_child = new BNode(bp);

            left_child->x_bound_left   = this->x_bound_left;
            left_child->x_bound_right  = p.position.x;
            left_child->y_bound_bottom = this->y_bound_bottom;
            left_child->y_bound_top    = this->y_bound_top;


            left_child->parent = this;
            return;
        }
        left_child->add_recur_y(bp);
    }
}

void BNode::add_recur_y(const BiomePoint &bp)
{
    if(bp.position.y > p.position.y)
    {
        if(right_child == nullptr)
        {
            right_child = new BNode(bp);

            right_child->x_bound_left   = this->x_bound_left;
            right_child->x_bound_right  = this->x_bound_right;
            right_child->y_bound_bottom = this->y_bound_bottom;
            right_child->y_bound_top    = p.position.y;

            right_child->parent = this;
            return;
        }
        right_child->add_recur_x(bp);
    }
    else 
    {
        if(left_child == nullptr)
        {
            left_child = new BNode(bp);

            left_child->x_bound_left   = this->x_bound_left;
            left_child->x_bound_right  = this->x_bound_right;
            left_child->y_bound_bottom = p.position.y;
            left_child->y_bound_top    = this->y_bound_top;


            left_child->parent = this;
            return;
        }
        left_child->add_recur_x(bp);
    }
}

std::vector<BiomePoint> KDTree::find_n_closest(v2f pos, s32 n)
{
    struct PriorityQueueEntry
    {
        float best_possible_distance;
        BNode *node;

        bool operator<(const PriorityQueueEntry &other) const
        {
            return this->best_possible_distance > other.best_possible_distance;
        }
    };

    struct BestSoFarEntry
    {
        float best_possible_distance;
        BNode *node;

        bool operator<(const BestSoFarEntry &other) const
        {
            return this->best_possible_distance < other.best_possible_distance;
        }
    };

    bool closestfound = false;
    bool tosearch;
    
    // Stores up to n best so far nodes
    std::priority_queue<BestSoFarEntry> best_so_far;
    // Prioritizes those branches that could result in the closest possible points
    std::priority_queue<PriorityQueueEntry> queue;

    queue.push(PriorityQueueEntry{0, root});

    PriorityQueueEntry curr;

    while(!queue.empty())
    {
        v2f bestpossible_left, bestpossible_right;

        curr = queue.top();
        queue.pop();

        best_so_far.push(BestSoFarEntry{(float) curr.node->p.position.distanceFrom(pos), curr.node});
        
        if (best_so_far.size() > n)
        {
            best_so_far.pop();
        }
        
        float left_distance, right_distance;
        if(curr.node->left_child != nullptr)
        {
            BNode *node = curr.node->left_child;
            bestpossible_left = {
                    std::min(std::max(node->x_bound_left, pos.x ), node->x_bound_right),
                    std::min(std::max(node->y_bound_bottom, pos.y), node->y_bound_top)
                };
            left_distance  = bestpossible_left.distanceFrom(pos);

            if(best_so_far.size() < n || left_distance < best_so_far.top().best_possible_distance)
            {
                queue.push(PriorityQueueEntry{left_distance,  curr.node->left_child});
            }
        }

        if(curr.node->right_child != nullptr)
        {
            BNode *node = curr.node->right_child;
            bestpossible_right = {
                    std::min(std::max(node->x_bound_left, pos.x), node->x_bound_right),
                    std::min(std::max(node->y_bound_bottom, pos.y), node->y_bound_top)
                };
            right_distance = bestpossible_right.distanceFrom(pos);

            if(best_so_far.size() < n || right_distance < best_so_far.top().best_possible_distance)
            {
                queue.push(PriorityQueueEntry{right_distance, curr.node->right_child});
            }
        }
    }

    std::vector<BiomePoint> bps;
    while(!best_so_far.empty())
    {
        bps.push_back(best_so_far.top().node->p);
        best_so_far.pop();
    }

    return bps;
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
        f32 chunk_size,
        const std::array<BiomePoint, VORONOI_BIOMES> &bps
        //KDTree *kd,
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
    f32 noise_scale = 16.0f;
    u32 octaves = 9;

    srand(seed);

    const siv::PerlinNoise perlin(seed);

    std::vector<Tile> tiles;

    glm::vec3 normals[(TILES_PER_SIDE + 1) * (TILES_PER_SIDE + 1)];

    memset(normals, 0, sizeof(glm::vec3) * (TILES_PER_SIDE + 1) * (TILES_PER_SIDE + 1));
    memset(navigable, 0, sizeof(char) * TILES_PER_SIDE * TILES_PER_SIDE);


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
                f32 noise = noise_scale * std::powf(perlin.octave2D((startx + corners[i].x) * perlin_scale, (starty + corners[i].y) * perlin_scale, octaves), 2);
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
                        this->material_indices[j * TILES_PER_SIDE + i] = 0;
                    else
                        this->material_indices[j * TILES_PER_SIDE + i] = 1;
                } 
                else if (noise <= -3.5) 
                {
                    this->material_indices[j * TILES_PER_SIDE + i] = 1;
                }
                else if (noise <= -2.0) 
                {
                    this->material_indices[j * TILES_PER_SIDE + i] = 2;
                }

                else if (noise <= 2.0) 
                {
                    this->material_indices[j * TILES_PER_SIDE + i] = 3;
                }

                else if (noise <= 4.0) 
                {
                    this->material_indices[j * TILES_PER_SIDE + i] = 4;
                }

                else if (noise <= 6.5) 
                {
                    this->material_indices[j * TILES_PER_SIDE + i] = 5;
                }

                else if (noise > 8.5) 
                {
                    u32 chance = rand() % 10;
                    if(chance >= 5)
                        this->material_indices[j * TILES_PER_SIDE + i] = 6;
                    else
                        this->material_indices[j * TILES_PER_SIDE + i] = 5;
                }
            }
        }
    }

    for (u32 i = 0; i < TILES_PER_CHUNK; i++)
    {
        //float xpos = i / TILES_PER_CHUNK + startx;
        //float ypos = i % TILES_PER_CHUNK + starty;
        v2f pos = getPosFromTileIndex(i, tile_width);

        f64 reciprocal_distance_table[NUM_BIOMES];
        f64 total_reciprocal_distances = 0.0;

        memset(reciprocal_distance_table, 0, sizeof(f64) * NUM_BIOMES);

        f64 min_dist = INFINITY;
        u32 biome_min_dist = 0;

        for(u32 j = 0; j < VORONOI_BIOMES; j++)
        {
            const BiomePoint &bp = bps[j];
            f64 sq_distance = bp.position.distanceSqFrom(pos);
            if (min_dist > sq_distance )
            {
                min_dist = sq_distance;
                biome_min_dist = bp.biome;
            }
            f64 recip_dist = 8.0 / sq_distance;
            if (recip_dist < 0.001) {
                recip_dist = 0.0;
            }
            reciprocal_distance_table[bp.biome] += recip_dist;
            total_reciprocal_distances += recip_dist;
        }

        if (total_reciprocal_distances != 0.0)
        {

            f64 biome_selector = ((f64)rand() / RAND_MAX) * total_reciprocal_distances;

            u8 selected_biome = 0;

            for (u32 j = 0; j < NUM_BIOMES; j++)
            {
                //if (reciprocal_distance_table[j] > max_so_far)
                //{
                //    max_so_far = reciprocal_distance_table[j];
                //}
                biome_selector -= reciprocal_distance_table[j];
                if (biome_selector <= 0.0)
                {
                    material_indices[i] += j * NUM_TEXTURES_PER_BIOME;
                    break;
                }
            }
            
        }
        else
        {
            material_indices[i] += biome_min_dist * NUM_TEXTURES_PER_BIOME;
        }

        //material_indices[i] += selected_biome + 1 * NUM_TEXTURES_PER_BIOME;
    }
    // TODO: Check each material index w neighbors, make sure they are not one of a kind
    // wrt biome

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
    AssetManager ass_man;
    srand(seed);

    // NO NO NO NO NO NO NO NO NO NO NO NO NO NO NO NO NO NO NO NO
    auto terrain_shader_src = *ass_man.getTextFile("shaders/terrain_shader.metal");
    std::vector<DZShader> terrain_shaders 
        = renderer.compileShaders(terrain_shader_src, {"vertexMain", "fragmentMain"});
    this->terrain_pipeline 
        = renderer.createPipeline(terrain_shaders[0], terrain_shaders[1]);

    this->terrain_uniform_buffer = renderer.createBufferOfSize(sizeof(MegaChunkData));

    memset(this->visible.data(), 0, sizeof(Chunk*) * 9);

    std::array<BiomePoint, VORONOI_BIOMES> biome_arr;
    biome_arr[0].position = v2f{0.0, 0.0};
    biome_arr[0].biome    = BIOME_DEFAULT;

    for(int i = 1; i < VORONOI_BIOMES; i++)
    {

        biome_arr[i].position = v2f {
            (float) (rand() % WORLDSIZE - WORLDSIZE/2),
            (float) (rand() % WORLDSIZE - WORLDSIZE/2)
        };
        f32 start_distance = biome_arr[i].position.distanceFrom(v2f {0.0f, 0.0f}); 

        // CHECK START AREA (CENTER)
        if(start_distance < START_AREA)
            biome_arr[i].biome = BIOME_DEFAULT;
        // CHECK RAINFOREST (WEST)
        else if(biome_arr[i].position.x > START_AREA 
             && biome_arr[i].position.y < START_AREA)
            biome_arr[i].biome = BIOME_RAINFOREST;
        // CHECK COLDLANDS (SOUTH)
        else if(biome_arr[i].position.x > START_AREA 
             && biome_arr[i].position.y > START_AREA)
            biome_arr[i].biome = BIOME_COLDLANDS;
        // CHECK SANDLANDS (NORTH)
        else if(biome_arr[i].position.x < START_AREA 
             && biome_arr[i].position.y < START_AREA)
            biome_arr[i].biome = BIOME_SANDLANDS;
        // CHECK GRAVELANDS (EAST)
        else if(biome_arr[i].position.x < START_AREA 
             && biome_arr[i].position.y > START_AREA) 
            biome_arr[i].biome = BIOME_GRAVELANDS;
        // CHECK MEATLANDS (FAR WEST)
        if(biome_arr[i].position.x > START_AREA * 2 
             && biome_arr[i].position.y < START_AREA * 2)
            biome_arr[i].biome = BIOME_MEATLANDS;
        // CHECK BADLANDS (FAR EAST)
        else if(biome_arr[i].position.x < START_AREA * 2 
             && biome_arr[i].position.y > START_AREA * 2)
            biome_arr[i].biome = BIOME_BADLANDS;
        else
            biome_arr[i].biome = BIOME_DEFAULT;
    }
    //kd.add(biome_arr);
    //
    this->bps = std::move(biome_arr);

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

    Chunk chunk(origin, seed, chunk_size, this->bps);

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
                                chunk->los_indices[index] = 255;
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

void Terrain::getVisible(Camera &camera)
{
    std::array<Chunk*, 9> new_visible;
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                visible[(j+1) * 3 + (i + 1)] 
                    = getChunkFromPos(
                            v2f
                            {
                                camera.target.x + chunk_size * i,
                                camera.target.y + chunk_size * j
                            }
                        );
            }
        }
    visible = new_visible;
}

void Terrain::updateUniforms(DZRenderer &renderer, std::array<Chunk*, 9> visible) const
{
    MegaChunkData mega_chunk_data;
    for (int i = 0; i < 9; i++)
    {
        if (visible[i])
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
    }

    renderer.setBufferOfSize(
            terrain_uniform_buffer, 
            &mega_chunk_data, 
            sizeof(MegaChunkData)
        );
}
