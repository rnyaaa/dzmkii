#include "camera.h"
#include "renderer.h"
#include "model.h"

#ifndef _SCENE_H
#define _SCENE_H


struct SceneUniforms
{
    CameraData camera;
    float elapsed_time;
};

struct Scene
{
    Camera camera;

    DZPipeline sand_pipeline;

    DZBuffer scene_uniform_buffer;

    Model screen_model;

    f32 tod;
    f32 elapsed_time;

    Scene(DZRenderer &renderer, DZPipeline sand_pipeline);

    void render(DZRenderer &renderer, const glm::vec2 &screen_dim);
};

#endif // _SCENE_H
