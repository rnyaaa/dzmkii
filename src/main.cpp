#include <SDL_events.h>
#include <SDL_metal.h>
#include <SDL_render.h>
#include <SDL_scancode.h>
#include <SDL_video.h>
#include <SDL.h>

#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <simd/simd.h>

#include "chunk.h"
#include "common.h"
#include "logger.h"
#include "renderer.h"
#include "camera.h"
#include "asset.h"

#define SDL_ERR(msg) \
    printf("[ERROR] %s\n\t%s\n", msg, SDL_GetError())

void movement(Camera &camera, const u8 *key_state);


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
            640 * 2,
            480 * 2,
            SDL_WINDOW_ALLOW_HIGHDPI
        );

    if (window == NULL)
        SDL_ERR("SDL_Window could not be created!");

    std::ifstream basic_shader_file("shaders/basic_shader.metal");
    std::stringstream basic_shader_stream;
    basic_shader_stream << basic_shader_file.rdbuf();

    DZRenderer renderer(
            window, 
            basic_shader_stream.str()
        );

    AssetManager ass_man;

    ass_man.addSearchDirectory("resources");

    std::optional<TextureData> td = ass_man.getTexture("default_ground.png");

    DZTexture tex = renderer.registerTexture(td.value());

    Chunk chunker(glm::vec2(0.0f,0.0f));

    std::vector<MeshData> mesh_datas = {
        MeshData::genMeshFromTiles(chunker.tiles),
    };

    std::vector<DZMesh> meshes
        = renderer.registerMeshes(mesh_datas);

    Camera camera;
    Sun sun({1.0f, 1.0f, 1.0f});

    const u8 *key_state = SDL_GetKeyboardState(nullptr);

    SDL_Event e;
    while(true)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) goto quit;

            if(e.type == SDL_MOUSEWHEEL)
            {
                if(e.wheel.y > 0)
                {
                    camera.zoom(-1.0f);
                }
                else 
                {
                    camera.zoom(1.0f);
                }
            }
        }

        if (key_state[SDL_SCANCODE_ESCAPE])
        {
            goto quit;
        }

        movement(camera, key_state);

        s32 window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);
        glm::vec2 screen_dim(
                (float) window_width, 
                (float) (window_height)
            );

        DZUniforms uniforms = {
            camera.getCameraData(screen_dim),
            sun.dir,
        };
        renderer.updateUniforms(uniforms);
        renderer.draw(meshes);
    }

quit:

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void movement(Camera &camera, const u8 *key_state)
{
    if(key_state[SDL_SCANCODE_W])
        camera.move(glm::vec3(-1.0f, -1.0f, 0.0f));
    if(key_state[SDL_SCANCODE_A])
        camera.move(glm::vec3(1.0f, -1.0f, 0.0f));
    if(key_state[SDL_SCANCODE_S])
        camera.move(glm::vec3(1.0f, 1.0f, 0.0f));
    if(key_state[SDL_SCANCODE_D])
        camera.move(glm::vec3(-1.0f, 1.0f, 0.0f));
}
