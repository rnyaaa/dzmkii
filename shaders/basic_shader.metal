#include <metal_stdlib>
using namespace metal;

struct v2f
{
    float4 position [[position]];
    float4 local_position;
    float4 world_position;
    float3 T;
    float3 B;
    float3 N;
    half3 color;
};

struct Vertex
{
    float4 position;
    float4 normal;
    float4 tangent;
    float4 bitangent;
    float4 color;
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
    int debug_texture;
};

struct PointLight
{
    packed_float3 pos;
    packed_float3 color;
    float falloff;
};

struct ModelUniforms
{
    float4x4 model_matrix;
    bool textured;
    bool lit;
};

v2f vertex vertexMain( 
        uint vertex_id [[ vertex_id ]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        device const Vertex *vertices [[ buffer(1) ]],
        constant ModelUniforms  &local_uniforms  [[ buffer(2) ]]
    )
{
    v2f o;
    o.local_position = vertices[vertex_id].position;
    o.world_position = local_uniforms.model_matrix * vertices[vertex_id].position;

    o.position = global_uniforms.camera.projection_matrix * global_uniforms.camera.view_matrix * o.world_position;

    o.color = half3 ( vertices[vertex_id].color.xyz );

    o.T = vertices[vertex_id].tangent.xyz;
    o.B = vertices[vertex_id].bitangent.xyz;
    o.N = normalize(local_uniforms.model_matrix * vertices[vertex_id].normal).xyz;

    return o;
};

    half3 blend(float2 pos, texture2d_array<half> tex, sampler tex_sampler, int index)
    {
        half3 color(0.0);
            half3 material = half3(tex.sample(
                    tex_sampler, pos, index));
        color.x += material.x;
        color.y += material.y;
        color.z += material.z;
        
        return color;
    };

    half3 norm_blend(float2 pos, texture2d_array<half> tex, sampler tex_sampler, int index)
    {
        half2 pd(0.0); 
 
            half3 n = half3(tex.sample(
                    tex_sampler, pos, index + 1));

            pd += n.xy/n.z;

        half3 normal = normalize(half3(n.xy, 1.0));
        return normal * 0.5 + 0.5;
    }



half4 fragment fragmentMain( 
        v2f in [[stage_in]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        constant ModelUniforms &local_uniforms [[ buffer(1) ]],
        texture2d_array<half> terrain_textures [[ texture(0) ]],
        constant PointLight *lights [[ buffer(3) ]],
        sampler texture_sampler [[ sampler(0) ]]
    )
{
        half3 texture = blend(in.local_position.xy, terrain_textures, texture_sampler, global_uniforms.debug_texture);
        
        float3 tex_normal = (float3) norm_blend(in.local_position.xy, terrain_textures, texture_sampler, global_uniforms.debug_texture+1);
        
        tex_normal = normalize(tex_normal * 2.0 - 1.0);
        float3x3 TBN = float3x3(in.T, in.B, in.N);
        float3 normal = normalize(in.N * tex_normal);


        float3 diffuse = float3(0.0);
        //for(int i = 0; i < global_uniforms.no_lights; i++)
        //{
        //    float3 to_light = lights[i].pos - in.world_position.xyz;
        //    float distance = length(to_light);
        //    float3 light_dir = normalize(to_light);
        //    float diffuse_factor = 2.0 * max(dot(normal, light_dir), 0.0) / (distance * distance);
        //    diffuse += lights[i].color * diffuse_factor;
        //}
            
        float3 sun_dir = normalize(global_uniforms.sun_dir);
        float sun_diff = max(dot(normal, sun_dir), 0.0);
        float3 sun_color = float3(1.0, 0.9, 0.7);


        diffuse += sun_color * sun_diff;
        diffuse *= (float3) texture;

        return half4( (half3) diffuse, 1.0 );
    }
