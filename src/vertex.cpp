#include "vertex.h"

Vertex::Vertex()
{
}

Vertex::Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 color)
{
    this->normal= glm::vec4(normal, 0.0f);
    this->pos   = glm::vec4(pos, 0.0f);
    this->color = glm::vec4(color, 0.0f);
}
