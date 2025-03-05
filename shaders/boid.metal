#include <metal_stdlib>
#include "lighting.hm"
#include "noise.hm"
#include "headers.hm"
#include "fog.hm"

using namespace metal;

struct BoidUniforms
{
    float4x4 model_matrix;
    bool ischasing;
};

v2f vertex boid_vertexMain( 
        uint vertex_id [[ vertex_id ]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        device const Vertex *vertices [[ buffer(1) ]],
        constant BoidUniforms  &local_uniforms  [[ buffer(2) ]]
    )
{
    v2f o;
    o.local_position = float4(vertices[vertex_id].position, 1.0);
    o.world_position = local_uniforms.model_matrix * float4(vertices[vertex_id].position, 1.0);
    o.position = global_uniforms.camera.projection_matrix * global_uniforms.camera.view_matrix * o.world_position;
    o.color = half3 ( vertices[vertex_id].color );
    o.uv = vertices[vertex_id].uv;
    o.T = vertices[vertex_id].tangent.xyz;
    o.B = vertices[vertex_id].bitangent.xyz;
    o.N = vertices[vertex_id].normal.xyz;
    
    return o;
};

half4 fragment boid_fragmentMain( 
        v2f in [[stage_in]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        constant BoidUniforms &local_uniforms [[ buffer(1) ]],
        constant PointLight *lights [[ buffer(3) ]]
    )
{
    float3 color = {0.0, 0.0, 0.0};
    if(local_uniforms.ischasing){
        color = {0.096, 0.0012, 0.012}; 
    } else {
        color = {0.024, 0.0024, 0.024};
    }

    color *= 2.f;

    float3x3 TBN = float3x3(in.T, in.B, in.N);
    float3x3 to_tangent = transpose(TBN);

    Material boid_material;
    boid_material.color             = color;// ambient;
    boid_material.roughness         = .2f; 
    boid_material.subsurface        = 0.0f;
    boid_material.sheen             = .0f; 
    boid_material.sheen_tint        = float3(1.f, 0.1f, 0.1f);
    boid_material.anisotropic       = .2f;
    boid_material.specular_strength = .5f;
    boid_material.specular_tint     = float3(1.f, 1.f, 1.f);
    boid_material.metallic          = 0.0f;
    boid_material.clearcoat         = 1.f;
    boid_material.clearcoat_gloss   = .9f;
    boid_material.ior               = .2f;
    boid_material.relative_ior      = 2.5f;
    boid_material.flatness          = 0.f;
    //boid_material.to_tangent        = to_tangent;
   
    Sun sun;
    sun.sun_dir   = global_uniforms.sun_dir;
    sun.sun_color = global_uniforms.sun_color;
    float3 reflection = do_lighting(boid_material, to_tangent, in.world_position.xyz, global_uniforms.camera.position.xyz, sun, &lights[MAX_NO_LIGHTS]);

    float3 fogColor = do_fog(global_uniforms.tod);

    float fogEnd = 100.0;
    float fogStart = 5.0;
    float depth = in.position.z / in.position.w;
    float fogFactor = clamp((fogEnd - depth) / (fogEnd - fogStart), 0.0, 1.0);

    return half4(half3(mix(fogColor, reflection, fogFactor)), 1.0);
};

