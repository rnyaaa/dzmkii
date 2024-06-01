#include <vector>
#include <optional>
#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Metal/Metal.hpp>
#include <simd/simd.h>

#include "common.h"
#include "vertex.h"

#ifndef _MESH_H
#define _MESH_H

enum class PrimitiveType
{
    POINT,
    LINE,
    LINE_STRIP,
    TRIANGLE,
    TRIANGLE_STRIP
};

struct MeshData
{
    std::vector<Vertex> vertices;
    // Could be reasonably be u16, need to change index type in renderer
    std::vector<u32> indices;
    PrimitiveType primitive_type;

    static MeshData UnitPlane()
    {
        return {
            {
                Vertex().withPos(glm::vec3(0.0f, 0.0f, 0.0f)),
                Vertex().withPos(glm::vec3(1.0f, 0.0f, 0.0f)),
                Vertex().withPos(glm::vec3(1.0f, 1.0f, 0.0f)),
                Vertex().withPos(glm::vec3(0.0f, 1.0f, 0.0f)),
            },
            { 0, 1, 2, 0, 3, 2 },
            PrimitiveType::TRIANGLE
        };
    }



    static MeshData UnitSquare()
    {
        return {
            { 
                Vertex()
                    .withPos(glm::vec3(0.0, 0.0, 0.0))
                    .withColor(glm::vec3(1.0, 1.0, 1.0)),
                Vertex()
                    .withPos(glm::vec3(1.0, 0.0, 0.0))
                    .withColor(glm::vec3(1.0, 1.0, 1.0)),
                Vertex()
                    .withPos(glm::vec3(1.0, 1.0, 0.0))
                    .withColor(glm::vec3(1.0, 1.0, 1.0)),
                Vertex()
                    .withPos(glm::vec3(0.0, 1.0, 0.0))
                    .withColor(glm::vec3(1.0, 1.0, 1.0)),
            },
            { 0, 1, 2, 3, 0 },
            PrimitiveType::LINE_STRIP
        };
    }

    void translate(glm::vec3 translation)
    {
        for (auto &v : this->vertices)
        {
            v.pos += glm::vec4(translation, 0.0);
        }
    }
};

#endif // _MESH_H
