#define GLM_SWIZZLE
#include <glm/glm.hpp>

#ifndef _VERTEX_H
#define _VERTEX_H

struct Vertex
{
    glm::vec4 pos;
    glm::vec4 normal;
    glm::vec4 tangent;
    glm::vec4 bitangent;
    glm::vec4 color;
    glm::vec2 uv;
    char __padding0[8];

    Vertex();
    Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 color, glm::vec3 tangent, glm::vec3 bitangent);

    Vertex withPos(glm::vec3);
    Vertex withColor(glm::vec3);

    void calculateTangentAndBitangent();
};

#endif
