#define GLM_SWIZZLE
#include "vertex.h"
#include "glm/geometric.hpp"
Vertex::Vertex()
{
}

Vertex::Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 color, glm::vec3 tangent, glm::vec3 bitangent)
{
    this->normal    = glm::vec4(normal, 0.0f);
    this->tangent   = glm::vec4(tangent, 0.0f);
    this->bitangent = glm::vec4(bitangent, 0.0f);
    this->pos       = glm::vec4(pos, 1.0f);
    this->color     = glm::vec4(color, 0.0f);
}

void Vertex::calculateTangentAndBitangent()
{
    glm::vec3 _normal = this->normal.xyz();
    glm::vec3 c1 = glm::cross(_normal, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec3 c2 = glm::cross(_normal, glm::vec3(0.0f, 1.0f, 0.0f));
    
    if(glm::length(c1) > glm::length(c2))
    {
        this->tangent = glm::vec4(c1, 0.0f);
    } 
    else 
    {
        this->tangent = glm::vec4(c2, 0.0f);
    }

    this->tangent = glm::normalize(this->tangent);

    glm::vec3 _tangent = this->tangent.xyz();
    this->bitangent = glm::vec4(glm::cross(_tangent, _normal), 0.0f);
}

Vertex Vertex::withPos(glm::vec3 pos)
{
    Vertex new_vert = *this;
    new_vert.pos = glm::vec4(pos, 1.0f);
    return new_vert;
}

Vertex Vertex::withColor(glm::vec3 color)
{
    Vertex new_vert = *this;
    new_vert.color = glm::vec4(color, 1.0f);
    return new_vert;
}

