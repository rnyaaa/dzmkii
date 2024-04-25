#include <metal_stdlib>
using namespace metal;

#define TILES_PER_SIDE 64
#define TILES_PER_CHUNK (64 * 64)
#define TILE_DIM     64.0/100.0
#define TILE_ORIGIN  float2(0.0, 0.0)
#define CHUNK_SIZE 100.0
struct v2f
{
    float4 position [[position]];
    float4 world_position;
    float4 local_position;
    float3 T;
    float3 B;
    float3 N;
    half3 color;
};

struct Vertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    float3 bitangent;
    float3 color;
    float2 uv;
};

struct CameraData
{
    float4x4 view_matrix;
    float4x4 projection_matrix;
};

struct GlobalUniforms
{
    CameraData camera;
    float3 sun_dir;
};

struct TerrainUniforms 
{
    uint8_t texture_indices[3 * 3 * 64 * 64];    
    uint8_t los_indices[3 * 3 * 64 * 64]; 
    // 0 = Unexplored  1 = Seen  2 == Visible
};

struct ChunkUniforms
{
    float4x4 model_matrix;
    int32_t chunk_index;
};

struct neighbor
{
    float2 pos;
    float distance;
    uint8_t material;
    uint8_t los;
    int32_t chunk_index;
};

// x = -1, 2;

float glsl_mod(float x, float y) 
{
    return x - y * floor(x / y);
}

// 11 22 33
// 11 22 33   
// 44 55 66
// 44 55 66
// 77 88 99
// 77 88 99

uint16_t tile_index_from_pos(float2 pos, int chunk_index)
{
    if (pos.x < 0)
    {
        pos.x += CHUNK_SIZE;
    }
    if (pos.y < 0)
    {
        pos.y += CHUNK_SIZE;
    }
    if (pos.x > CHUNK_SIZE)
    {
        pos.x -= CHUNK_SIZE;
    }
    if (pos.y > CHUNK_SIZE)
    {
        pos.y -= CHUNK_SIZE;
    }
    float tile_width = CHUNK_SIZE / TILES_PER_SIDE;
    //int x = floor(glsl_mod(pos.x, CHUNK_SIZE) / tile_width);
    //int y = floor(glsl_mod(pos.y, CHUNK_SIZE) / tile_width);
    int x = floor(pos.x / tile_width);
    int y = floor(pos.y / tile_width);
    return TILES_PER_CHUNK * chunk_index + (y * TILES_PER_SIDE + x);
}

struct Neighbors
{
    neighbor bottomright;
    neighbor bottomleft;
    neighbor topright;    
    neighbor topleft;

    void set_materials(uint8_t const constant *texture_index, uint8_t const constant *los_index)
    {
        bottomright.material = texture_index[tile_index_from_pos(bottomright.pos, bottomright.chunk_index)];
        bottomleft.material  = texture_index[tile_index_from_pos(bottomleft.pos, bottomleft.chunk_index)];
        topright.material    = texture_index[tile_index_from_pos(topright.pos, topright.chunk_index)];
        topleft.material     = texture_index[tile_index_from_pos(topleft.pos, topleft.chunk_index)];

        bottomright.los = los_index[tile_index_from_pos(bottomright.pos, bottomright.chunk_index)];
        bottomleft.los  = los_index[tile_index_from_pos(bottomleft.pos, bottomleft.chunk_index)];
        topright.los    = los_index[tile_index_from_pos(topright.pos, topright.chunk_index)];
        topleft.los     = los_index[tile_index_from_pos(topleft.pos, topleft.chunk_index)];
    };

    void set_proportions(float2 fragpos)
    {
        bottomright.distance = 1.0 - distance(fragpos.xy, bottomright.pos);
        bottomleft.distance  = 1.0 - distance(fragpos.xy, bottomleft.pos);
        topright.distance    = 1.0 - distance(fragpos.xy, topright.pos);
        topleft.distance     = 1.0 - distance(fragpos.xy, topleft.pos);

        float distsum = bottomright.distance
                        + bottomleft.distance
                        + topright.distance
                        + topleft.distance;

        bottomright.distance /= distsum; 
        bottomleft.distance  /= distsum; 
        topright.distance    /= distsum; 
        topleft.distance     /= distsum;    
    };

};

v2f vertex vertexMain( 
        uint vertex_id [[ vertex_id ]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        device const Vertex *vertices [[ buffer(1) ]],
        constant ChunkUniforms  &local_uniforms  [[ buffer(2) ]]
    )
{
    v2f o;
    o.local_position = float4(vertices[vertex_id].position, 1.0);
    o.world_position = local_uniforms.model_matrix * float4(vertices[vertex_id].position, 1.0);
    o.position = global_uniforms.camera.projection_matrix * global_uniforms.camera.view_matrix * o.world_position;
    o.color = half3 ( vertices[vertex_id].color );

    o.T = vertices[vertex_id].tangent.xyz;
    o.B = vertices[vertex_id].bitangent.xyz;
    o.N = vertices[vertex_id].normal.xyz;

    //o.textures = TerrainUniforms.texture_index;
    return o;
};


void set_chunk_index(thread neighbor &nbor)
{
    if (nbor.pos.x < 0){
        if(nbor.chunk_index % 3 != 0)
        {
            nbor.chunk_index -= 1;
        }
    }
    if (nbor.pos.y < 0){
        if(nbor.chunk_index > 2)
        {
            nbor.chunk_index -= 3;
        }
    }
    if (nbor.pos.x > CHUNK_SIZE){
        if(nbor.chunk_index % 3 != 2)
        {
            nbor.chunk_index += 1;
        }
    }
    if (nbor.pos.y > CHUNK_SIZE){
        if(nbor.chunk_index < 6)
        {
            nbor.chunk_index += 3;
        }

    }
};


Neighbors get_neighbors(float2 pos, int chunk_index)
{
    float tile_offset_x = glsl_mod(pos.x, TILE_DIM);
    float tile_offset_y = glsl_mod(pos.y, TILE_DIM);

    float2 center = float2(pos.x - tile_offset_x, pos.y - tile_offset_y);

    bool is_top = pos.x > center.x;
    bool is_left = pos.y < center.y;

    Neighbors nbors;

    if(is_top)
    {
        if(is_left)
        {
            nbors.bottomright.pos = center;
            nbors.bottomleft.pos  = float2(center.x, center.y - TILE_DIM);
            nbors.topright.pos    = float2(center.x + TILE_DIM, center.y);
            nbors.topleft.pos     = float2(center.x + TILE_DIM, center.y - TILE_DIM);
        } else {
            nbors.bottomright.pos = float2(center.x, center.y + TILE_DIM);
            nbors.bottomleft.pos  = center;
            nbors.topright.pos    = float2(center.x + TILE_DIM, center.y + TILE_DIM);
            nbors.topleft.pos     = float2(center.x + TILE_DIM, center.y);
        }
    } else{
        if(is_left)
        {
            nbors.bottomright.pos = float2(center.x - TILE_DIM, center.y);
            nbors.bottomleft.pos  = float2(center.x - TILE_DIM, center.y - TILE_DIM);
            nbors.topright.pos    = center;
            nbors.topleft.pos     = float2(center.x, center.y - TILE_DIM * 2);
        } else {
            nbors.bottomright.pos = float2(center.x - TILE_DIM, center.y + TILE_DIM);
            nbors.bottomleft.pos  = float2(center.x - TILE_DIM, center.y);
            nbors.topright.pos    = float2(center.x, center.y + TILE_DIM);
            nbors.topleft.pos     = center;
        }

    }

    nbors.bottomright.chunk_index = chunk_index;
    nbors.bottomleft.chunk_index  = chunk_index;
    nbors.topright.chunk_index    = chunk_index;
    nbors.topleft.chunk_index     = chunk_index;

    return nbors;
};

half3 blend(float2 pos, Neighbors nbors, texture2d_array<half> tex, sampler tex_sampler)
{
    half3 color(0.0);

    half3 material = half3(tex.sample(
            tex_sampler, pos / 8.0, nbors.bottomright.material * 2));
    if(nbors.bottomright.los == 2)
    {
        color.x += material.x * nbors.bottomright.distance;
        color.y += material.y * nbors.bottomright.distance;
        color.z += material.z * nbors.bottomright.distance;
    } else if (nbors.bottomright.los == 1) {
        float gray;
        gray =  material.x * nbors.bottomright.distance;
        gray += material.y * nbors.bottomright.distance;
        gray += material.z * nbors.bottomright.distance;
        gray /= 3;
        
        color.x += gray;
        color.y += gray;
        color.z += gray;
    }

    material = half3(tex.sample(
        tex_sampler, pos / 8.0, nbors.bottomleft.material * 2));
    if(nbors.bottomleft.los == 2)
    {
        color.x += material.x * nbors.bottomleft.distance;
        color.y += material.y * nbors.bottomleft.distance;
        color.z += material.z * nbors.bottomleft.distance;
    } else if (nbors.bottomleft.los == 1) {
        float gray;
        gray =  material.x * nbors.bottomleft.distance;
        gray += material.y * nbors.bottomleft.distance;
        gray += material.z * nbors.bottomleft.distance;
        gray /= 3;
        
        color.x += gray;
        color.y += gray;
        color.z += gray;
    }
    
    material = half3(tex.sample(
        tex_sampler, pos / 8.0, nbors.topright.material * 2));
    if(nbors.topright.los == 2)
    {
        color.x += material.x * nbors.topright.distance;
        color.y += material.y * nbors.topright.distance;
        color.z += material.z * nbors.topright.distance;
    } else if (nbors.topright.los == 1) {
        float gray;
        gray =  material.x * nbors.topright.distance;
        gray += material.y * nbors.topright.distance;
        gray += material.z * nbors.topright.distance;
        gray /= 3;
        
        color.x += gray;
        color.y += gray;
        color.z += gray;
    }

    material = half3(tex.sample(
        tex_sampler, pos / 8.0, nbors.topleft.material * 2));
    if(nbors.topleft.los == 2)
    {
    color.x += material.x * nbors.topleft.distance;
    color.y += material.y * nbors.topleft.distance;
    color.z += material.z * nbors.topleft.distance;
    } else if (nbors.topleft.los == 1)
    {
    float gray;
    gray =  material.x * nbors.topleft.distance;
    gray += material.y * nbors.topleft.distance;
    gray += material.z * nbors.topleft.distance;
    gray /= 3;
    color.x += gray;
    color.y += gray;
    color.z += gray;
    }

    return color;
};

half3 norm_blend(float2 pos, Neighbors nbors, texture2d_array<half> tex, sampler tex_sampler)
{
    half3 n1 = half3(tex.sample(
            tex_sampler, pos / 8.0, nbors.bottomright.material * 2 + 1));

    half3 n2 = half3(tex.sample(
            tex_sampler, pos / 8.0, nbors.bottomleft.material * 2 + 1));

    half3 n3 = half3(tex.sample(
            tex_sampler, pos / 8.0, nbors.topright.material * 2 + 1));

    half3 n4 = half3(tex.sample(
            tex_sampler, pos / 8.0, nbors.topleft.material * 2 + 1));

    half2 pd = n1.xy/n1.z + n2.xy/n2.z + n3.xy/n3.z + n4.xy/n4.z;

    half3 normal = normalize(half3(pd, 1.0));

    return normal * 0.5 + 0.5;
}

half4 fragment fragmentMain( 
        v2f in [[stage_in]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        constant ChunkUniforms &local_uniforms [[ buffer(1) ]],
        constant TerrainUniforms &terrain_uniforms [[ buffer(2) ]],
        texture2d_array<half> terrain_textures [[ texture(0) ]],
        sampler texture_sampler [[ sampler(0) ]]
    )
{
    Neighbors neighbors = get_neighbors(in.local_position.xy, local_uniforms.chunk_index);
    set_chunk_index(neighbors.bottomright);
    set_chunk_index(neighbors.bottomleft);
    set_chunk_index(neighbors.topright);
    set_chunk_index(neighbors.topleft);
    neighbors.set_proportions(in.world_position.xy);
    neighbors.set_materials(&terrain_uniforms.texture_indices[0], &terrain_uniforms.los_indices[0]);
    
    half3 texture = blend(in.world_position.xy, neighbors, terrain_textures, texture_sampler);
    
    
    float3 tex_normal = (float3) norm_blend(in.world_position.xy, neighbors, terrain_textures, texture_sampler);
    tex_normal = normalize(tex_normal * 2.0 - 1.0);

    //texture = (CHUNK_SIZE / in.local_position.y); 
    float3x3 TBN = float3x3(in.T, in.B, in.N);

    float3 normal = normalize(TBN * tex_normal);
    //uint8_t index = local_uniforms.texture_index[tile_index_from_pos(in.world_position.xy)];
    //half3 texture = 
    //    half3(terrain_textures.sample(
    //        texture_sampler, in.world_position.xy / 8.0, neighbors.bottomright.material));

    //texture = half3(1.0, 0.0, 1.0);

    if(neighbors.topleft.pos.y <= 0 || neighbors.topright.pos.y <= 0)
    {
        //texture = (255.0);
    }
    if(neighbors.topleft.pos.y < 0 || neighbors.topright.pos.y > CHUNK_SIZE)
    {
        //texture = (100.0, 0.0, 0.0);
    }

    float3 sun_dir = normalize(global_uniforms.sun_dir);
    float sun_diff = max(dot(normal, sun_dir), 0.0);
    half3 sun_color = half3(1.0, 0.9, 0.7);
    half3 diffuse = 1.0 * sun_color * sun_diff * texture + 0.0 * texture;
    //diffuse = half3(tile_index_from_pos(in.world_position.xy) / (64.0 * 64.0));

    //diffuse = half3(in.world_position.x, in.world_position.y, 0.0);

    //diffuse = half3(0.0);
 
    

    return half4( diffuse, 1.0 );
}
