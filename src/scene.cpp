#include "scene.h"
#include "light.h"
#include "renderer.h"
#include "model.h"

Scene::Scene(DZRenderer &renderer, DZPipeline terrain_pipeline, DZPipeline model_pipeline, DZPipeline gui_pipeline, DZPipeline fow_pipeline) 
    : terrain(renderer, 100.0f, 616u)
    , sun({1.0f, 1.0f, 1.0f})
    , terrain_pipeline(terrain_pipeline)
    , model_pipeline(model_pipeline)
    , gui_pipeline(gui_pipeline)
    , fow_pipeline(fow_pipeline)
{

    this->LOS_ON = 1;
    this->debug_texture = 0;

    this->scene_uniform_buffer 
        = renderer.createBufferOfSize(sizeof(SceneUniforms));

    this->light_buffer 
        = renderer.createBufferOfSize(sizeof(DynamicPointLightData));
}
