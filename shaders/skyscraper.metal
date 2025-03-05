#include <metal_stdlib>
#include "headers.hm"
#include "lighting.hm"
#include "noise.hm"
#include "fog.hm"

using namespace metal;

struct SkyscraperUniforms
{
    float4x4 model_matrix;
    uint32_t foo;
    uint8_t texture;
};

v2f vertex skyscraper_vertexMain( 
        uint vertex_id [[ vertex_id ]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        device const Vertex *vertices [[ buffer(1) ]],
        constant SkyscraperUniforms  &local_uniforms  [[ buffer(2) ]]
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
    
    o.texture = local_uniforms.texture; 
    return o;
};

half4 fragment skyscraper_fragmentMain( 
        v2f in [[stage_in]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        constant SkyscraperUniforms &local_uniforms [[ buffer(1) ]],
        constant PointLight *lights [[ buffer(3) ]],
        texture2d_array<half> terrain_textures [[ texture(0) ]],
        sampler texture_sampler [[ sampler(0) ]]
    )
{    
    half3 material = half3(terrain_textures.sample(texture_sampler, float2(in.uv.x, in.uv.y * 10), in.texture + (7 * 3)));

    float3x3 TBN = float3x3(in.T, in.B, in.N);
    float3x3 to_tangent = transpose(TBN);

    Material skyscraper_material;
    skyscraper_material.color             = float3(material);
    skyscraper_material.roughness         = .025f; 
    skyscraper_material.subsurface        = .2f;
    skyscraper_material.sheen             = .0f; 
    skyscraper_material.sheen_tint        = float3(1.f, 0.1f, 0.1f);
    skyscraper_material.anisotropic       = .3f;
    skyscraper_material.specular_strength = .0f;
    skyscraper_material.specular_tint     = float3(.0f, .0f, .0f);
    skyscraper_material.metallic          = .0f;
    skyscraper_material.clearcoat         = .0f;
    skyscraper_material.clearcoat_gloss   = .9f;
    skyscraper_material.ior               = .0f;
    skyscraper_material.relative_ior      = 1.25f;
    skyscraper_material.flatness          = 0.f;

    float3 viewDir = normalize(global_uniforms.camera.position.xyz - in.world_position.xyz);
    if(length(material) < .05f)
    {
        skyscraper_material.color = skybox_reflection(TBN[2], viewDir, global_uniforms.tod, global_uniforms.u_time);
        skyscraper_material.specular_strength = .5f;
        skyscraper_material.specular_tint = float3(.1f, .8f, .1f);
        skyscraper_material.clearcoat = .5f;
        skyscraper_material.metallic = .2f;
    }
   
    Sun sun;
    sun.sun_dir   = global_uniforms.sun_dir;
    sun.sun_color = global_uniforms.sun_color;
    float3 reflection = do_lighting(skyscraper_material, to_tangent, in.world_position.xyz, global_uniforms.camera.position.xyz, sun, &lights[MAX_NO_LIGHTS]);

    float3 fogColor = do_fog(global_uniforms.tod);

    float fogEnd = 100.0;
    float fogStart = 5.0;
    float depth = in.position.z / in.position.w;
    float fogFactor = clamp((fogEnd - depth) / (fogEnd - fogStart), 0.0, 1.0);

    return half4(half3(mix(fogColor, reflection, fogFactor)), 1.0);
}; 


