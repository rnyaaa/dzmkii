#include <metal_stdlib>
using namespace metal;

struct v2f
{
    float4 position [[position]];
    float3 color;
    float2 uv;
};

struct LocalUniforms
{
    float4x4 model_matrix;
    bool textured;
    bool lit;
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

v2f vertex vertexMain (
        uint vertex_id [[ vertex_id ]],
        device const Vertex *vertices [[ buffer(1) ]],
        constant LocalUniforms &local_uniforms [[ buffer(2) ]]
    )
{
    v2f o;

    o.position = local_uniforms.model_matrix * float4(vertices[vertex_id].position, 1.0);
    o.color    = vertices[vertex_id].color;
    o.uv       = vertices[vertex_id].uv;

    return o;
};

half4 fragment fragmentMain( 
        v2f in [[stage_in]],
        constant LocalUniforms &local_uniforms [[ buffer(2) ]],
        texture2d<half> texture [[ texture(0) ]],
        sampler texture_sampler [[ sampler(0) ]]
    )
{
    half3 color;
    if (local_uniforms.textured)
    {
        color = texture.sample(texture_sampler, in.uv).xyz;
    }
    else
    {
        color = half3(in.color);
    }

    return half4(half3(color), 0.0);
}
