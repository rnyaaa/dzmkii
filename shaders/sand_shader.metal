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
    float4 position;              
    float4x4 view_matrix;
    float4x4 projection_matrix;
};

struct GlobalUniforms
{
    CameraData camera;
    float elapsed_time;
};

struct ModelUniforms
{
    float4x4 model_matrix;
    bool textured;                
    bool lit;
    uint material_index;          
};

vertex v2f vertexMain( 
        uint vertex_id [[ vertex_id ]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        device const Vertex *vertices [[ buffer(1) ]],
        constant ModelUniforms &local_uniforms [[ buffer(2) ]]
    )
{
    v2f o;
    o.local_position = vertices[vertex_id].position;
    o.world_position = local_uniforms.model_matrix * vertices[vertex_id].position;
    o.position = global_uniforms.camera.projection_matrix * global_uniforms.camera.view_matrix * o.world_position;
    return o;
}

fragment half4 fragmentMain( 
        v2f in [[stage_in]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        constant ModelUniforms &local_uniforms [[ buffer(1) ]]
    )
{
    return half4(1.0, 0.0, 0.0, 1.0);
}
