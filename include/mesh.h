#ifndef _MESH_H
#define _MESH_H

#include <vector>
#include <optional>
#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Metal/Metal.hpp>
#include <simd/simd.h>

#include "common.h"
#include "tile.h"
#include "vertex.h"

struct Mesh
{
    u32 num_vertices;
    MTL::Buffer *vertices_buffer;
    MTL::Buffer *colors_buffer;

    Mesh(
            MTL::Device *device, 
            std::vector<simd::float3> vertices, 
            std::vector<simd::float3> colors
        )
    {
        if (vertices.size() != colors.size())
        {
            Log::warning("No. of Vertices does not match no. of colors\
                         during mesh creation.");
        }

        num_vertices = vertices.size();

        vertices_buffer = device->newBuffer(
                vertices.size() * sizeof(simd::float3),
                MTL::ResourceStorageModeManaged
            );

        memcpy(
                vertices_buffer->contents(),
                vertices.data(),
                vertices.size() * sizeof(simd::float3)
            );

        vertices_buffer ->didModifyRange(
                NS::Range::Make(0, vertices_buffer->length())
            );

        colors_buffer = device->newBuffer(
                colors.size() * sizeof(simd::float3),
                MTL::ResourceStorageModeManaged
            );

        memcpy(
                colors_buffer->contents(),
                colors.data(),
                colors.size() * sizeof(simd::float3)
            );

        colors_buffer ->didModifyRange(
                NS::Range::Make(0, colors_buffer->length())
            );
    };

    Mesh gen_mesh_from_tiles(std::vector<Tile> tiles);
    static Mesh fromVertexData(std::vector<Vertex> vertices);
    static std::optional<std::vector<Mesh>> importMeshesFromFile(const std::string &path);
};

#endif // _MESH_H
