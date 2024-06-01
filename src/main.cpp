#include "systems.h"
#define GLM_SWIZZLE

#include <SDL_events.h>
#include <SDL_metal.h>
#include <SDL_mouse.h>
#include <SDL_render.h>
#include <SDL_scancode.h>
#include <SDL_video.h>
#include <SDL.h>

#include <chrono>
#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <simd/simd.h>

#include <entt.hpp>

#include "common.h"
#include "logger.h"
#include "renderer.h"
#include "camera.h"
#include "asset.h"
#include "terrain.h"
#include "texture.h"
#include "model.h"
#include "term_renderer.h"
#include "entity.h"
#include "geometry.h"
#include "factory.h"
#include "light.h"
#include "scene.h"
#include "systems.h"

#define SDL_ERR(msg) \
    printf("[ERROR] %s\n\t%s\n", msg, SDL_GetError())

const glm::vec3 north(-1.0f, -1.0f, 0.0f);
const glm::vec3 south(1.0f, 1.0f, 0.0f);
const glm::vec3 east(-1.0f, 1.0f, 0.0f);
const glm::vec3 west(1.0f, -1.0f, 0.0f);


struct World
{
    Scene scene;

    std::vector<std::function<void(GAMESYSTEM_ARGS)>> game_systems;
    std::vector<std::function<void(RENDERSYSTEM_ARGS)>> render_systems;
};

int main(int argc, char *argv[])
{
    // TODO: Set log level with option or defines
    Log::setLogLevel(Log::LogLevel::VERBOSE);

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        SDL_ERR("SDL could not initialize!");

    SDL_Window *window = SDL_CreateWindow(
            "SDL2Test",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            1024,
            768,
            SDL_WINDOW_ALLOW_HIGHDPI
            | SDL_WINDOW_BORDERLESS
        );


    if (window == NULL)
        SDL_ERR("SDL_Window could not be created!");

    AssetManager ass_man;

    auto gui_shader_src = *ass_man.getTextFile("shaders/gui_shader.metal");
    auto basic_shader_src = *ass_man.getTextFile("shaders/basic_shader.metal");
    auto terrain_shader_src = *ass_man.getTextFile("shaders/terrain_shader.metal");

    DZRenderer renderer(window);

    std::vector<DZShader> gui_shaders 
        = renderer.compileShaders(gui_shader_src, {"vertexMain", "fragmentMain"});
    std::vector<DZShader> basic_shaders 
        = renderer.compileShaders(basic_shader_src, {"vertexMain", "fragmentMain"});
    std::vector<DZShader> terrain_shaders 
        = renderer.compileShaders(terrain_shader_src, {"vertexMain", "fragmentMain"});

    // TODO: Handle more gracefully
    if(gui_shaders.size()     != 2) exit(1);
    if(basic_shaders.size()   != 2) exit(1);
    if(terrain_shaders.size() != 2) exit(1);

    DZPipeline gui_pipeline 
        = renderer.createPipeline(gui_shaders[0], gui_shaders[1]);
    DZPipeline basic_pipeline 
        = renderer.createPipeline(basic_shaders[0], basic_shaders[1]);
    DZPipeline terrain_pipeline 
        = renderer.createPipeline(terrain_shaders[0], terrain_shaders[1]);

    // TODO: Get these from a config file
    ass_man.addSearchDirectory("resources");
    ass_man.addSearchDirectory("resources/new");
    ass_man.addSearchDirectory("resources/new/1-alldirt-negnegneg");
    ass_man.addSearchDirectory("resources/new/2-moredirt-negneg");
    ass_man.addSearchDirectory("resources/new/3-bitdirt-neg");
    ass_man.addSearchDirectory("resources/new/4-grassy-mid");
    ass_man.addSearchDirectory("resources/new/5-mossy-pos");
    ass_man.addSearchDirectory("resources/new/6-darkmoss-pospos");
    ass_man.addSearchDirectory("resources/new/7-undergrowth-pospospos");

    ass_man.addSearchDirectory("resources/new/badlands/");
    ass_man.addSearchDirectory("resources/new/badlands/1 - brickwork mossy fungusy ruins");
    ass_man.addSearchDirectory("resources/new/badlands/2-burnt undergrowth, charred sticks(2)");
    ass_man.addSearchDirectory("resources/new/badlands/3 - burnt undergrowth, charred sticks burnt dark moss");
    ass_man.addSearchDirectory("resources/new/badlands/4 - dead ground smoldering ground burnt grass, charred dirt. black and evil earth.");
    ass_man.addSearchDirectory("resources/new/badlands/5 - dry, cracks. green ooze in cracks.");
    ass_man.addSearchDirectory("resources/new/badlands/6 - dirt. wet green ooze around. bright green.");
    ass_man.addSearchDirectory("resources/new/badlands/7 - all ooze, some dirt pokes out, putrid");

//   ass_man.addSearchDirectory("resources/new/meatlands/");
//   ass_man.addSearchDirectory("resources/new/meatlands/1 - vastmembrane");
//   ass_man.addSearchDirectory("resources/new/meatlands/2 - biggerorgans");
//   ass_man.addSearchDirectory("resources/new/meatlands/3 - organy");
//   ass_man.addSearchDirectory("resources/new/meatlands/4 - meat");
//   ass_man.addSearchDirectory("resources/new/meatlands/5 - more blood guts");
//   ass_man.addSearchDirectory("resources/new/meatlands/6 - bloodier");
//   ass_man.addSearchDirectory("resources/new/meatlands/7 - blood everywhere");
//
//   ass_man.addSearchDirectory("resources/new/rainforest/");
//   ass_man.addSearchDirectory("resources/new/rainforest/1 - "); 
//   ass_man.addSearchDirectory("resources/new/rainforest/2 - "); 
//   ass_man.addSearchDirectory("resources/new/rainforest/3 - "); 
//   ass_man.addSearchDirectory("resources/new/rainforest/4 - "); 
//   ass_man.addSearchDirectory("resources/new/rainforest/5 - "); 
//   ass_man.addSearchDirectory("resources/new/rainforest/6 - "); 
//   ass_man.addSearchDirectory("resources/new/rainforest/6 - "); 
//
//   ass_man.addSearchDirectory("resources/new/badlands/");
//   ass_man.addSearchDirectory("resources/new/badlands/1 - ");
//   ass_man.addSearchDirectory("resources/new/badlands/2 - ");
//   ass_man.addSearchDirectory("resources/new/badlands/3 - ");
//   ass_man.addSearchDirectory("resources/new/badlands/4 - ");
//   ass_man.addSearchDirectory("resources/new/badlands/5 - ");
//   ass_man.addSearchDirectory("resources/new/badlands/6 - ");
//   ass_man.addSearchDirectory("resources/new/badlands/6 - ");




    std::vector<std::string> texture_paths = {
        "alldirt-albedo.png",    
        "alldirt-normal.png",    
        "moredirt-albedo.png",    
        "moredirt-normal.png", 
        "bitdirt-albedo.png",
        "bitdirt-normal.png",
        "grassy-albedo.png",    
        "grassy-normal.png",    
        "mossy-albedo.png",    
        "mossy-normal.png",
        "darkmoss-albedo.png",
        "darkmoss-normal.png",
        "undergrowth-albedo.png",
        "undergrowth-normal.png",

        "brick-albedo.png",    
        "brick-normal.png",    
        "burnt-albedo.png",    
        "burnt-normal.png",
        "burntmoss-albedo.png",
        "burntmoss-normal.png",
        "deadground-albedo.png",  
        "deadground-normal.png",    
        "cracksooze-albedo.png",    
        "cracksooze-normal.png",
        "dirtooze-albedo.png",
        "dirtooze-normal.png",
        "ooze-albedo.png",
        "ooze-normal.png",

//       ".png",    
//       ".png",    
//       ".png",    
//       ".png",
//       ".png",
//       ".png",
//       ".png",  
//       ".png",    
//       ".png",    
//       ".png",
//       ".png",
//       ".png",
//       ".png",
//       ".png",
//
//       ".png",    
//       ".png",    
//       ".png",    
//       ".png",
//       ".png",
//       ".png",
//       ".png",  
//       ".png",    
//       ".png",    
//       ".png",
//       ".png",
//       ".png",
//       ".png",
//       ".png",
//
//       ".png",    
//       ".png",    
//       ".png",    
//       ".png",
//       ".png",
//       ".png",
//       ".png",  
//       ".png",    
//       ".png",    
//       ".png",
//       ".png",
//       ".png",
//       ".png",
//       ".png",
    };

    std::vector<TextureData> texture_datas;

    for (const auto &tex_path : texture_paths)
    {
        texture_datas.push_back(ass_man.getTexture(tex_path).value());
    }

    DZTextureArray tex_array = renderer.createTextureArray(texture_datas);

    const u8 *key_state = SDL_GetKeyboardState(nullptr);
    u8 *prev_key_state = (u8 *) malloc(SDL_NUM_SCANCODES);

    World world {
        Scene(renderer, terrain_pipeline, basic_pipeline),
        {},
        {}
    };

    world.scene.terrain.createChunk(renderer, glm::vec2(0.0f, 0.0f));
    world.scene.terrain.createChunk(renderer, glm::vec2(-100.0f, -100.0f));
    world.scene.terrain.createChunk(renderer, glm::vec2(-100.0f, 0.0f));
    world.scene.terrain.createChunk(renderer, glm::vec2(0.0f, -100.0f));

    const entt::entity c = world.scene.registry.create();
    Transform trans;
    trans.pos = glm::vec3(0.0f, 0.0f, 0.0f);

    world.scene.registry.emplace<Transform>(c, trans);
    world.scene.registry.emplace<LineOfSight>(c, 5u);
    world.scene.registry.emplace<MoveSpeed>(c, 5u);
    auto loser_plane_data = MeshData::UnitPlane();
    loser_plane_data.translate(glm::vec3(-0.5, -0.5, 0.0));
    DZMesh loser_mesh = renderer.createMesh(loser_plane_data);

    for (int i = 0; i < 10; i++)
    {
        const entt::entity c = world.scene.registry.create();
        Transform loser_transform;
        loser_transform.pos = glm::vec3((rand() % 200) * 1.0f,(rand() % 200) * 1.0f, 0.0f);
        world.scene.registry.emplace<Transform>(c, loser_transform);
        Model loser_model = Model::fromMeshes(renderer, std::vector<DZMesh> { loser_mesh });
        world.scene.registry.emplace<Model>(c, loser_model);
        world.scene.registry.emplace<LineOfSight>(c, 5u);
        world.scene.registry.emplace<MoveSpeed>(c, rand() % 10u + 2u);
    }

    AArect2i selection;
    selection.pos = {0, 0};
    selection.dim = {0, 0};

    Model selection_rect_model = 
        Model::fromMeshDatas(renderer, { MeshData::UnitSquare() });

    SDL_Event e;
    bool mouse_held = false;
    int x_start, y_start;
    int x_curr, y_curr;

    Transform selection_rect_transform;

    double delta_time = 0.0;

    world.game_systems.push_back(&GameSystem::cameraMovement);
    world.game_systems.push_back(&GameSystem::terrainGeneration);
    world.game_systems.push_back(&GameSystem::unitMovement);
    world.game_systems.push_back(&GameSystem::LOS);

    world.render_systems.push_back(&RenderSystem::updateData);
    world.render_systems.push_back(&RenderSystem::terrain);
    world.render_systems.push_back(&RenderSystem::models);
    
    World *curr_world;

    curr_world = &world;
    

    // // // ALL  WORLD CREATE \\ \\ \\

    World alt_world = {
        Scene(renderer, terrain_pipeline, basic_pipeline),
        {},
        {}
    };

    alt_world.scene.camera.ortho = false;

    alt_world.game_systems.push_back(&GameSystem::cameraMovement);
    alt_world.render_systems.push_back(&RenderSystem::updateData);
    alt_world.render_systems.push_back(&RenderSystem::models);

    auto plane_entity = alt_world.scene.registry.create();
    auto plane_entity_mesh_data = MeshData::UnitPlane();
    plane_entity_mesh_data.translate(glm::vec3(-0.5, -0.5, 0.0));
    Model plane_model 
        = Model::fromMeshDatas(renderer, {plane_entity_mesh_data});
    alt_world.scene.registry.emplace<Model>(plane_entity, plane_model);
    alt_world.scene.registry.emplace<Transform>(plane_entity);

    while(true)
    {
        const auto frame_start = std::chrono::steady_clock::now();

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) 
                goto quit;

            if(e.type == SDL_MOUSEWHEEL)
            {

                if(!curr_world->scene.camera.ortho)
                {                    
                    glm::vec3 position = curr_world->scene.camera.position;
                    glm::vec3 target   = curr_world->scene.camera.target;
                    glm::vec3 toTarget = target - position;

                    f32 distTarget = toTarget.length();

                    f32 scroll_dir = e.wheel.y > 0 ? -1.0f : 1.0f;

                    if (toTarget.length() > 10.0f, toTarget.length() < 100.0f)
                    {
                        curr_world->scene.camera.position 
                            += toTarget * scroll_dir * (toTarget.length() * toTarget.length() / 100.0f);
                    }
                } 
                else 
                {
                    curr_world->scene.camera.zoom(e.wheel.y > 0 ? -1.0f : 1.0f);
                }
            } 

            if(e.type == SDL_MOUSEBUTTONDOWN)
            {
                SDL_GetMouseState(&x_start, &y_start);
                mouse_held = true;

                selection.pos = v2i {x_start, y_start};
                Log::verbose("Mouse button down");
            }

            if(e.type == SDL_MOUSEBUTTONUP)
            {
                mouse_held = false;
                Log::verbose("Mouse button up");
            }
        }

        s32 window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);
        glm::vec2 screen_dim(
                (float) window_width, 
                (float) (window_height)
            );

        SDL_GetMouseState(&x_curr, &y_curr);

        if (mouse_held)
        {
            selection.dim = v2i { x_curr - x_start, y_curr - y_start };
            selection_rect_transform.pos = glm::vec3(
                        (selection.pos.x / screen_dim.x * 2.0f) - 1.0f, 
                    (-selection.pos.y / screen_dim.y * 2.0f) + 1.0f, 
                    0.0);
            selection_rect_transform.scale = glm::vec3(
                    selection.dim.x / screen_dim.x * 2.0f, 
                    -selection.dim.y / screen_dim.y * 2.0f, 
                    0.0);
        }

        if (key_state[SDL_SCANCODE_ESCAPE])
            goto quit;

        if (key_state[SDL_SCANCODE_C])
            curr_world->scene.terrain.chunks.clear();

        if (key_state[SDL_SCANCODE_F] && !prev_key_state[SDL_SCANCODE_F])
        {
            if (curr_world == &world)
                curr_world = &alt_world;
            else
                curr_world = &world;
        }

        renderer.waitForRenderFinish();

        for (auto system : curr_world->game_systems)
            system(renderer, curr_world->scene, key_state, delta_time);
   
        for (auto system : curr_world->render_systems)
            system(renderer, curr_world->scene, screen_dim);
        
        
        renderer.enqueueCommand(
                DZRenderCommand::SetPipeline(gui_pipeline));

        if (mouse_held)
        {
            selection_rect_model.render(renderer, selection_rect_transform);
        }

        renderer.executeCommandQueue();

        const auto frame_end = std::chrono::steady_clock::now();
        const std::chrono::duration<double> frame_delta = frame_end - frame_start;

        delta_time = frame_delta.count();

        memcpy(prev_key_state, key_state, SDL_NUM_SCANCODES);
    }

quit:

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


