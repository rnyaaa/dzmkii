#include <metal_stdlib>
using namespace metal;

struct v2f
{
    float4 position [[position]];
    float4 world_position;
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
    o.world_position = local_uniforms.model_matrix * float4(vertices[vertex_id].position, 1.0);

    o.position = global_uniforms.camera.projection_matrix * global_uniforms.camera.view_matrix * o.world_position;

    o.color = half3 ( vertices[vertex_id].color );

    o.T = vertices[vertex_id].tangent.xyz;
    o.B = vertices[vertex_id].bitangent.xyz;
    o.N = vertices[vertex_id].normal.xyz;

    return o;
};

half4 fragment fragmentMain( 
        v2f in [[stage_in]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        constant ModelUniforms &local_uniforms [[ buffer(1) ]],
        texture2d_array<half> terrain_textures [[ texture(0) ]],
        sampler texture_sampler [[ sampler(0) ]]
    )
{
    return half4(0.5, 0.5, 0.5, 1.0 );
}
