#include "systems.h"
#include <SDL_error.h>
#define GLM_SWIZZLE

#include <SDL_events.h>
#include <SDL_metal.h>
#include <SDL_mouse.h>
#include <SDL_render.h>
#include <SDL_scancode.h>
#include <SDL_video.h>
#include <SDL.h>

#include <chrono>
#include <optional>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <future>

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
#include "model.h"
#include "term_renderer.h"
#include "entity.h"
#include "geometry.h"
#include "scene.h"
#include "systems.h"
#include "input.h"

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
    // INITIALIZE WINDOW
    // TODO: Set log level with option or defines
    Log::setLogLevel(Log::LogLevel::VERBOSE);

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        Log::error("SDL could not initialize:\n\t%s", SDL_GetError());
        exit(1);
    }

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
    {
        Log::error("SDL_Window could not be created:\n\t%s", SDL_GetError());
        exit(1);
    }

    // INITIALIZE ASSET MANAGER

    AssetManager ass_man;

    // INITIALIZE RENDERING

    auto gui_shader_src     = *ass_man.getTextFile("shaders/gui_shader.metal");
    auto basic_shader_src   = *ass_man.getTextFile("shaders/basic_shader.metal");
    auto terrain_shader_src = *ass_man.getTextFile("shaders/terrain_shader.metal");
    auto fow_shader_src     = *ass_man.getTextFile("shaders/LOS_shader.metal");

    DZRenderer renderer(window);

    std::vector<DZShader> gui_shaders 
        = renderer.compileShaders(gui_shader_src, {"vertexMain", "fragmentMain"});
    std::vector<DZShader> basic_shaders 
        = renderer.compileShaders(basic_shader_src, {"vertexMain", "fragmentMain"});
    std::vector<DZShader> terrain_shaders 
        = renderer.compileShaders(terrain_shader_src, {"vertexMain", "fragmentMain"});
    std::vector<DZShader> fow_shaders 
        = renderer.compileShaders(fow_shader_src, {"vertexMain", "fragmentMain"});

    // TODO: Handle more gracefully
    if(gui_shaders.size()     != 2) exit(1);
    if(basic_shaders.size()   != 2) exit(1);
    if(terrain_shaders.size() != 2) exit(1);
    if(fow_shaders.size()     != 2) exit(1);

    DZPipeline gui_pipeline 
        = renderer.createPipeline(gui_shaders[0], gui_shaders[1]);
    DZPipeline basic_pipeline 
        = renderer.createPipeline(basic_shaders[0], basic_shaders[1]);
    DZPipeline terrain_pipeline 
        = renderer.createPipeline(terrain_shaders[0], terrain_shaders[1]);
    DZPipeline fow_pipeline
        = renderer.createPipeline(fow_shaders[0], fow_shaders[1]);

    // TODO: Get these from a config file
    ass_man.addSearchDirectory("resources/new", true);

    std::vector<std::string> texture_paths = {
        // DEFAULT 0-6
        "alldirt",
        "moredirt",
        "bitdirt",
        "grassy",
        "mossy",
        "darkmoss",
        "undergrowth",

        // RAINFOREST 7-13
        "tarryoil",
        "tarryblack",
        "rainundergrowth",
        "forestgore",
        "bloodvines",
        "redvein",
        "veinmass",

        // COLDLANDS 13-20
        "bogfrost",
        "frozen",
        "permafrost",
        "boggytundragrass",
        "coldbog",
        "sparserocks",
        "rocks",

        // SANDLANDS 21-27
        "greener",
        "straw",
        "sparse",
        "sand",
        "rockier",
        "boulders",
        "grayrockface",

        // GRAVELANDS 28-34
        "allbones",
        "bones",
        "gravedirt",
        "deadgrass",
        "sparsedirt",
        "needles",
        "pineundergrowth",

        // MEATLANDS 35 - 41
        "vastmembrane",
        "biggerorgans",
        "organy",
        "meat",
        "morebloodguts",
        "bloodier",
        "bloodeverywhere",

        // BADLANDS 42-48
        "brick",
        "burnt",
        "burntmoss",
        "deadground",
        "cracksooze",
        "dirtooze",
        "ooze",

    };

    std::vector<std::future<TextureData>> texture_datas_futures;
    std::vector<TextureData> texture_datas;

    for (const auto &tex_path : texture_paths)
    {
        texture_datas_futures.push_back(
                std::async(std::launch::async, [&]{ return ass_man.getTexture(tex_path + "-albedo.png").value(); }));
        texture_datas_futures.push_back(
                std::async(std::launch::async, [&]{ return ass_man.getTexture(tex_path + "-normal.png").value(); }));
        texture_datas_futures.push_back(
                std::async(std::launch::async, [&]{ return ass_man.getTexture(tex_path + "-displacement.png").value(); }));
    }

    for (auto &future : texture_datas_futures)
    {
        future.wait();
        texture_datas.push_back(future.get());
    }

    texture_datas_futures.clear();

    DZTextureArray tex_array = renderer.createTextureArray(texture_datas);

    DZTexture tex = renderer.createTexture(texture_datas[2]);

    // CREATE WORLD

    World world {
        Scene(renderer, terrain_pipeline, basic_pipeline, gui_pipeline, fow_pipeline),
        {},
        {}
    };

    // INITIALIZE TERRAIN AND WORLD REGISTRY

    // world.scene.terrain.createChunk(renderer, glm::vec2(0.0f, 0.0f));
    // world.scene.terrain.createChunk(renderer, glm::vec2(-100.0f, -100.0f));
    // world.scene.terrain.createChunk(renderer, glm::vec2(-100.0f, 0.0f));
    // world.scene.terrain.createChunk(renderer, glm::vec2(0.0f, -100.0f));

    const entt::entity c = world.scene.registry.create();

    // CREATE DEBUG UNITS 
    
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

    // SET WORLD SYSTEMS

    world.game_systems.push_back(&GameSystem::inputActions);
    world.game_systems.push_back(&GameSystem::cameraMovement);
    world.game_systems.push_back(&GameSystem::terrainGeneration);
    world.game_systems.push_back(&GameSystem::unitMovement);
    world.game_systems.push_back(&GameSystem::LOS);

    world.render_systems.push_back(&RenderSystem::updateData);
    world.render_systems.push_back(&RenderSystem::terrain);
    world.render_systems.push_back(&RenderSystem::models);
    world.render_systems.push_back(&RenderSystem::gui);
    world.render_systems.push_back(&RenderSystem::fow);
    
    // DEBUG WORLD

    World model_view_world = {
        Scene(renderer, terrain_pipeline, basic_pipeline, gui_pipeline, fow_pipeline),
        {},
        {}
    };

    model_view_world.scene.camera.ortho = false;
    model_view_world.game_systems.push_back(&GameSystem::debugControl);
    model_view_world.render_systems.push_back(&RenderSystem::updateData);
    model_view_world.render_systems.push_back(&RenderSystem::models);

    auto plane_entity = model_view_world.scene.registry.create();
    auto plane_entity_mesh_data = MeshData::UnitPlane();
    plane_entity_mesh_data.translate(glm::vec3(-0.5, -0.5, 0.0));
    Model plane_model 
        = Model::fromMeshDatas(renderer, {plane_entity_mesh_data});
    model_view_world.scene.registry.emplace<Model>(plane_entity, plane_model);
    model_view_world.scene.registry.emplace<Transform>(plane_entity);

    // INITIALIZE THINGIES
    
    World *curr_world;
    curr_world = &model_view_world;
    
    GUI gui;
    gui.selection_rect_model = Model::fromMeshDatas(renderer, { MeshData::UnitSquare() });
    gui.selection_rect_model.textured = false;
    SDL_Event e;
    InputState input;
    double delta_time = 0.0;
    double elapsed_time = 0.0;

    // Temporary
    // TODO: Remove
    
    Model texture_display_model = Model::fromMeshDatas(renderer, { MeshData::UnitPlane() });
    Transform texture_display_transform = {
        glm::vec3(-.5, -.5, 0.0),
        glm::vec3(1.0),
        glm::vec3(0.0)
    };

    while(true)
    {
        elapsed_time += delta_time;
        const auto frame_start = std::chrono::steady_clock::now();
        input.update();
 
        s32 window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);
        glm::vec2 screen_dim(
                (float) window_width, 
                (float) (window_height)
            );


        if (input.quit)
            goto quit;

        if (input.key[DZKey::F] && !input.key_prev[DZKey::F])
        {
            if (curr_world == &world)
                curr_world = &model_view_world;
            else
                curr_world = &world;
        }

        renderer.waitForRenderFinish();

        for (auto system : curr_world->game_systems)
            system(renderer, curr_world->scene, input, gui, delta_time);
   
        for (auto system : curr_world->render_systems)
            system(renderer, curr_world->scene, input, screen_dim, gui, elapsed_time);

        // Texture preview
        //renderer.enqueueCommand(
        //        DZRenderCommand::SetPipeline(curr_world->scene.gui_pipeline));
        //renderer.enqueueCommand(
        //        DZRenderCommand::BindTexture(Binding<DZTexture>::Fragment(tex, 0)));

        //texture_display_model.textured = true;
        //texture_display_model.render(renderer, texture_display_transform);
        // /Texture Preview

        renderer.executeCommandQueue();

        const auto frame_end = std::chrono::steady_clock::now();
        const std::chrono::duration<double> frame_delta = frame_end - frame_start;

        delta_time = frame_delta.count();
    }

quit:

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


