#include "camera.h"
#include "light.h"
#include "renderer.h"
#include "sun.h"
#include "terrain.h"
#include "boid.h"

#define NO_LIGHTS 200
#ifndef _SCENE_H
#define _SCENE_H

struct SkyboxUniforms
{
    glm::mat4 model_matrix;
};

struct SceneUniforms
{
    CameraData camera;
    glm::vec4 sun;
    glm::vec4 sun_color;
    s32 no_lights;
    float elapsed_time;
    float tod;
};

struct Scene
{
    Sun sun;
    Terrain terrain;
    Camera camera;

    float boid_mass_factor;
    float boid_match_factor;
    float boid_avoidance_factor;

    DZPipeline terrain_pipeline;
    DZPipeline skyscraper_pipeline;
    DZPipeline skybox_pipeline;
    DZPipeline boid_pipeline;

    DZBuffer scene_uniform_buffer;
    DZBuffer light_buffer;

    DZMesh skybox_mesh;
    DZBuffer skybox_buffer;
    Transform skybox_transform;

    int no_lights;
    f32 tod;
    f32 elapsed_time;
    DynamicPointLightData lights[NO_LIGHTS];
    std::vector<Boid*> boids;

    Scene(DZRenderer &renderer, DZPipeline terrain_pipeline, DZPipeline skyscraper_pipeline, DZPipeline skybox_pipeline, DZPipeline boid_pipeline);

    void render(DZRenderer &renderer, const glm::vec2 &screen_dim);
};

#endif // _SCENE_H
