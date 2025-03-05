#include <glm/glm.hpp>
#include "camera.h"
#include "light.h"
#include "geometry.h"

#ifndef _PLAYER_H
struct Player
{
    circ2f bounding_circle;
    f64 distance_travelled;
    glm::vec3 pos;
    glm::vec3 velocity;
    Camera camera;
    int boost;

    Player(glm::vec3 pos, Camera &camera)
    {
        this->pos = pos;
        this->camera = camera;
        this->boost = 200;
    };
};


#define _PLAYER_H
#endif
