#include "scene.h"
#include "light.h"
#include "renderer.h"
#include "model.h"

Scene::Scene(DZRenderer &renderer, DZPipeline terrain_pipeline, DZPipeline skyscraper_pipeline, DZPipeline skybox_pipeline, DZPipeline boid_pipeline) 
    : sun({1.0f, 1.0f, 1.0f})
    , terrain(renderer, 100.0f, 616u)
    , camera()
    , terrain_pipeline(terrain_pipeline)
    , skyscraper_pipeline(skyscraper_pipeline)
    , skybox_pipeline(skybox_pipeline)
    , boid_pipeline(boid_pipeline)
    , skybox_transform()
    , tod(0.0)
    , elapsed_time(0.0)
    //, lights()
    //, boids()
{
    MeshData skyboxmeshdata = MeshData::UnitSphere();

    this->skybox_mesh = renderer.createMesh(skyboxmeshdata);
    this->skybox_buffer = renderer.createBufferOfSize(sizeof(SkyboxUniforms));

    this->scene_uniform_buffer 
        = renderer.createBufferOfSize(sizeof(SceneUniforms));

    this->light_buffer 
        = renderer.createBufferOfSize(sizeof(this->lights));
    this->boid_avoidance_factor = 0.1f;
    this->boid_mass_factor = 0.0075f;
    this->boid_match_factor = 0.03f;
}
