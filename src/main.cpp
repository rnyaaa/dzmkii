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

#include "asset.h"
#include "common.h"
#include "logger.h"
#include "renderer.h"
#include "camera.h"
#include "model.h"
#include "geometry.h"
#include "scene.h"
#include "systems.h"
#include "input.h"

#define SDL_ERR(msg) \
    printf("[ERROR] %s\n\t%s\n", msg, SDL_GetError())

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

    //SDL_ShowCursor(SDL_DISABLE);

    // INITIALIZE ASSET MANAGER

    AssetManager ass_man;

    // INITIALIZE RENDERING

    auto sand_shader_src_handle = *ass_man.loadText("shaders/sand_shader.metal");
    
    //auto lighting_shader_h_handle = *ass_man.loadText("shaders/lighting.hm");
    //auto noise_shader_h_handle = *ass_man.loadText("shaders/noise.hm");
    //auto header_shader_h_handle = *ass_man.loadText("shaders/headers.hm");

    DZRenderer renderer(window);
std::string shader_source = R"(
#include <metal_stdlib>
using namespace metal;

struct v2f
{
    float4 position [[position]];
};

struct Vertex
{
    float4 position;
    float4 normal;
    float4 tangent;
    float4 bitangent;
    float4 color;
    float2 uv;
};

struct CameraData
{
    float4 position;
    float4x4 view_matrix;
    float4x4 projection_matrix;
};

struct GlobalUniforms
{
    CameraData camera;
    float elapsed_time;
};

struct ModelUniforms
{
    float4x4 model_matrix;
    bool textured;
    bool lit;
    uint material_index;
};

vertex v2f vertexMain( 
        uint vertex_id [[ vertex_id ]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        device const Vertex *vertices [[ buffer(1) ]],
        constant ModelUniforms &local_uniforms [[ buffer(2) ]]
    )
{
    v2f o;
    float4 local_pos = vertices[vertex_id].position - float4{0.5, 0.5, 0.0, 0.0};
    float4 world_pos = local_uniforms.model_matrix * local_pos;
    o.position = global_uniforms.camera.projection_matrix * global_uniforms.camera.view_matrix * world_pos;
    return o;
}

fragment half4 fragmentMain(v2f in [[stage_in]])
{
    return half4(1.0, 0.0, 0.0, 1.0);
}
)";

std::vector<DZShader> shaders 
    = renderer.compileShaders(shader_source, {"vertexMain", "fragmentMain"});    // TODO: Handle more gracefully
    //if(terrain_shaders.size() != 2) exit(1);
    //if(skyscraper_shaders.size() != 2) exit(1);

    DZPipeline sand_pipeline
        = renderer.createPipeline(shaders[0], shaders[1]);

    //std::vector<std::string> texture_paths = {
    //   // "skyscraper6"
    //};


    //Log::verbose("Loading textures...");

    //std::vector<std::future<TextureData>> texture_datas_futures;
    //std::vector<TextureData> texture_datas;

    //for (const auto &tex_path : terrain_paths)
    //{
    //    texture_datas_futures.push_back(
    //            std::async(std::launch::async, [&]{ return *ass_man.getTextureData(ass_man.loadTexture(tex_path + "-albedo.png").value()); }));
    //    texture_datas_futures.push_back(
    //            std::async(std::launch::async, [&]{ return *ass_man.getTextureData(ass_man.loadTexture(tex_path + "-normal.png").value()); }));
    //    texture_datas_futures.push_back(
    //            std::async(std::launch::async, [&]{ return *ass_man.getTextureData(ass_man.loadTexture(tex_path + "-displacement.png").value()); }));
    //}

    //for (const auto &tex_path : texture_paths)
    //{
    //    texture_datas_futures.push_back(
    //            std::async(std::launch::async, [&]{ return *ass_man.getTextureData(ass_man.loadTexture(tex_path + ".png").value()); }));
    //}

    //Log::verbose("Awaiting textures...");

    //for (auto &future : texture_datas_futures)
    //{
    //    future.wait();
    //    texture_datas.push_back(future.get());
    //}

    //Log::verbose("Texture datas loaded.");

    //texture_datas_futures.clear();

    //DZTextureArray tex_array = renderer.createTextureArray(texture_datas);

    //DZTexture tex = renderer.createTexture(texture_datas[2]);

    // CREATE WORLD

    World world {
        Scene(renderer, sand_pipeline),
        {},
        {}
    };

    // SET WORLD SYSTEMS

    world.game_systems.push_back(&GameSystem::inputActions);
    world.game_systems.push_back(&GameSystem::updatePlayer);

    world.render_systems.push_back(&RenderSystem::updateData);
    
    SDL_Event e;
    InputState input;
    double delta_time = 0.0;
    double elapsed_time = 0.0;

    // CREATE Player


    while(true)
    {
        elapsed_time += delta_time;

        const auto frame_start = std::chrono::steady_clock::now();

        input.update();

        s32 window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);
        //SDL_WarpMouseInWindow(window, window_width/2.f, window_height/2.f);

        glm::vec2 screen_dim(
                (float) window_width, 
                (float) window_height
            );

        if (input.quit)
            goto quit;

        renderer.waitForRenderFinish();

        for (auto system : world.game_systems)
            system(renderer, world.scene, input, delta_time);
   
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

