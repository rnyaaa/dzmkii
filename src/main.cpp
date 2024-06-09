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
#include "input.h"

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
    // INITIALIZE WINDOW
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

    // INITIALIZE ASSET MANAGER

    AssetManager ass_man;

    // INITIALIZE RENDERING

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
                                       
    ass_man.addSearchDirectory("resources/new/rainforest/");                                       
    ass_man.addSearchDirectory("resources/new/rainforest/1 - tarryoil");                               
    ass_man.addSearchDirectory("resources/new/rainforest/2 - tarryblack"); 
    ass_man.addSearchDirectory("resources/new/rainforest/3 - rainundergrowth"); 
    ass_man.addSearchDirectory("resources/new/rainforest/4 - forestgore"); 
    ass_man.addSearchDirectory("resources/new/rainforest/5 - bloodvines"); 
    ass_man.addSearchDirectory("resources/new/rainforest/6 - redvein"); 
    ass_man.addSearchDirectory("resources/new/rainforest/7 - veinmass"); 
 
    ass_man.addSearchDirectory("resources/new/coldlands/");
    ass_man.addSearchDirectory("resources/new/coldlands/1 - bogfrost");
    ass_man.addSearchDirectory("resources/new/coldlands/2 - frozen");
    ass_man.addSearchDirectory("resources/new/coldlands/3 - permafrost");
    ass_man.addSearchDirectory("resources/new/coldlands/4 - boggytundragrass");
    ass_man.addSearchDirectory("resources/new/coldlands/5 - coldbog");
    ass_man.addSearchDirectory("resources/new/coldlands/6 - sparserocks");
    ass_man.addSearchDirectory("resources/new/coldlands/7 - rocks");
 
    ass_man.addSearchDirectory("resources/new/desertlands/");
    ass_man.addSearchDirectory("resources/new/desertlands/1 - greener");
    ass_man.addSearchDirectory("resources/new/desertlands/2 - straw");
    ass_man.addSearchDirectory("resources/new/desertlands/3 - sparse");
    ass_man.addSearchDirectory("resources/new/desertlands/4 - sand");
    ass_man.addSearchDirectory("resources/new/desertlands/5 - rockier");
    ass_man.addSearchDirectory("resources/new/desertlands/6 - boulders");
    ass_man.addSearchDirectory("resources/new/desertlands/7 - grayrockface");
 
    ass_man.addSearchDirectory("resources/new/gravelands/");
    ass_man.addSearchDirectory("resources/new/gravelands/1 - allbones");
    ass_man.addSearchDirectory("resources/new/gravelands/2 - bones");
    ass_man.addSearchDirectory("resources/new/gravelands/3 - gravedirt");
    ass_man.addSearchDirectory("resources/new/gravelands/4 - deadgrass");
    ass_man.addSearchDirectory("resources/new/gravelands/5 - sparsedirt");
    ass_man.addSearchDirectory("resources/new/gravelands/6 - needles");
    ass_man.addSearchDirectory("resources/new/gravelands/7 - pineundergrowth");
 
    ass_man.addSearchDirectory("resources/new/meatlands/");
    ass_man.addSearchDirectory("resources/new/meatlands/1 - vastmembrane");
    ass_man.addSearchDirectory("resources/new/meatlands/2 - biggerorgans"); 
    ass_man.addSearchDirectory("resources/new/meatlands/3 - organy");
    ass_man.addSearchDirectory("resources/new/meatlands/4 - meat");
    ass_man.addSearchDirectory("resources/new/meatlands/5 - morebloodguts");
    ass_man.addSearchDirectory("resources/new/meatlands/6 - bloodier"); 
    ass_man.addSearchDirectory("resources/new/meatlands/7 - bloodeverywhere");                    
 
    ass_man.addSearchDirectory("resources/new/badlands/");
    ass_man.addSearchDirectory("resources/new/badlands/1 - brick");
    ass_man.addSearchDirectory("resources/new/badlands/2 - burnt");
    ass_man.addSearchDirectory("resources/new/badlands/3 - burntmoss");
    ass_man.addSearchDirectory("resources/new/badlands/4 - deadground");
    ass_man.addSearchDirectory("resources/new/badlands/5 - cracksooze");
    ass_man.addSearchDirectory("resources/new/badlands/6 - dirtooze");
    ass_man.addSearchDirectory("resources/new/badlands/7 - ooze");

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

    std::vector<TextureData> texture_datas;

    for (const auto &tex_path : texture_paths)
    {
        texture_datas.push_back(ass_man.getTexture(tex_path + "-albedo.png").value());
        texture_datas.push_back(ass_man.getTexture(tex_path + "-normal.png").value());
        texture_datas.push_back(ass_man.getTexture(tex_path + "-displacement.png").value());
    }

    DZTextureArray tex_array = renderer.createTextureArray(texture_datas);

    // CREATE WORLD

    World world {
        Scene(renderer, terrain_pipeline, basic_pipeline, gui_pipeline),
        {},
        {}
    };

    // INITIALIZE TERRAIN AND WORLD REGISTRY

    world.scene.terrain.createChunk(renderer, glm::vec2(0.0f, 0.0f), world.scene.terrain.biomepoints);
    world.scene.terrain.createChunk(renderer, glm::vec2(-100.0f, -100.0f), world.scene.terrain.biomepoints);
    world.scene.terrain.createChunk(renderer, glm::vec2(-100.0f, 0.0f), world.scene.terrain.biomepoints);
    world.scene.terrain.createChunk(renderer, glm::vec2(0.0f, -100.0f), world.scene.terrain.biomepoints);

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
    
    World *curr_world;
    curr_world = &world;
    

    // DEBUG WORLD

    World alt_world = {
        Scene(renderer, terrain_pipeline, basic_pipeline, gui_pipeline),
        {},
        {}
    };

    alt_world.scene.camera.ortho = false;
    alt_world.game_systems.push_back(&GameSystem::debugControl);
    alt_world.render_systems.push_back(&RenderSystem::updateData);
    alt_world.render_systems.push_back(&RenderSystem::models);

    auto plane_entity = alt_world.scene.registry.create();
    auto plane_entity_mesh_data = MeshData::UnitPlane();
    plane_entity_mesh_data.translate(glm::vec3(-0.5, -0.5, 0.0));
    Model plane_model 
        = Model::fromMeshDatas(renderer, {plane_entity_mesh_data});
    alt_world.scene.registry.emplace<Model>(plane_entity, plane_model);
    alt_world.scene.registry.emplace<Transform>(plane_entity);

    // INITIALIZE THINGIES

    GUI gui;
    gui.selection_rect_model = Model::fromMeshDatas(renderer, { MeshData::UnitSquare() });
    SDL_Event e;
    InputState input;
    double delta_time = 0.0;

    while(true)
    {
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
                curr_world = &alt_world;
            else
                curr_world = &world;
        }

        renderer.waitForRenderFinish();

        for (auto system : curr_world->game_systems)
            system(renderer, curr_world->scene, input, gui, delta_time);
   
        for (auto system : curr_world->render_systems)
            system(renderer, curr_world->scene, input, screen_dim, gui);

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


