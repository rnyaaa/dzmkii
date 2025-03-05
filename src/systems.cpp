#include "renderer.h"
#include <cmath>
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
#include "light.h"
#include "noise.h"
#include "boid.h"

void GameSystem::updateBoid(GAMESYSTEM_ARGS)
{
    std::vector<glm::vec3> updated_vectors(scene.boids.size());

    // We love to leak boids
    if (input.key[DZKey::Y])
        scene.boids.clear();

    for(int i = 0; i < scene.boids.size(); i++){
        Boid *boid = scene.boids[i];
        boid->ischasing = false;
        int no_boids_detected = 0;
        int no_boids_avoiding = 0;

        glm::vec3 to_center_of_mass = {0.f, 0.f, 0.f};
        glm::vec3 avoidance_vector = {0.f, 0.f, 0.f};
        glm::vec3 matching_velocity = {0.f, 0.f, 0.f};
        glm::vec3 to_player = {0.f, 0.f, 0.f};
        for(auto otherboid: scene.boids)
        {
            if (otherboid == boid) continue;

            if(glm::distance(otherboid->pos, boid->pos) < boid->boid_detection_range)
            {
                no_boids_detected += 1;
                to_center_of_mass += otherboid->pos;
                matching_velocity += otherboid->velocity;
                if(glm::distance(otherboid->pos, boid->pos) < boid->boid_avoidance_range)
                {
                    no_boids_avoiding += 1;
                    avoidance_vector -= (otherboid->pos - boid->pos); 
                }
            }
        }
        if(glm::distance(player.pos, boid->pos) < boid->boid_detection_range)
        {
            to_player += (player.pos - boid->pos);
            boid->ischasing = true;
        }

        if(glm::distance(scene.terrain.getHeight(boid->pos.xy()), boid->pos.z) < boid->boid_avoidance_range * 2.f)
        {
            avoidance_vector.z += glm::distance(scene.terrain.getHeight(boid->pos.xy()), boid->pos.z) * 8.f; 
        }

        if(boid->pos.z - 20 > boid->boid_avoidance_range)
        {
            avoidance_vector.z -= (boid->pos.z - 20);
        }

        Chunk* inside_chunk = scene.terrain.getChunkFromPos(v2f{boid->pos.x, boid->pos.y});
        Skyscraper *closest_skyscraper;
        if(inside_chunk)
        {
            f32 min_dist = INFINITY;
            
            for(Skyscraper *skyscrp : inside_chunk->skyscrapers)
            {
                f32 dist_to_skyscraper = glm::distance(skyscrp->transform.pos, boid->pos);

                if (dist_to_skyscraper < min_dist)
                {
                    min_dist = dist_to_skyscraper;
                    closest_skyscraper = skyscrp;
                }
            }

            AArect2f collision = closest_skyscraper->collision_bound;
            f32 roof = closest_skyscraper->transform.pos.z + closest_skyscraper->transform.scale.z / 2.f;
            if(closest_skyscraper->collision_bound.collidesWith(circ2f{boid->pos.x, boid->pos.y, boid->boid_avoidance_range}) && (boid->pos.z - roof) < boid->boid_avoidance_range)
            {
                glm::vec3 center = glm::vec3(
                                    collision.pos.x + collision.dim.x/2, 
                                    collision.pos.y + collision.dim.y/2,
                                    closest_skyscraper->transform.pos.z + closest_skyscraper->transform.scale.z / 2.f);

                avoidance_vector -= (center - boid->pos) * glm::vec3(5.f);
            }
        }

        if(no_boids_detected < 1){ no_boids_detected = 1; }
        if(no_boids_avoiding < 1){ no_boids_avoiding = 1; }

        matching_velocity /= glm::vec3(no_boids_detected);
        to_center_of_mass /= glm::vec3(no_boids_detected);
        avoidance_vector  /= glm::vec3(no_boids_detected);

        if(input.key[DZKey::U])
            scene.boid_mass_factor += 0.000001;
        if(input.key[DZKey::I])
            scene.boid_match_factor += 0.000001;
        if(input.key[DZKey::O])
            scene.boid_avoidance_factor += 0.000001;

        if(input.key[DZKey::J])
            scene.boid_mass_factor -= 0.000001;
        if(input.key[DZKey::K])
            scene.boid_match_factor -= 0.000001;
        if(input.key[DZKey::L])
            scene.boid_avoidance_factor -= 0.000001;

        f32 to_player_factor = 0.025;
        //Log::verbose("MASS: %f, MATCH: %f, AVOIDANCE: %f", scene.boid_mass_factor, scene.boid_match_factor, scene.boid_avoidance_factor);
        to_center_of_mass = (to_center_of_mass - boid->pos) * scene.boid_mass_factor;
        matching_velocity = (matching_velocity - boid->velocity) * scene.boid_match_factor;
        avoidance_vector *= scene.boid_avoidance_factor;
        to_player *= to_player_factor;

        updated_vectors[i] = glm::normalize(boid->velocity + to_center_of_mass + matching_velocity + avoidance_vector + to_player);
    }
    for(int i = 0; i < scene.boids.size(); i++){
        scene.boids[i]->velocity = updated_vectors[i];
        scene.boids[i]->pos += scene.boids[i]->velocity * glm::vec3(delta_time) * glm::vec3(10.0);
        if(scene.boids[i]->pos.z < scene.terrain.getHeight(scene.boids[i]->pos.xy()) + scene.boids[i]->scale){
            scene.boids[i]->pos.z = (scene.terrain.getHeight(scene.boids[i]->pos.xy()) + scene.boids[i]->scale);
            scene.boids[i]->velocity.z += 1.f;
        }
    }
}

void GameSystem::inputActions(GAMESYSTEM_ARGS)
{
    if(input.key[DZKey::L] && !input.key_prev[DZKey::L])
    {
    }
}

void GameSystem::updatePlayer(GAMESYSTEM_ARGS)
{
    f32 speed = 0.5f;
    glm::vec2 player_right = scene.camera.right.xy() * speed;
    glm::vec2 player_forward = scene.camera.forward.xy() * speed;
    // WASD
    if(input.key[DZKey::W])
    {
        player.velocity.x += player_forward.x;
        player.velocity.y += player_forward.y;
    }
    if(input.key[DZKey::A])
    {
        player.velocity.x += player_right.x;
        player.velocity.y += player_right.y;
    }
    if(input.key[DZKey::S])
    {
        player.velocity.x += -player_forward.x;
        player.velocity.y += -player_forward.y;
    }
    if(input.key[DZKey::D])
    {
        player.velocity.x += -player_right.x;
        player.velocity.y += -player_right.y;
    }
   
    if(input.key[DZKey::SPACE])
    {
        if(player.boost > 0)
        {
            player.velocity.z += 6.5 ;
            player.velocity += scene.camera.forward * glm::vec3(0.5);

            //player.boost -= 3;
        }
    }

    player.boost += 1;
    player.velocity.z -= 1 * delta_time * 9.8 * 20;

    if (input.key[DZKey::N])
    {
        scene.elapsed_time += 0.5;
    }

    glm::vec3 newpos = player.pos + player.velocity * glm::vec3(delta_time);
    bool collided = false;
    circ2f player_bound{v2f{newpos.x, newpos.y}, 1.0f};
    f32 min_dist = INFINITY;
    Skyscraper *closest_skyscraper;
    bool on_roof = false;
    for(auto chunk : scene.terrain.visible)
    {
        for(Skyscraper *skyscrp : chunk->skyscrapers)
        {
            f32 dist_to_skyscraper = glm::distance(skyscrp->transform.pos, player.pos);

            if (dist_to_skyscraper < min_dist)
            {
                min_dist = dist_to_skyscraper;
                closest_skyscraper = skyscrp;
            }
            skyscrp->collision(player.velocity, on_roof, newpos);
        }
    }
    player.pos += player.velocity * glm::vec3(delta_time);

    if(player.pos.z < 2 + scene.terrain.getHeight(glm::vec2(player.pos.x, player.pos.y)))
    {
        player.pos.z = 2 + scene.terrain.getHeight(glm::vec2(player.pos.x, player.pos.y));
    }

    player.distance_travelled += glm::length(player.velocity.xy()) / 80.0;
    scene.camera.position = player.pos;

    if(std::abs(player.pos.z - scene.terrain.getHeight(glm::vec2(player.pos.x, player.pos.y))) < 2.5 || on_roof)
    {
        if(input.key[DZKey::SHIFT] )
        {
            player.velocity *= glm::vec3(1.1);
        }
        scene.camera.position.z += glm::pow(sin(player.distance_travelled), 2.0) / 4.0;
        player.velocity *= 0.9;
    }
    glm::vec3 lightpos = (scene.camera.forward * glm::vec3(3.0)) + player.pos;
    lightpos.z = std::max(scene.terrain.getHeight(lightpos.xy()) + 1.f, lightpos.z);
    
    scene.lights[0].pos = lightpos;
}

void GameSystem::cameraMovement(GAMESYSTEM_ARGS)
{
    // ZOOM
    scene.camera.zoom(
        std::abs(input.mouse.wheel_delta) * (input.mouse.wheel_delta > 0 ? -1.0f : 1.0f)
    );

   scene.camera.processMouseMovement(input.mouse.yrel, input.mouse.xrel);
}

void GameSystem::terrainGeneration(GAMESYSTEM_ARGS)
{
    for (int i = -2; i <= 2; i++)
    {
        for (int j = -2; j <= 2; j++)
        {
            glm::vec2 glm_pos = glm::vec2(scene.camera.position.xy() + glm::vec2(i * 100.0f, j * 100.0f));
            v2f pos = {glm_pos.x, glm_pos.y};
            if(scene.terrain.getChunkFromPos(pos) == nullptr){
                for(int l = 0; l < 10; l++)
                {
                    Boid *boid = new Boid(
                                            glm::vec3(
                                                scene.camera.position.x + rand()%i*100,
                                                scene.camera.position.y + rand()%j*100, 
                                                25.f
                                            )
                                        );
                    scene.boids.push_back(boid);
                }
            }
            scene.terrain.createChunk(renderer, scene.camera.position.xy() + glm::vec2(i * 100.0f, j * 100.0f));
        }
    }
    scene.terrain.getVisible(scene.camera);
}

void GameSystem::dayNight(GAMESYSTEM_ARGS)
{
    f32 perlin_scale = 0.5f;
    f32 noise_scale = 1.5f;
    u32 octaves = 16;

    const siv::PerlinNoise perlin(scene.elapsed_time);


    glm::vec3 daytime = glm::vec3(253, 251, 211);
    glm::vec3 dawndusk = glm::vec3(200, 60, 60);
    glm::vec3 night = glm::vec3(5, 12, 42);

    scene.elapsed_time += delta_time;
    f32 tod = sin(sin(scene.elapsed_time/10) * 1.5);
    scene.sun.dir.x = tod;
    scene.tod = tod;
    if(tod <= 0)
    {
        if(tod <= -0.0)
        {
            f32 noise = noise_scale * perlin.octave2D(tod * perlin_scale, scene.elapsed_time * perlin_scale, octaves);
            if(tod + noise < -0.2)
            {
                scene.lights[0].color = {2.0, 2.0, 2.0};
            } else {
                scene.lights[0].color = {0.0, 0.0, 0.0};
            }
        }
        scene.sun.color = glm::vec4(glm::mix(night, dawndusk, tod+1), 1);
    } else 
    {
        scene.sun.color = glm::vec4(glm::mix(dawndusk, daytime, tod), 1);
    }
    //scene.sun.dir.x = tod * 40;
    //scene.sun.dir.y = tod * 40;
}

void RenderSystem::skyscrapers(RENDERSYSTEM_ARGS)
{   
    renderer.enqueueCommand(
            DZRenderCommand::SetPipeline(scene.skyscraper_pipeline));
    
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
                    Binding<DZBuffer>::Fragment(scene.light_buffer, 2)
                ));

    for (int i = 0; i < 25; i++)
    {
        if (!scene.terrain.visible[i])
            continue;

        for(auto skyscraper : scene.terrain.visible[i]->skyscrapers)
        {
            renderer.enqueueCommand(
                    DZRenderCommand::BindBuffer(
                        Binding<DZBuffer>::Vertex(
                            skyscraper->local_uniforms_buffer, 2)));

            renderer.enqueueCommand(
                    DZRenderCommand::BindBuffer(
                        Binding<DZBuffer>::Fragment(
                            skyscraper->local_uniforms_buffer, 1)));

            renderer.enqueueCommand(
                     DZRenderCommand::DrawMesh(
                         skyscraper->mesh));
        }
    }
}

void RenderSystem::boids(RENDERSYSTEM_ARGS)
{   
    renderer.enqueueCommand(
            DZRenderCommand::SetPipeline(scene.boid_pipeline));
    
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
                    Binding<DZBuffer>::Fragment(scene.light_buffer, 2)
                ));


    for(auto boid : scene.boids)
    {
        renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Vertex(
                        boid->local_uniforms_buffer, 2)));

        renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Fragment(
                        boid->local_uniforms_buffer, 1)));

        renderer.enqueueCommand(
                 DZRenderCommand::DrawMesh(
                        boid->mesh));
    }
}

void RenderSystem::updateData(RENDERSYSTEM_ARGS)
{
    DynamicPointLightData player_light = scene.lights[0];
    std::sort(scene.boids.begin(), scene.boids.end(), [&scene](Boid *a, Boid *b) {
        return (a->pos - scene.camera.position).length() 
            < (b->pos - scene.camera.position).length(); // Sort in ascending order
    });
    for(int i = 1; i < NO_LIGHTS; i++){
        scene.boids[i%scene.boids.size()]->light.pos = scene.boids[i%scene.boids.size()]->pos;
        DynamicPointLightData boidlight = DynamicPointLightData{
            scene.boids[i%scene.boids.size()]->light.pos,
            scene.boids[i%scene.boids.size()]->color,
            scene.boids[i%scene.boids.size()]->light.falloff
        };
        if(scene.boids[i%scene.boids.size()]->ischasing)
        {
            boidlight.falloff *= 4.f;
            boidlight.color = scene.boids[i%scene.boids.size()]->chasing_color * 4.f;
        }
        scene.lights[i] = boidlight;
    }
    renderer
        .setBufferOfSize(scene.light_buffer, &scene.lights, sizeof(scene.lights));

    CameraData camdata = scene.camera.getCameraData(screen_dim);
    camdata.position = glm::vec4(scene.camera.position, 0.0f);
    SceneUniforms uniforms = {
        camdata,
        scene.sun.dir,
        scene.sun.color/ glm::vec4(255, 255, 255, 1.0),
        scene.no_lights,
        scene.elapsed_time,
        scene.tod,
    };

    renderer.setBufferOfSize(scene.scene_uniform_buffer, &uniforms, sizeof(SceneUniforms));

    scene.terrain.updateUniforms(renderer, scene.terrain.visible);

    for (int i = 0; i < 25; i++)
    {
        if (scene.terrain.visible[i])
        {
            scene.terrain.visible[i]->updateUniforms(renderer, i);
            for (int j = 0; j < SKYSCRAPERS_PER_CUNK; j++)
            {
                scene.terrain.visible[i]
                    ->skyscrapers[j]->updateUniforms(renderer);
            }
        } 
    }
    for(auto boid : scene.boids)
    {
        boid->updateUniforms(renderer);
    }
}

void RenderSystem::skybox(RENDERSYSTEM_ARGS)
{
    renderer.enqueueCommand(
            DZRenderCommand::SetPipeline(scene.skybox_pipeline));

    renderer.enqueueCommand(DZRenderCommand::SetDepthState(false));

    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Fragment(scene.scene_uniform_buffer, 0)
                ));

    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Vertex(scene.scene_uniform_buffer, 0)
                ));

    renderer.enqueueCommand(
             DZRenderCommand::DrawMesh(
                 scene.skybox_mesh
                 ));

    renderer.enqueueCommand(DZRenderCommand::SetDepthState(true));
}

void RenderSystem::terrain(RENDERSYSTEM_ARGS)
{    
    renderer.enqueueCommand(
            DZRenderCommand::SetPipeline(scene.terrain_pipeline));

    renderer.enqueueCommand(
            DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Fragment(scene.scene_uniform_buffer, 0)
                ));

    renderer.enqueueCommand(DZRenderCommand::BindBuffer(
        Binding<DZBuffer>::Vertex(scene.scene_uniform_buffer, 0)));

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
        
    for (int i = 0; i < 25; i++)
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

