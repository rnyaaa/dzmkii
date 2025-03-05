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

    DynamicPointLightData(glm::vec3 pos, glm::vec3 color, f32 falloff)
    {
        this->pos = pos;
        this->color = color;
        this->falloff = falloff;
    }
    DynamicPointLightData()
    {
        this->pos = glm::vec3(0.0f, 0.0f, 0.0f);
        this->color = glm::vec3(0.0f, 0.0f, 0.0f);
        this->falloff = 0.0f;
    }
};


#endif // _LIGHT_H
