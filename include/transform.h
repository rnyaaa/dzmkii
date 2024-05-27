#include <glm/glm.hpp>
#include "glm/ext/matrix_transform.hpp"

#ifndef _TRANSFORM_H
#define _TRANSFORM_H

struct Transform
{
    glm::vec3 pos      = glm::vec3(0.0);
    glm::vec3 scale    = glm::vec3(1.0);
    glm::vec3 rotation = glm::vec3(0.0);

    glm::mat4 asMat4() const
    {
        glm::mat4 mat = glm::mat4(1.0f);

        mat = glm::translate(mat, this->pos);
        mat = glm::scale(mat, this->scale);
        mat = glm::rotate(mat, rotation.z, glm::vec3(0, 0, 1));
        mat = glm::rotate(mat, rotation.y, glm::vec3(0, 1, 0));
        mat = glm::rotate(mat, rotation.x, glm::vec3(1, 0, 0));

        return mat;
    }
};

#endif
