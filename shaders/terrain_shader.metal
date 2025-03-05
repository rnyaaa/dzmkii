#include <metal_stdlib>
#include "noise.hm"
#include "headers.hm"
#include "lighting.hm"
#include "fog.hm"
using namespace metal;

#define TILES_PER_SIDE 64
#define TILES_PER_CHUNK (64 * 64)
#define TILE_DIM     (64.0/100.0)
#define TILE_ORIGIN  float2(0.0, 0.0)
#define CHUNK_SIZE 100.0

struct TerrainUniforms 
{
    uint8_t texture_indices[5 * 5 * 64 * 64];    
};

struct ChunkUniforms
{
    float4x4 model_matrix;
    int32_t chunk_index;
};

struct [[nodiscard]] neighbor
{
    float2 pos;
    half3 color;
    half3 norm;
    float distance;
    float alpha;
    uint8_t material;
    int32_t chunk_index;

    neighbor() 
    {
        pos = float2 {0.0, 0.0};
        alpha = 0.0;
        distance = 0.0;
        material = 255.0;
        chunk_index = 0;
    }
};

float glsl_mod(float x, float y) 
{
    return x - y * floor(x / y);
}

int32_t get_chunk_index(float2 pos, int32_t chunk_index)
{
    int32_t ret = chunk_index;

    if (pos.x < 0)
    {
        if(chunk_index % 5 != 0)
        {
            ret -= 1;
        }
    }
    else if (pos.x > CHUNK_SIZE)
    {
        if(chunk_index % 5 != 4)
        {
            ret += 1;
        }
    }

    if (pos.y < 0)
    {
        if(chunk_index > 4)
        {
            ret -= 5;
        }
    }
    else if (pos.y > CHUNK_SIZE)
    {
        if(chunk_index < 20)
        {
            ret += 5;
        }
    }
    return ret;
}

void get_neighbors(float2 frag_pos, int32_t chunk_index, thread neighbor *nbors)
{
    float tile_offset_x = glsl_mod(frag_pos.x, TILE_DIM);
    float tile_offset_y = glsl_mod(frag_pos.y, TILE_DIM);

    float2 tile_center = float2(frag_pos.x - tile_offset_x, frag_pos.y - tile_offset_y) + float2(TILE_DIM / 2.0);
    
    int count = 0;
    float2 diff = frag_pos - tile_center;
    // LEFT

    int i_start, i_max, j_start, j_max;

    i_start = (diff.x < 0) ? -1 : 0;
    i_max   = (diff.x < 0) ?  0 : 1;
    j_start = (diff.y < 0) ? -1 : 0;
    j_max   = (diff.y < 0) ?  0 : 1;

    for(int i = i_start; i <= i_max; i++)
    {
        for(int j = j_start; j <= j_max; j++)
        {
            nbors[count].pos = float2(tile_center.x + (i * TILE_DIM), tile_center.y + (j *  TILE_DIM));
            nbors[count].chunk_index = get_chunk_index(nbors[count].pos, chunk_index);
            count += 1;
        }
    }
};

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
    int x = floor(pos.x / tile_width);
    int y = floor(pos.y / tile_width);
    return TILES_PER_CHUNK * chunk_index + (y * TILES_PER_SIDE + x);
};

void set_materials(thread neighbor *nbors, uint8_t const constant *texture_index, int n_nbor)
{
    for(int i = 0; i < n_nbor; i++)
    {
        nbors[i].material = texture_index[tile_index_from_pos(nbors[i].pos, nbors[i].chunk_index)];
    }
};

void set_proportions(thread neighbor *nbors, int n_nbor, float2 fragpos)
{
    float2 rel_pos = (fragpos - nbors[0].pos) / TILE_DIM;

    nbors[0].distance = (1 - rel_pos.x) * (1 - rel_pos.y);
    nbors[1].distance = (1 - rel_pos.x) * rel_pos.y;
    nbors[2].distance = rel_pos.x * (1 - rel_pos.y);
    nbors[3].distance = rel_pos.x * rel_pos.y;
};

v2f vertex terrain_vertexMain( 
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

    return o;
};

half3 blend(float2 pos, thread neighbor *nbors, int n_nbor, texture2d_array<half> tex, sampler tex_sampler)
{
    half3 color(0.0);
    for(int i = 0; i < n_nbor; i++)
    {            
        half3 material = half3(tex.sample(
                tex_sampler, pos / 8.0, nbors[i].material * 3));

        half3 normal = half3(tex.sample(
                tex_sampler, pos / 8.0, nbors[i].material * 3 + 1));

        half4 alpha = half4(tex.sample(
                tex_sampler, float2(pos.x + i % 2 + 1, pos.y + i % 2 -1) / 2, nbors[i].material * 3 + 2));

        nbors[i].color = material;
        nbors[i].norm  = normal;
        nbors[i].alpha = alpha.x * nbors[i].distance; 
    } 
    neighbor max_alpha;
    for(int i = 0; i < n_nbor; i++)
    {
        color += nbors[i].color * nbors[i].distance;
        if(nbors[i].alpha > max_alpha.alpha)
        {
            max_alpha = nbors[i];
        }
    }
    return mix(max_alpha.color, color, 0.5);
};

half3 norm_blend(float2 pos, const thread neighbor nbors[9], int n_nbor, texture2d_array<half> tex, sampler tex_sampler)
{
    neighbor max_alpha;
    for(int i = 0; i < n_nbor; i++)
    {
        if(nbors[i].alpha > max_alpha.alpha)
        {
           max_alpha = nbors[i]; 
        }
    }
    return max_alpha.norm;
}

half4 fragment terrain_fragmentMain( 
        v2f in [[stage_in]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        constant ChunkUniforms &local_uniforms [[ buffer(1) ]],
        constant TerrainUniforms &terrain_uniforms [[ buffer(2) ]],
        constant PointLight *lights [[ buffer(3) ]],
        texture2d_array<half> terrain_textures [[ texture(0) ]],
        sampler texture_sampler [[ sampler(0) ]]
    )
{    
    neighbor nbors[4];
    get_neighbors(in.local_position.xy, local_uniforms.chunk_index, nbors);
    set_proportions(nbors, 4, in.local_position.xy);
    set_materials(nbors, &terrain_uniforms.texture_indices[0], 4);
    
    half3 texture = blend(in.local_position.xy, nbors, 4, terrain_textures, texture_sampler);
    
    float3 tex_normal = (float3) norm_blend(in.world_position.xy, &nbors[0], 4, terrain_textures, texture_sampler);
    tex_normal        = normalize(tex_normal * 2 - 1);
    float3x3 TBN      = float3x3(normalize(in.T), normalize(in.B), normalize(in.N));
    float3 normal     = normalize(TBN * tex_normal);

    TBN[0] = normalize(cross(normal, in.B));
    TBN[1] = normalize(cross(normal, TBN[0]));
    TBN[2] = normal;

    float3x3 to_tangent = transpose(TBN);

    Material terrain_material;
    terrain_material.color             = float3(texture);
    terrain_material.roughness         = 0.f; 
    terrain_material.subsurface        = .0f;
    terrain_material.sheen             = .0f; 
    terrain_material.sheen_tint        = float3(1.f, 0.1f, 0.1f);
    terrain_material.anisotropic       = .0f;
    terrain_material.specular_strength = .0f;
    terrain_material.specular_tint     = float3(.1f, .8f, .1f);
    terrain_material.metallic          = .0f;
    terrain_material.clearcoat         = .0f;
    terrain_material.clearcoat_gloss   = .0f;
    terrain_material.ior               = .0f;
    terrain_material.relative_ior      = .0f;
    terrain_material.flatness          = 0.0f;
   
    Sun sun;
    sun.sun_dir       = global_uniforms.sun_dir;
    sun.sun_color     = global_uniforms.sun_color;
    float3 reflection = do_lighting(terrain_material, to_tangent, in.world_position.xyz, global_uniforms.camera.position.xyz, sun, &lights[MAX_NO_LIGHTS]);

    float3 fogColor = do_fog(global_uniforms.tod);

    float fogEnd    = 100.0;
    float fogStart  = 5.0;
    float depth     = in.position.z / in.position.w;
    float fogFactor = clamp((fogEnd - depth) / (fogEnd - fogStart), 0.0, 1.0);

    //reflection    = dot(sun.sun_dir, normalize(in.N));

    return half4(half3(mix(fogColor, reflection, fogFactor)), 1.0);
};
