#include "scene.h"
#include "renderer.h"
#include "model.h"

Scene::Scene(DZRenderer &renderer, DZPipeline sand_pipeline) 
    : camera()
    , sand_pipeline(sand_pipeline)
    , tod(0.0)
    , elapsed_time(0.0)
{
    this->scene_uniform_buffer 
        = renderer.createBufferOfSize(sizeof(SceneUniforms));
    
    // Initialize screen model
    auto plane = MeshData::UnitPlane();
    
    Log::verbose("=== UnitPlane vertices ===");
    for (size_t i = 0; i < plane.vertices.size(); i++) {
        auto& v = plane.vertices[i];
        Log::verbose("Vertex %zu: pos(%f, %f, %f, %f)", 
                     i, v.pos.x, v.pos.y, v.pos.z, v.pos.w);
    }
    Log::verbose("=== UnitPlane indices ===");
    for (size_t i = 0; i < plane.indices.size(); i++) {
        Log::verbose("Index %zu: %u", i, plane.indices[i]);
    }
    
    std::vector<MeshData> screen_mesh = {plane};
    this->screen_model = Model::fromMeshDatas(renderer, screen_mesh);
    
    Log::verbose("Scene created. screen_model has %zu meshes", this->screen_model.meshes.size());
}
