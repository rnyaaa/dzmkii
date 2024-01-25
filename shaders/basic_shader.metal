#include <metal_stdlib>
using namespace metal;

struct v2f
{
    float4 position [[position]];
    half3 color;
};

struct CameraData
{
    float4x4 view_matrix;
    float4x4 projection_matrix;
};

v2f vertex vertexMain( 
        uint vertexId [[vertex_id]],
        device const float3* positions [[buffer(0)]],
        device const float3* colors [[buffer(1)]] ,
        constant CameraData &camera_data [[buffer(2)]]
    )
{
    v2f o;
    o.position = camera_data.projection_matrix * camera_data.view_matrix * float4( positions[ vertexId ], 1.0 );
    o.color = half3 ( colors[ vertexId ] );

    return o;
}

half4 fragment fragmentMain( 
        v2f in [[stage_in]]
    )
{
    return half4( in.color, 1.0 );
}
