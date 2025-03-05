#include "systems.h"
#include <CoreFoundation/CoreFoundation.h>
#define GLM_FORCE_SWIZZLE

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

#include "boid.h"
#include "player.h"
#include "asset.h"
#include "common.h"
#include "logger.h"
#include "renderer.h"
#include "camera.h"
#include "terrain.h"
#include "model.h"
#include "geometry.h"
#include "scene.h"
#include "systems.h"
#include "input.h"
#include "light.h"

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

    SDL_ShowCursor(SDL_DISABLE);

    // INITIALIZE ASSET MANAGER

    AssetManager ass_man;

    // INITIALIZE RENDERING

    auto terrain_shader_src_handle = *ass_man.loadText("shaders/terrain_shader.metal");
    auto skyscraper_shader_src_handle = *ass_man.loadText("shaders/skyscraper.metal");
    auto fog_shader_src_handle = *ass_man.loadText("shaders/fog.metal");
    auto boid_shader_src_handle = *ass_man.loadText("shaders/boid.metal");
    
    //auto lighting_shader_h_handle = *ass_man.loadText("shaders/lighting.hm");
    //auto noise_shader_h_handle = *ass_man.loadText("shaders/noise.hm");
    //auto header_shader_h_handle = *ass_man.loadText("shaders/headers.hm");

    DZRenderer renderer(window);

    std::vector<DZShader> shaders 
        = renderer.loadPrecompiledShaders("build/shaders/lib/MyShaders.metallib", 
                {"terrain_vertexMain", "terrain_fragmentMain",
                "skyscraper_vertexMain", "skyscraper_fragmentMain",
                "skybox_vertexMain", "skybox_fragmentMain",
                "boid_vertexMain", "boid_fragmentMain"});

    //std::vector<DZShader> lighting_shaders 
    //   = renderer.compileShaders(*ass_man.getText(lighting_shader_h_handle), {});

    //std::vector<DZShader> noise_shaders 
    //    = renderer.compileShaders(*ass_man.getText(noise_shader_h_handle), {});

    //std::vector<DZShader> shader_headers
    //    = renderer.compileShaders(*ass_man.getText(header_shader_h_handle), {});


    // TODO: Handle more gracefully
    //if(terrain_shaders.size() != 2) exit(1);
    //if(skyscraper_shaders.size() != 2) exit(1);

    DZPipeline terrain_pipeline 
        = renderer.createPipeline(shaders[0], shaders[1]);

    DZPipeline skyscraper_pipeline 
        = renderer.createPipeline(shaders[2], shaders[3]);

    DZPipeline skybox_pipeline
        = renderer.createPipeline(shaders[4], shaders[5]);

    DZPipeline boid_pipeline
        = renderer.createPipeline(shaders[6], shaders[7]);


    ass_man.addSearchDirectory("resources/textures", true);

    std::vector<std::string> terrain_paths = {
        "alldirt",
        "moredirt",
        "bitdirt",
        "grassy",
        "mossy",
        "darkmoss",
        "undergrowth"
    };

    std::vector<std::string> texture_paths = {
        "skyscraper2",
        "skyscraper3",
        "skyscraper4",
        "skyscraper5",
        "skyscraper6"
    };


    Log::verbose("Loading textures...");

    std::vector<std::future<TextureData>> texture_datas_futures;
    std::vector<TextureData> texture_datas;

    for (const auto &tex_path : terrain_paths)
    {
        texture_datas_futures.push_back(
                std::async(std::launch::async, [&]{ return *ass_man.getTextureData(ass_man.loadTexture(tex_path + "-albedo.png").value()); }));
        texture_datas_futures.push_back(
                std::async(std::launch::async, [&]{ return *ass_man.getTextureData(ass_man.loadTexture(tex_path + "-normal.png").value()); }));
        texture_datas_futures.push_back(
                std::async(std::launch::async, [&]{ return *ass_man.getTextureData(ass_man.loadTexture(tex_path + "-displacement.png").value()); }));
    }

    for (const auto &tex_path : texture_paths)
    {
        texture_datas_futures.push_back(
                std::async(std::launch::async, [&]{ return *ass_man.getTextureData(ass_man.loadTexture(tex_path + ".png").value()); }));
    }

    Log::verbose("Awaiting textures...");

    for (auto &future : texture_datas_futures)
    {
        future.wait();
        texture_datas.push_back(future.get());
    }

    Log::verbose("Texture datas loaded.");

    texture_datas_futures.clear();

    DZTextureArray tex_array = renderer.createTextureArray(texture_datas);

    DZTexture tex = renderer.createTexture(texture_datas[2]);

    // CREATE WORLD

    World world {
        Scene(renderer, terrain_pipeline, skyscraper_pipeline, skybox_pipeline, boid_pipeline),
        {},
        {}
    };

    // SET WORLD SYSTEMS

    world.game_systems.push_back(&GameSystem::terrainGeneration);
    world.game_systems.push_back(&GameSystem::inputActions);
    world.game_systems.push_back(&GameSystem::updatePlayer);
    world.game_systems.push_back(&GameSystem::cameraMovement);
    world.game_systems.push_back(&GameSystem::dayNight);
    world.game_systems.push_back(&GameSystem::updateBoid);

    world.render_systems.push_back(&RenderSystem::updateData);
    world.render_systems.push_back(&RenderSystem::skybox);
    world.render_systems.push_back(&RenderSystem::terrain);
    world.render_systems.push_back(&RenderSystem::skyscrapers);
    world.render_systems.push_back(&RenderSystem::boids);
    
    SDL_Event e;
    InputState input;
    double delta_time = 0.0;
    double elapsed_time = 0.0;

    // CREATE Player

    Player player(glm::vec3(0, 0, 1), world.scene.camera);
    DynamicPointLightData player_light = { 
        glm::vec3(0.0),
        glm::vec3(255.f/255.f, 245.f/255.f, 213.f/255.f),
        1.0f
    };
    world.scene.lights[0] = player_light;


    while(true)
    {
        elapsed_time += delta_time;

        const auto frame_start = std::chrono::steady_clock::now();

        input.update();

        s32 window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);

        SDL_WarpMouseInWindow(window, window_width/2.f, window_height/2.f);

        glm::vec2 screen_dim(
                (float) window_width, 
                (float) window_height
            );

        if (input.quit)
            goto quit;

        renderer.waitForRenderFinish();

        for (auto system : world.game_systems)
            system(renderer, world.scene, input, player, delta_time);
   
        for (auto system : world.render_systems)
            system(renderer, world.scene, input, screen_dim, elapsed_time);

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

