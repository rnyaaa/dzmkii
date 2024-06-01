#define GLM_SWIZZLE
#include "systems.h"
#include "model.h"
#include "SDL_keycode.h"
#include "light.h"
#include "movement.h"

void GameSystem::cameraMovement(GAMESYSTEM_ARGS)
{
    if(scene.camera.ortho)
    {
        if(key_state[SDL_SCANCODE_W])
            scene.camera.move(glm::vec3(-1.0f, -1.0f, 0.0f));
        if(key_state[SDL_SCANCODE_A])
            scene.camera.move(glm::vec3(1.0f, -1.0f, 0.0f));
        if(key_state[SDL_SCANCODE_S])
            scene.camera.move(glm::vec3(1.0f, 1.0f, 0.0f));
        if(key_state[SDL_SCANCODE_D])
            scene.camera.move(glm::vec3(-1.0f, 1.0f, 0.0f));
    }
    else 
    {
        if(key_state[SDL_SCANCODE_W])
            scene.camera.rotateWithOrigin(v2f{1.f, 0.f});
        if(key_state[SDL_SCANCODE_A])
            scene.camera.rotateWithOrigin(v2f{0.f, -1.f});
        if(key_state[SDL_SCANCODE_S])
            scene.camera.rotateWithOrigin(v2f{-1.f, 0.f});
        if(key_state[SDL_SCANCODE_D])
            scene.camera.rotateWithOrigin(v2f{0.f, 1.f});
    }
}

void GameSystem::unitMovement(GAMESYSTEM_ARGS)
{
    scene.registry
        .view<Transform, MoveSpeed>()
        .each(
                [&](auto &transform, auto &move_speed)
                {
                    updateMovement(
                            move_speed.speed * delta_time, 
                            transform, 
                            v3f
                            {
                                scene.camera.target.x, 
                                scene.camera.target.y, 
                                scene.camera.target.z
                            }, 
                            616u, 
                            scene.terrain
                        );
                }
            );

}

void GameSystem::LOS(GAMESYSTEM_ARGS)
{    
    for (int i = 0; i < 9; i++)
    {
        for(auto &los_index : scene.terrain.visible[i]->los_indices)
        {
            if(los_index == 2) los_index = 1;
        }
    }

    scene.registry
        .view<Transform, LineOfSight>()
        .each(
                [&](auto &transform, auto &los)
                {
                    scene.terrain.updateLOS(transform.pos.xy(), los.los); 
                }
            );
}

void GameSystem::terrainGeneration(GAMESYSTEM_ARGS)
{
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                scene.terrain.createChunk(renderer, scene.camera.target.xy() + glm::vec2(i * 100.0f, j * 100.0f));
            }
        }
        scene.terrain.getVisible(scene.camera);
}

void RenderSystem::updateData(RENDERSYSTEM_ARGS)
{

    DynamicPointLightData light_data = { 
        scene.camera.target,
        glm::vec3(1.0),
        9.0f
    };

    renderer
        .setBufferOfSize(scene.light_buffer, &light_data, sizeof(DynamicPointLightData));

    SceneUniforms uniforms = {
        scene.camera.getCameraData(screen_dim),
        scene.sun.dir,
        1
    };

    renderer.setBufferOfSize(scene.scene_uniform_buffer, &uniforms, sizeof(SceneUniforms));

    scene.terrain.updateUniforms(renderer, scene.terrain.visible);

    for (int i = 0; i < 9; i++)
    {
        if (scene.terrain.visible[i])
        {
            scene.terrain.visible[i]->updateUniforms(renderer, i);
        }
    }
}

void RenderSystem::terrain(RENDERSYSTEM_ARGS)
{    
    renderer.enqueueCommand(
            DZRenderCommand::SetPipeline(scene.terrain.terrain_pipeline));

    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    DZBufferBinding::Fragment(scene.scene_uniform_buffer, 0)
                ));

    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    DZBufferBinding::Vertex(scene.scene_uniform_buffer, 0)
                ));

    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    DZBufferBinding::Fragment(scene.light_buffer, 3)
                ));

    renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    DZBufferBinding::Fragment(
                        scene.terrain.terrain_uniform_buffer, 2)
                    )
            );
        
    for (int i = 0; i < 9; i++)
    {
        if (!scene.terrain.visible[i])
            continue;

        renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    DZBufferBinding::Vertex(
                        scene.terrain.visible[i]->local_uniforms_buffer, 2)));

        renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    DZBufferBinding::Fragment(
                        scene.terrain.visible[i]->local_uniforms_buffer, 1)));

        renderer.enqueueCommand(
                 DZRenderCommand::DrawMesh(
                     scene.terrain.visible[i]->mesh));
    }
}

void RenderSystem::models(RENDERSYSTEM_ARGS)
{
    renderer.enqueueCommand(DZRenderCommand::SetPipeline(scene.model_pipeline));

    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    DZBufferBinding::Fragment(scene.scene_uniform_buffer, 0)
                ));

    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    DZBufferBinding::Fragment(scene.light_buffer, 3)
                ));
    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    DZBufferBinding::Vertex(scene.scene_uniform_buffer, 0)
                ));

    scene.registry
        .view<Transform, Model>()
        .each(
                [&](const auto &transform, const auto &model)
                {
                    model.render(renderer, transform);
                }
            );
}
