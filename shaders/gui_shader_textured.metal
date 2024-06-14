#include <metal_stdlib>
using namespace metal;

struct v2f
{
    float4 position [[position]];
    float3 color;
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
    o.color = vertices[vertex_id].color;

    return o;
};

half4 fragment fragmentMain( 
        v2f in [[stage_in]]
    )
{
    return half4(half3(in.color), 1.0);
}
