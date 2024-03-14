#ifndef _SUN_H
#define _SUN_H

#include <glm/glm.hpp>

struct Sun 
{
    glm::vec4 dir;

    Sun(glm::vec3 dir);
    void Update();
};

#endif

