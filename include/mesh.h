#include <vector>

#include <Metal/Metal.hpp>
#include <simd/simd.h>

#define GLM_FORCE_SWIZZLE
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/constants.hpp>

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
        auto vert = 
            Vertex()
                .withNormal(glm::vec3(0.0f, 0.0f, 1.0f));

        return {
            {
                vert.withPos(glm::vec3(0.0f, 0.0f, 0.0f))
                    .withUV(glm::vec2(0.0, 0.0)),
                vert.withPos(glm::vec3(1.0f, 0.0f, 0.0f))
                    .withUV(glm::vec2(1.0, 0.0)),
                vert.withPos(glm::vec3(1.0f, 1.0f, 0.0f))
                    .withUV(glm::vec2(1.0, 1.0)),
                vert.withPos(glm::vec3(0.0f, 1.0f, 0.0f))
                    .withUV(glm::vec2(0.0, 1.0)),
            },
            { 0, 1, 2, 0, 3, 2 },
            PrimitiveType::TRIANGLE
        };
    }



    static MeshData UnitSquare()
    {
        // TODO: NORMALS
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

    static MeshData UnitSphere()
    {
        int divs = 24;
        std::vector<Vertex> vertices;
        for(int i = 0; i < divs; i++)
        {
            glm::vec3 p1, p2, p3, p4;
            f32 z1, z2;
            p1.z = -1.0 + 2 * (i / (f32) divs);
            p2.z = -1.0 + 2 * ((i+1) / (f32) divs);
            p3.z = -1.0 + 2 * (i / (f32) divs);
            p4.z = -1.0 + 2 * ((i+1) / (f32) divs);
            
            for(int j = 0; j < divs; j++)
            {
                f32 theta1, theta2;
                theta1 = glm::sqrt(1.0 - glm::pow(2.0 * i / (f32) divs - 1.0, 2.0));
                theta2 = glm::sqrt(1.0 - glm::pow(2.0 * (i+1) / (f32) divs - 1.0, 2.0));
                
                // Calculate positions
                p1.x = theta1 * sin(j * 2.0 * glm::pi<f32>() / divs);
                p1.y = theta1 * cos(j * 2.0 * glm::pi<f32>() / divs);
                p2.x = theta2 * sin(j * 2.0 * glm::pi<f32>() / divs);
                p2.y = theta2 * cos(j * 2.0 * glm::pi<f32>() / divs);
                p3.x = theta1 * sin((j+1) * 2.0 * glm::pi<f32>() / divs);
                p3.y = theta1 * cos((j+1) * 2.0 * glm::pi<f32>() / divs);
                p4.x = theta2 * sin((j+1) * 2.0 * glm::pi<f32>() / divs);
                p4.y = theta2 * cos((j+1) * 2.0 * glm::pi<f32>() / divs);

                // Calculate tangents and bitangents for each vertex
                // For a sphere, tangent points along the parallels (around the sphere)
                // and bitangent points along the meridians (up/down)
                
                // For p1
                glm::vec3 tangent1 = glm::normalize(glm::vec3(
                    -sin((j * 2.0f * glm::pi<f32>() / divs) + glm::pi<f32>() / 2.0f),
                    -cos((j * 2.0f * glm::pi<f32>() / divs) + glm::pi<f32>() / 2.0f),
                    0.0f
                ));
                glm::vec3 bitangent1 = glm::normalize(glm::cross(p1, tangent1));

                // For p2
                glm::vec3 tangent2 = glm::normalize(glm::vec3(
                    -sin((j * 2.0f * glm::pi<f32>() / divs) + glm::pi<f32>() / 2.0f),
                    -cos((j * 2.0f * glm::pi<f32>() / divs) + glm::pi<f32>() / 2.0f),
                    0.0f
                ));
                glm::vec3 bitangent2 = glm::normalize(glm::cross(p2, tangent2));

                // For p3
                glm::vec3 tangent3 = glm::normalize(glm::vec3(
                    -sin(((j+1) * 2.0f * glm::pi<f32>() / divs) + glm::pi<f32>() / 2.0f),
                    -cos(((j+1) * 2.0f * glm::pi<f32>() / divs) + glm::pi<f32>() / 2.0f),
                    0.0f
                ));
                glm::vec3 bitangent3 = glm::normalize(glm::cross(p3, tangent3));

                // For p4
                glm::vec3 tangent4 = glm::normalize(glm::vec3(
                    -sin(((j+1) * 2.0f * glm::pi<f32>() / divs) + glm::pi<f32>() / 2.0f),
                    -cos(((j+1) * 2.0f * glm::pi<f32>() / divs) + glm::pi<f32>() / 2.0f),
                    0.0f
                ));
                glm::vec3 bitangent4 = glm::normalize(glm::cross(p4, tangent4));

                vertices.push_back(Vertex().withPos(p1).withNormal(p1).withTangent(tangent1).withBitangent(bitangent1));
                vertices.push_back(Vertex().withPos(p2).withNormal(p2).withTangent(tangent2).withBitangent(bitangent2));
                vertices.push_back(Vertex().withPos(p3).withNormal(p3).withTangent(tangent3).withBitangent(bitangent3));
                vertices.push_back(Vertex().withPos(p2).withNormal(p2).withTangent(tangent2).withBitangent(bitangent2));
                vertices.push_back(Vertex().withPos(p4).withNormal(p4).withTangent(tangent4).withBitangent(bitangent4));
                vertices.push_back(Vertex().withPos(p3).withNormal(p3).withTangent(tangent3).withBitangent(bitangent3));
            }
        }
        return {
            vertices,
            {},
            PrimitiveType::TRIANGLE
        };
}

    static MeshData UnitCube()
    {
        std::vector<Vertex> vertices(36);

        const glm::vec3 UP(0.f, 0.f, 1.f);
        const glm::vec3 RIGHT(1.f, 0.f, 0.f);
        const glm::vec3 FORWARD(0.f, 1.f, 0.f);

        const f32 PI = glm::pi<f32>();
        const f32 HALF_PI = glm::half_pi<f32>();

        vertices[0] = Vertex()
            .withPos(glm::vec3(.5f, .5f, .5f))
            .withUV({1.f, 0.f})
            .withNormal(UP)
            .withTangent(RIGHT)
            .withBitangent(FORWARD);

        vertices[1] = vertices[0]
            .withPos(glm::vec3(-.5f, .5f, .5f))
            .withUV({0.f, 0.f});

        vertices[2] = vertices[0]
            .withPos(glm::vec3(.5f, -.5f, .5f))
            .withUV({1.f, 1.f});

        vertices[3] = vertices[0].rotated(UP, PI)
            .withUV({0.f, 1.f});
        vertices[4] = vertices[1].rotated(UP, PI)
            .withUV({1.f, 1.f});
        vertices[5] = vertices[2].rotated(UP, PI)
            .withUV({0.f, 0.f});

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 6; j++)
                vertices[6 + j + i * 6] = vertices[j + i * 6].rotated(RIGHT, HALF_PI);

        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 6; j++)
                vertices[24 + i * 6 + j] = vertices[6 + j].rotated(UP, -HALF_PI + i*PI);

        return {
            vertices,
            {},
            PrimitiveType::TRIANGLE
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
