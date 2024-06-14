#include "input.h"
#include "logger.h"
#include <SDL_scancode.h>
#define GLM_SWIZZLE
#include "systems.h"
#include "model.h"
#include "SDL_keycode.h"
#include "light.h"
#include "movement.h"


void GameSystem::inputActions(GAMESYSTEM_ARGS)
{
    if(input.mouse.left_button_down && !input.mouse_prev.left_button_down)
    {
        gui.selection.pos = v2i{input.mouse.pos.x, input.mouse.pos.y};
    }

    if (input.key[DZKey::C])
        scene.terrain.chunks.clear();

    if (!input.mouse.left_button_down && input.mouse_prev.left_button_down)
    {

    }
    
    if(input.key[DZKey::L] && !input.key_prev[DZKey::L])
    {
        if(scene.LOS_ON)
        {
            scene.LOS_ON = 0;
        }
        else
        {
            scene.LOS_ON = 1;
        }
        Log::verbose("LOS: %d", scene.LOS_ON);
    }
}

void GameSystem::debugControl(GAMESYSTEM_ARGS)
{
    // ZOOM
    glm::vec3 position = scene.camera.position;
    glm::vec3 target   = scene.camera.target;
    glm::vec3 toTarget = target - position;
    f32 distTarget = toTarget.length();
    f32 scroll_dir = std::abs(input.mouse.wheel_delta) * input.mouse.wheel_delta > 0 ? -1.0f : 1.0f;

    if (toTarget.length() > 10.0f && toTarget.length() < 100.0f)
    {
        scene.camera.position 
            += toTarget * scroll_dir * (toTarget.length() * toTarget.length() / 100.0f);
    }

    // WASD
    if(input.key[DZKey::W])
        scene.camera.rotateWithOrigin(v2f{1.f, 0.f});
    if(input.key[DZKey::A])
        scene.camera.rotateWithOrigin(v2f{0.f, -1.f});
    if(input.key[DZKey::S])
        scene.camera.rotateWithOrigin(v2f{-1.f, 0.f});
    if(input.key[DZKey::D])
        scene.camera.rotateWithOrigin(v2f{0.f, 1.f});

    // ARROW KEYS
    scene.registry
        .view<Transform>()
        .each(
                [&](auto &transform)
                {
                    if(input.key[DZKey::UP])
                        transform.rotation.x += 0.5f * delta_time;
                    if(input.key[DZKey::DOWN])
                        transform.rotation.x -= 0.5f * delta_time;
                    if(input.key[DZKey::RIGHT])
                        transform.rotation.z += 0.5f * delta_time;
                    if(input.key[DZKey::LEFT])
                        transform.rotation.z -= 0.5f * delta_time;
                }
            );

    // CHANGE TEXTURE
    if(input.key[DZKey::N] && !input.key_prev[DZKey::N])
    {
        scene.debug_texture += 2;
        scene.debug_texture %= 28;
    }
}

void GameSystem::cameraMovement(GAMESYSTEM_ARGS)
{
    // ZOOM
    scene.camera.zoom(
        std::abs(input.mouse.wheel_delta) * (input.mouse.wheel_delta > 0 ? -1.0f : 1.0f)
    );

    // WASD
    if(input.key[DZKey::W])
        scene.camera.move(glm::vec3(-1.0f, -1.0f, 0.0f));
    if(input.key[DZKey::A])
        scene.camera.move(glm::vec3(1.0f, -1.0f, 0.0f));
    if(input.key[DZKey::S])
        scene.camera.move(glm::vec3(1.0f, 1.0f, 0.0f));
    if(input.key[DZKey::D])
        scene.camera.move(glm::vec3(-1.0f, 1.0f, 0.0f));
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
            if(los_index > 100) los_index -= 1;
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
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(1.0),
        10.0f
    };

    renderer
        .setBufferOfSize(scene.light_buffer, &light_data, sizeof(DynamicPointLightData));

    SceneUniforms uniforms = {
        scene.camera.getCameraData(screen_dim),
        scene.sun.dir,
        1,
        scene.debug_texture,
        scene.LOS_ON,
        elapsed_time
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
                    Binding<DZBuffer>::Fragment(scene.scene_uniform_buffer, 0)
                ));

    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Vertex(scene.scene_uniform_buffer, 0)
                ));

    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Fragment(scene.light_buffer, 3)
                ));

    renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Fragment(
                        scene.terrain.terrain_uniform_buffer, 2)
                    )
            );
        
    for (int i = 0; i < 9; i++)
    {
        if (!scene.terrain.visible[i])
            continue;

        renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Vertex(
                        scene.terrain.visible[i]->local_uniforms_buffer, 2)));

        renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Fragment(
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
                    Binding<DZBuffer>::Fragment(scene.scene_uniform_buffer, 0)
                ));

    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Vertex(scene.scene_uniform_buffer, 0)
                ));

    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Fragment(scene.light_buffer, 3)
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

void RenderSystem::fow(RENDERSYSTEM_ARGS)
{
    renderer.enqueueCommand(DZRenderCommand::SetPipeline(scene.fow_pipeline));


    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Vertex(scene.scene_uniform_buffer, 0)
                ));

    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Fragment(scene.scene_uniform_buffer, 0)
                ));

    renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Fragment(
                        scene.terrain.terrain_uniform_buffer, 1)
                    )
            );

    for (int i = 0; i < 9; i++)
    {
        if (!scene.terrain.visible[i])
            continue;

        renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Vertex(
                        scene.terrain.visible[i]->local_uniforms_buffer, 2)));

        renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Fragment(
                        scene.terrain.visible[i]->local_uniforms_buffer, 2)));

        renderer.enqueueCommand(
                 DZRenderCommand::DrawMesh(
                     scene.terrain.visible[i]->mesh));

    }

}

void RenderSystem::gui(RENDERSYSTEM_ARGS)
{
    if (input.mouse.left_button_down)
    {
        gui.selection.dim = v2i {input.mouse.pos.x - gui.selection.pos.x, input.mouse.pos.y - gui.selection.pos.y};
        gui.selection_rect_transform.pos = glm::vec3(
                    (gui.selection.pos.x / screen_dim.x * 2.0f) - 1.0f, 
                (-gui.selection.pos.y / screen_dim.y * 2.0f) + 1.0f, 
                0.0);
        gui.selection_rect_transform.scale = glm::vec3(
                gui.selection.dim.x / screen_dim.x * 2.0f, 
                -gui.selection.dim.y / screen_dim.y * 2.0f, 
                0.0);

        renderer.enqueueCommand(
                DZRenderCommand::SetPipeline(scene.gui_pipeline));
        gui.selection_rect_model.render(renderer, gui.selection_rect_transform);
    }
}
