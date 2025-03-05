#ifndef _SUN_H
#define _SUN_H

#include <glm/glm.hpp>
#include "common.h"

struct Sun 
{
    glm::vec4 dir;
    glm::vec4 color;

    Sun(glm::vec3 dir);
};

#endif

