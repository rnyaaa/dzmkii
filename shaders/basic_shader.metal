#include <metal_stdlib>
using namespace metal;

struct v2f
{
    float4 position [[position]];
    float4 world_position;
    float3 normal;
    half3 color;
};

struct Vertex
{
    float3 position;
    float3 normal;
    float3 color;
};

struct CameraData
{
    float4x4 view_matrix;
    float4x4 projection_matrix;
};

struct Uniforms
{
    CameraData camera;
    float3 sun_dir;
};

v2f vertex vertexMain( 
        uint vertex_id [[ vertex_id ]],
        device const Vertex *vertices [[ buffer(0) ]],
        constant Uniforms &uniforms [[ buffer(1) ]]
    )
{
    v2f o;
    o.position = uniforms.camera.projection_matrix * uniforms.camera.view_matrix * float4( vertices[vertex_id].position, 1.0 );
    o.world_position = float4(vertices[vertex_id].position, 1.0);
    o.color = half3 ( vertices[vertex_id].color );
    o.normal = vertices[vertex_id].normal;
    return o;
}

half4 fragment fragmentMain( 
        v2f in [[stage_in]],
        constant Uniforms &uniforms [[ buffer(0) ]],
        texture2d<half> color_texture [[ texture(0) ]],
        sampler texture_sampler [[ sampler(0) ]]
    )
{
    const half3 texture = 
        half3(color_texture.sample(texture_sampler, in.world_position.xy / 8.0));

    float3 sun_dir = normalize(uniforms.sun_dir);
    float sun_diff = max(dot(in.normal, sun_dir), 0.0);
    half3 sun_color = half3(1.0, 0.9, 0.7);
    half3 diffuse = 1.0 * sun_color * sun_diff * texture + 0.0 * texture;

    return half4( diffuse, 1.0 );
}
