#include <entt.hpp>

#include "camera.h"
#include "renderer.h"
#include "sun.h"
#include "terrain.h"

#ifndef _SCENE_H
#define _SCENE_H

struct SceneUniforms
{
    CameraData camera;
    glm::vec4 sun;
    s32 no_lights;
};

struct Scene
{
    entt::registry registry;

    Sun sun;
    Terrain terrain;
    Camera camera;

    DZPipeline terrain_pipeline;
    DZPipeline model_pipeline;

    DZBuffer scene_uniform_buffer;
    DZBuffer light_buffer;

    Scene(DZRenderer &renderer, DZPipeline terrain_pipeline, DZPipeline model_pipeline);

    void render(DZRenderer &renderer, const glm::vec2 &screen_dim);
};

#endif // _SCENE_H
