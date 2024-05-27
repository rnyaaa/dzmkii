#include <metal_stdlib>
using namespace metal;

#define TILES_PER_SIDE 64
#define TILES_PER_CHUNK (64 * 64)
#define TILE_DIM     64.0/100.0
#define TILE_ORIGIN  float2(0.0, 0.0)
#define CHUNK_SIZE 100.0
#define MAX_NO_LIGHTS 100

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
    int no_lights;
};

struct PointLight
{
    packed_float3 pos;
    packed_float3 color;
    float falloff;
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

struct [[nodiscard]] neighbor
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

float2 xy_glsl_mod(float2 in, float mod) 
{
    float2 ret;
    ret.x = in.x - mod * floor(in.x / mod);
    ret.y = in.y - mod * floor(in.y / mod);
    return ret;
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
    neighbor center;
    neighbor top;
    neighbor bottom; 
    neighbor left;
    neighbor right;

    neighbor bottomright;
    neighbor bottomleft;
    neighbor topright;    
    neighbor topleft;
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

float magnitude(float2 vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y);
}

// float distance(float2 origin, float2 target)
// {
//     return magnitude(origin - target);
// }

void set_chunk_index(thread neighbor *nbors, int n_nbor)
{
    for(int i = 0; i < n_nbor; i++)
    {
        if (nbors[i].pos.x < 0){
            if(nbors[i].chunk_index % 3 != 0)
            {
                nbors[i].chunk_index -= 1;
            }
        }
        if (nbors[i].pos.y < 0){
            if(nbors[i].chunk_index > 2)
            {
                nbors[i].chunk_index -= 3;
            }
        }
        if (nbors[i].pos.x > CHUNK_SIZE){
            if(nbors[i].chunk_index % 3 != 2)
            {
                nbors[i].chunk_index += 1;
            }
        }
        if (nbors[i].pos.y > CHUNK_SIZE){
            if(nbors[i].chunk_index < 6)
            {
                nbors[i].chunk_index += 3;
            }
        }
    }
}


Neighbors get_neighbors(float2 pos, int chunk_index)
{
    float tile_offset_x = glsl_mod(pos.x, TILE_DIM);
    float tile_offset_y = glsl_mod(pos.y, TILE_DIM);

    float2 center = float2(pos.x - tile_offset_x, pos.y - tile_offset_y);

    Neighbors nbors;

    nbors.center.pos      = center;

    nbors.bottomright.pos = float2(center.x - TILE_DIM, center.y - TILE_DIM);
    nbors.right.pos       = float2(center.x - TILE_DIM, center.y);
    nbors.bottom.pos      = float2(center.x, center.y - TILE_DIM);

    nbors.topleft.pos     = float2(center.x + TILE_DIM, center.y + TILE_DIM);
    nbors.left.pos        = float2(center.x + TILE_DIM, center.y);
    nbors.top.pos         = float2(center.x, center.y + TILE_DIM);

    nbors.bottomleft.pos  = float2(center.x + TILE_DIM, center.y - TILE_DIM);
    nbors.topright.pos    = float2(center.x - TILE_DIM, center.y + TILE_DIM);

    nbors.bottomright.chunk_index = chunk_index;
    nbors.bottomleft.chunk_index  = chunk_index;
    nbors.topright.chunk_index    = chunk_index;
    nbors.topleft.chunk_index     = chunk_index;

    nbors.top.chunk_index         = chunk_index;
    nbors.bottom.chunk_index      = chunk_index;
    nbors.left.chunk_index        = chunk_index;
    nbors.right.chunk_index       = chunk_index;

    nbors.center.chunk_index      = chunk_index;

    return nbors;
};

void set_materials(thread neighbor *nbors, uint8_t const constant *texture_index, uint8_t const constant *los_index, int n_nbor)
{
    for(int i = 0; i < n_nbor; i++)
    {
        nbors[i].material    = texture_index[tile_index_from_pos(nbors[i].pos, nbors[i].chunk_index)];
        nbors[i].los = los_index[tile_index_from_pos(nbors[i].pos, nbors[i].chunk_index)];
    }
};

void set_proportions(thread neighbor *nbors, int n_nbor, float2 fragpos)
{
    float distsum = 0;
    for(int i = 0; i < n_nbor; i++)
    {
        nbors[i].distance = distance(fragpos.xy, nbors[i].pos);
        distsum += nbors[i].distance;
    }

    for(int i = 0; i < n_nbor; i++)
    {
        nbors[i].distance = 0.175 - nbors[i].distance / distsum; 
    }
};


half3 blend(float2 pos, const thread neighbor *nbors, int n_nbor, texture2d_array<half> tex, sampler tex_sampler)
{
    half3 color(0.0);
    for(int i = 0; i < n_nbor; i++)
    {
        half3 material = half3(tex.sample(
                tex_sampler, pos / 8.0, nbors[i].material * 2));
        if(nbors[i].los == 2)
        {
            color.x += material.x * nbors[i].distance;
            color.y += material.y * nbors[i].distance;
            color.z += material.z * nbors[i].distance;
        } else if (nbors[i].los == 1) {
            float gray;
            gray =  material.x * nbors[i].distance;
            gray += material.y * nbors[i].distance;
            gray += material.z * nbors[i].distance;
            gray /= 3;
            
            color.x += gray;
            color.y += gray;
            color.z += gray;
        }
    }

    return color;
};

half3 norm_blend(float2 pos, const thread neighbor nbors[4], int n_nbor, texture2d_array<half> tex, sampler tex_sampler)
{
    half2 pd(0.0); 
    for(int i = 0; i < n_nbor; i++)
    { 
        half3 n = half3(tex.sample(
                tex_sampler, pos / 8.0, nbors[i].material * 2 + 1));

        pd += n.xy/n.z;
    }

    half3 normal = normalize(half3(pd, 1.0));
    return normal * 0.5 + 0.5;
}

half4 fragment fragmentMain( 
        v2f in [[stage_in]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        constant ChunkUniforms &local_uniforms [[ buffer(1) ]],
        constant TerrainUniforms &terrain_uniforms [[ buffer(2) ]],
        constant PointLight *lights [[ buffer(3) ]],
        texture2d_array<half> terrain_textures [[ texture(0) ]],
        sampler texture_sampler [[ sampler(0) ]]
    )
{
    Neighbors neighbors = get_neighbors(in.local_position.xy, local_uniforms.chunk_index);
    neighbor nbors[9] = {
        neighbors.bottomleft,
        neighbors.bottomright,
        neighbors.topleft,
        neighbors.bottomleft,
        neighbors.center,
        neighbors.bottom,
        neighbors.top,
        neighbors.left,
        neighbors.right
    };
    set_chunk_index(nbors, 9);
    set_proportions(nbors, 9, in.world_position.xy);
    set_materials(nbors, &terrain_uniforms.texture_indices[0], &terrain_uniforms.los_indices[0], 9);
    
    half3 texture = blend(in.world_position.xy, &nbors[0], 9, terrain_textures, texture_sampler);
    
    float3 tex_normal = (float3) norm_blend(in.world_position.xy, nbors, 9, terrain_textures, texture_sampler);
    tex_normal = normalize(tex_normal * 2.0 - 1.0);
    float3x3 TBN = float3x3(in.T, in.B, in.N);
    float3 normal = normalize(TBN * tex_normal);


    float3 diffuse = float3(0.0);
    for(int i = 0; i < global_uniforms.no_lights; i++)
    {
        float3 to_light = lights[i].pos - in.world_position.xyz;
        float distance = length(to_light);
        float3 light_dir = normalize(to_light);
        float diffuse_factor = 2.0 * max(dot(normal, light_dir), 0.0) / (distance * distance);
        diffuse += lights[i].color * diffuse_factor;
    }
        
    float3 sun_dir = normalize(global_uniforms.sun_dir);
    float sun_diff = max(dot(normal, sun_dir), 0.0);
    float3 sun_color = float3(1.0, 0.9, 0.7);


    diffuse += sun_color * sun_diff;
    diffuse *= (float3) texture;

    return half4( (half3) diffuse, 1.0 );
}
