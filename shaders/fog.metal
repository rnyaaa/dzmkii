#include <metal_stdlib>
#include "noise.hm"
#include "headers.hm"
using namespace metal;

v2f vertex skybox_vertexMain( 
        uint vertex_id [[ vertex_id ]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        device const Vertex *vertices [[ buffer(1) ]]
    )
{
    v2f o;
    o.local_position = float4(vertices[vertex_id].position, 1.0);
    o.world_position = o.local_position;
    float4x4 projection_matrix = global_uniforms.camera.projection_matrix;

    projection_matrix[3][0] = 0.0;
    projection_matrix[3][1] = 0.0;
    projection_matrix[3][2] = 0.0;
    projection_matrix[3][3] = 1.0;  // Preserve the homogeneous coordinate

    float4x4 view_matrix = global_uniforms.camera.view_matrix;

    view_matrix[3][0] = 0.0;
    view_matrix[3][1] = 0.0;
    view_matrix[3][2] = 0.0;
    view_matrix[3][3] = 1.0;  // Preserve the homogeneous coordinate

    o.position = 
        global_uniforms.camera.projection_matrix * view_matrix * o.world_position;
    o.color = half3 ( vertices[vertex_id].color );
    o.uv = vertices[vertex_id].uv;
    o.T  = vertices[vertex_id].tangent.xyz;
    o.B  = vertices[vertex_id].bitangent.xyz;
    o.N  = vertices[vertex_id].normal.xyz;
    
    return o;
};

half4 fragment skybox_fragmentMain( 
        v2f in [[stage_in]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]]
    )
{

    float3 night = {0.01, 0.015, 0.05};
    float3 day = {0.1, 0.15, 0.4};
    float3 dawndusk = {100.0/255.0, 30.0/255.0, 30.0/255.0};

    float3 skycolor = {0.0};
    if(global_uniforms.tod < 0)
    {
        skycolor = mix(night, dawndusk, (global_uniforms.tod + 1.0));
    } else {
        skycolor = mix(dawndusk, day, global_uniforms.tod);
    }
    float tod = (global_uniforms.tod + 1.0)/2.0;
    float2 pos = (in.world_position.xy) * 5.0;
    float3 fogColor = {0.0};
    float2 q = {0.0};
    q.x = fbm(pos + 0.0 * global_uniforms.u_time);
    q.y = fbm(pos + float2(1.0, 1.0));

    float2 r = {0.0};
    r.x = fbm(pos + 1.0*q + float2(1.7, 9.2) + 0.15*global_uniforms.u_time);
    r.y = fbm(pos + 1.0*q + float2(8.3, 2.8) + 0.126*global_uniforms.u_time);

    float f = fbm(pos + r);

    fogColor = mix(float3(0.101961/64*tod, 0.619608/64*tod, 0.666667/64*tod),
                float3(0.666667/32*tod, 0.666667/32*tod, 0.498039/32*tod),
                clamp((f*f)*4.0, 0.0, 1.0));

    fogColor = mix(fogColor, 
                skycolor,
                clamp(length(q), 0.0, 1.0));

    fogColor = mix(fogColor,
                float3(0.666667/8*tod, 1.0/8*tod, 1.0/8*tod),
                clamp(abs(r.x), 0.0, tod));
    // Step 4: Mix the scene color with the fog color based on the fog factor

    return half4((half3) fogColor, 1.0 );
};
