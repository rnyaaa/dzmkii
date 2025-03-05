#include "renderer.h"

#ifndef _MATERIAL_H
#define _MATERIAL_H

struct TerrainMaterial
{

};

struct Material
{
    u32 asset_handle;
// Base
    DZTexture albedo; 
    DZTexture normal;
    DZTexture displacement;
    DZTexture roughness;

//   v4f color;
//
//   f32 metallic;
//   f32 anisotropy;
//   f32 anisotropy_rotation;
//   f32 translucency;
//   f32 transparency;
//   f32 cut_out_opacity;
//   f32 specular;
//   v4f specular_tint;
//
/// Sheen
//   //  sheen?
//   v4f sheen_color;
//   f32 sheen_roughness;
//
/// Flakes
//   f32 flake_coverage;
//   v4f flake_color;
//   f32 flake_size;
//   f32 flake_roughness;
//
/// Coating 
//   f32 clearcoat;
//   f32 clearcoat_roughness;
//   // clearcoat normal map?
//
/// Emission
//   v4f emission_color;
//   f32 emission_value;
//   // emission mode?
//   // energy normalization?
//
/// Volume 
//   // Thin walled?
//   // Index of refraction?
//   // Attenuation color?
//   // Attenuation distance?
//   v4f subsurface_color;
//   f32 subsurface_anisotropy;
};

#endif // _MATERIAL_H
