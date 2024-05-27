#ifndef _LIGHT_H
#define _LIGHT_H
#define MAX_NO_LIGHTS 100

#include "transform.h"
#include "common.h"

struct PointLight{};
struct SpotLight{};
struct StaticLight{};

struct DirLight{};

struct DynamicPointLightData
{
    glm::vec3 pos;
    glm::vec3 color;
    f32 falloff;
};


#endif // _LIGHT_H
