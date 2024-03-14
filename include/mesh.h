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

#ifndef _MESH_H
#define _MESH_H

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<u32> indices;

    static MeshData genMeshFromTiles(std::vector<Tile> tiles);
    //static std::optional<std::vector<MeshData>> importMeshesFromFile(const std::string &path);
};

#endif // _MESH_H
