#include "common.h"
#include "transform.h"
#include "mesh.h"
#include "renderer.h"
#include <glm/glm.hpp>
#include "light.h"
#ifndef _BOID_H
#define _BOID_H

struct BoidData
{
    glm::mat4 model_matrix;
    bool ischasing;
};

struct Boid 
{
    f32 damage = 1;
    f32 health = 10;

    glm::vec3 pos;
    glm::vec3 velocity;
    f32 scale;
    f32 boid_detection_range;
    f32 boid_avoidance_range;
    f32 target_detection_range;
    bool ischasing;
    DynamicPointLightData light;
    glm::vec3 color;
    glm::vec3 chasing_color;

    static bool mesh_generated;
    static MeshData mesh_data;
    static DZMesh mesh;
    static bool mesh_registered;

    bool has_uniforms_buffer = false;
    DZBuffer local_uniforms_buffer;


    Boid(glm::vec3 pos);
    void updateUniforms(DZRenderer &renderer);
};

#endif // _BOID_H
