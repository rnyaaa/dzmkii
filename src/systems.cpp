#include <cmath>
#include "renderer.h"
#include <algorithm>
#define GLM_FORCE_SWIZZLE
#include "glm/geometric.hpp"
#include "input.h"
#include "logger.h"
#include <MacTypes.h>
#include <SDL_scancode.h>
#include "systems.h"
#include "model.h"
#include "SDL_keycode.h"
#include "camera.h"

void GameSystem::inputActions(GAMESYSTEM_ARGS)
{
   //if(input.key[DZKey::L] && !input.key_prev[DZKey::L])
   //{
   //}
}

void GameSystem::updatePlayer(GAMESYSTEM_ARGS)
{
}

void RenderSystem::updateData(RENDERSYSTEM_ARGS)
{
    renderer.enqueueCommand(DZRenderCommand::SetPipeline(scene.sand_pipeline));

    SceneUniforms uniforms = {
        scene.camera.getCameraData(screen_dim),
    };

    // Log the matrices
    auto cam_data = scene.camera.getCameraData(screen_dim);

    renderer.setBufferOfSize(scene.scene_uniform_buffer, &uniforms, sizeof(SceneUniforms));
    


    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Fragment(scene.scene_uniform_buffer, 0)
                ));

    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Vertex(scene.scene_uniform_buffer, 0)
                ));   
    
    Transform screen_transform;
    screen_transform.pos = glm::vec3(0.0f, 0.0f, 0.0f);
    screen_transform.scale = glm::vec3(screen_dim.x, screen_dim.y, 1.0f);
    
    scene.screen_model.render(renderer, screen_transform);
    
}
