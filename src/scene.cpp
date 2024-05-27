#include "scene.h"
#include "light.h"
#include "renderer.h"
#include "model.h"

Scene::Scene(DZRenderer &renderer, DZPipeline terrain_pipeline, DZPipeline model_pipeline) 
    : terrain(renderer, 100.0f, 616u)
    , sun({1.0f, 1.0f, 1.0f})
    , terrain_pipeline(terrain_pipeline)
    , model_pipeline(model_pipeline)
{
    this->scene_uniform_buffer 
        = renderer.createBufferOfSize(sizeof(SceneUniforms));

    this->light_buffer 
        = renderer.createBufferOfSize(sizeof(DynamicPointLightData));
}

void Scene::render(DZRenderer &renderer, const glm::vec2 &screen_dim)
{
//   DynamicPointLightData light_data = { 
//       camera.target,
//       glm::vec3(1.0),
//       9.0f
//   };
//
//   renderer
//       .setBufferOfSize(light_buffer, &light_data, sizeof(DynamicPointLightData));
//
//   SceneUniforms uniforms = {
//       scene.camera.getCameraData(screen_dim),
//       sun.dir,
//       1
//   };
//
//   renderer.setBufferOfSize(scene_uniform_buffer, &uniforms, sizeof(SceneUniforms));
//
//   renderer.enqueueCommand(
//           DZRenderCommand::SetPipeline(terrain_pipeline));
//
//   renderer.enqueueCommand(
//           DZRenderCommand::BindBuffer(
//                   DZBufferBinding::Fragment(scene_uniform_buffer, 0)
//               ));
//
//   renderer.enqueueCommand(
//           DZRenderCommand::BindBuffer(
//                   DZBufferBinding::Vertex(scene_uniform_buffer, 0)
//               ));
//
//   renderer.enqueueCommand(
//           DZRenderCommand::BindBuffer(
//                   DZBufferBinding::Fragment(light_buffer, 3)
//               ));
//
//   // BEGIN TERRAIN RENDERING
//   // TODO: MORE SMART LESS DUMB
//   std::array<Chunk*, 9> visible;
//   for (int i = -1; i <= 1; i++)
//   {
//       for (int j = -1; j <= 1; j++)
//       {
//           visible[(j+1) * 3 + (i + 1)] 
//               = terrain.getChunkFromPos(
//                       v2f
//                       {
//                           camera.target.x + terrain.chunk_size * i,
//                           camera.target.y + terrain.chunk_size * j
//                       }
//                   );
//       }
//   }
//
//   terrain.updateUniforms(renderer, visible);
//
//   renderer.enqueueCommand(
//               DZRenderCommand::BindBuffer(
//                   DZBufferBinding::Fragment(
//                       terrain.terrain_uniform_buffer, 2)
//                   )
//           );
//       
//   for (int i = 0; i < 9; i++)
//   {
//       visible[i]->updateUniforms(renderer, i);
//       renderer.enqueueCommand(
//               DZRenderCommand::BindBuffer(
//                   DZBufferBinding::Vertex(
//                       visible[i]->local_uniforms_buffer, 2)));
//
//       renderer.enqueueCommand(
//               DZRenderCommand::BindBuffer(
//                   DZBufferBinding::Fragment(
//                       visible[i]->local_uniforms_buffer, 1)));
//
//       renderer.enqueueCommand(
//                DZRenderCommand::DrawMesh(
//                    visible[i]->mesh));
//   }
//
//   for (int i = 0; i < 9; i++)
//   {
//       for(auto &los_index : visible[i]->los_indices)
//       {
//           if(los_index == 2) los_index = 1;
//       }
//   }
//
//   // END TERRAIN RENDERING
//
//   renderer.enqueueCommand(DZRenderCommand::SetPipeline(model_pipeline));
//
//   registry
//       .view<Transform, Model>()
//       .each(
//               [&](const auto &transform, const auto &model)
//               {
//                   model.render(renderer, transform);
//               }
//           );
}
