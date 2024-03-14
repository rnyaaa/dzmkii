#include <glm/glm.hpp>

#ifndef _VERTEX_H
#define _VERTEX_H

struct Vertex
{
    glm::vec4 pos;
    glm::vec4 normal;
    glm::vec4 color;

    Vertex();
    Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 color);
};

#endif
