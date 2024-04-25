#include <SDL_events.h>
#include <SDL_metal.h>
#include <SDL_render.h>
#include <SDL_scancode.h>
#include <SDL_video.h>
#include <SDL.h>

#include <chrono>
#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdio.h>

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <simd/simd.h>

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

#define SDL_ERR(msg) \
    printf("[ERROR] %s\n\t%s\n", msg, SDL_GetError())

void movement(Camera &camera, const u8 *key_state);

struct GlobalUniforms
{
    CameraData camera;
    glm::vec4 sun;
};

struct Lol
{
    char *str;
    int x, y;

    Lol(char *str, int x, int y) : str(str), x(x), y(y) { }

    void tick(DZTermRenderer &term)
    {
        term.setCursor(x, y);
        term.setStroke(Color::WHITE, Color::BLACK, '#');
        term.setFill(Color::WHITE, Color::BLACK, '-');
        term.rect(strlen(str) + 4, 5);
        term.setCursor(x + 2, y + 2);
        term.write(str);
        x++;
        x %= 150;
    }
};

std::optional<std::pair<glm::vec2, glm::vec2>> rect_intersect(glm::vec2 pos1, glm::vec2 dim1, glm::vec2 pos2, glm::vec2 dim2)
{
    float min_x, min_y, max_x, max_y;

    min_x = std::max(pos1.x         , pos2.x);
    max_x = std::min(pos1.x + dim1.x, pos2.x + dim2.x);

    min_y = std::max(pos1.y         , pos2.y);
    max_y = std::min(pos1.y + dim1.y, pos2.y + dim2.y);

    if (min_x >= max_x || min_y >= max_y) 
        return std::nullopt;

    return std::make_pair(
            glm::vec2(min_x, min_y), 
            glm::vec2(max_x, max_y)
        );
}

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

    std::ifstream basic_shader_file("shaders/basic_shader.metal");
    std::stringstream basic_shader_stream;
    basic_shader_stream << basic_shader_file.rdbuf();

    std::ifstream terrain_shader_file("shaders/terrain_shader.metal");
    std::stringstream terrain_shader_stream;
    terrain_shader_stream << terrain_shader_file.rdbuf();

    DZRenderer renderer(window);

    std::vector<DZShader> terrain_shaders = renderer.compileShaders(terrain_shader_stream.str(), {"vertexMain", "fragmentMain"});
    std::vector<DZShader> basic_shaders = renderer.compileShaders(basic_shader_stream.str(), {"vertexMain", "fragmentMain"});


    DZPipeline terrain_pipeline = renderer.createPipeline(terrain_shaders[0], terrain_shaders[1]);
    DZPipeline basic_pipeline = renderer.createPipeline(basic_shaders[0], basic_shaders[1]);

    AssetManager ass_man;

    ass_man.addSearchDirectory("resources");
    ass_man.addSearchDirectory("resources/new");
    ass_man.addSearchDirectory("resources/new/1-alldirt-negnegneg");
    ass_man.addSearchDirectory("resources/new/2-moredirt-negneg");
    ass_man.addSearchDirectory("resources/new/3-bitdirt-neg");
    ass_man.addSearchDirectory("resources/new/4-grassy-mid");
    ass_man.addSearchDirectory("resources/new/5-mossy-pos");
    ass_man.addSearchDirectory("resources/new/6-darkmoss-pospos");
    ass_man.addSearchDirectory("resources/new/7-undergrowth-pospospos");
 
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
        "undergrowth-normal.png"
    };

    std::vector<TextureData> texture_datas;

    for (const auto &tex_path : texture_paths)
    {
        texture_datas.push_back(ass_man.getTexture(tex_path).value());
    }

    DZTextureArray tex_array = renderer.createTextureArray(texture_datas);
    Log::verbose("Creating terrain...");
    
    Terrain terrain(renderer, 100.0f, 616u);


    Log::verbose("Generating chunk");

    terrain.createChunk(renderer, glm::vec2(0.0f, 0.0f));
    terrain.createChunk(renderer, glm::vec2(-100.0f, -100.0f));
    terrain.createChunk(renderer, glm::vec2(-100.0f, 0.0f));
    terrain.createChunk(renderer, glm::vec2(0.0f, -100.0f));
    // TODO: terrain.createVisibleChunks(camera);
    //       Create any chunk that might be visible on 
    //       the camera

    Log::verbose("Created terrain.");

    Camera camera;
    Sun sun({1.0f, 1.0f, 1.0f});

    glm::vec3 north(-1.0f, -1.0f, 0.0f);
    glm::vec3 south(1.0f, 1.0f, 0.0f);
    glm::vec3 east(-1.0f, 1.0f, 0.0f);
    glm::vec3 west(1.0f, -1.0f, 0.0f);

    DZBuffer global_uniform_buffer = 
        renderer.createBufferOfSize(sizeof(GlobalUniforms), StorageMode::SHARED);

    const u8 *key_state = SDL_GetKeyboardState(nullptr);

    DZTermRenderer term(150, 40);
    Entity los_tester;
    los_tester.line_of_sight = 15;

    SDL_Event e;
    while(true)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) 
                goto quit;

            if(e.type == SDL_MOUSEWHEEL)
                camera.zoom(e.wheel.y > 0 ? -1.0f : 1.0f);
        }

        if (key_state[SDL_SCANCODE_ESCAPE])
            goto quit;

        if (key_state[SDL_SCANCODE_C])
            terrain.chunks.clear();

        movement(camera, key_state);

        s32 window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);
        glm::vec2 screen_dim(
                (float) window_width, 
                (float) (window_height)
            );

        GlobalUniforms uniforms = {
            camera.getCameraData(screen_dim),
            sun.dir,
        };

        renderer.setBufferOfSize(global_uniform_buffer, &uniforms, sizeof(GlobalUniforms));

        renderer.enqueueCommand(
                DZRenderCommand::SetPipeline(terrain_pipeline));

        renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    DZBufferBinding::Fragment(global_uniform_buffer, 0)
                    ));
        renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    DZBufferBinding::Vertex(global_uniform_buffer, 0)
                    ));

        std::array<glm::vec3, 4> frustrum_bounds = camera.getFrustrumBounds(screen_dim);

        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                terrain.createChunk(renderer, camera.target.xy() + glm::vec2(i * 100.0f, j * 100.0f));
            }
        }
        std::array<Chunk*, 9> visible;
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                visible[(j+1) * 3 + (i + 1)] = terrain.getChunkFromPos(v2f{camera.target.x + terrain.chunk_size * i, 
                                                                            camera.target.y + terrain.chunk_size * j});
            }
        }

        for (int i = 0; i < 9; i++)
        {
            for(auto &los_index : visible[i]->los_indices)
            {
                if(los_index == 2) los_index = 1;
            }
            visible[i]->updateUniforms(renderer, i);
        }
        //Log::verbose("%f %f %f", camera.target.x, camera.target.y, camera.target.z);
        los_tester.transform.pos = camera.target;
        terrain.updateLOS(los_tester.transform.pos.xy(), los_tester.line_of_sight);

        terrain.updateUniforms(renderer, visible);
        renderer.enqueueCommand(
                    DZRenderCommand::BindBuffer(
                        DZBufferBinding::Fragment(
                            terrain.terrain_uniform_buffer, 2)
                        )
                );

        for (int i = 0; i < 9; i++)
        {
            visible[i]->updateUniforms(renderer, i);
            renderer.enqueueCommand(
                    DZRenderCommand::BindBuffer(
                        DZBufferBinding::Vertex(
                            visible[i]->local_uniforms_buffer, 2)));

            renderer.enqueueCommand(
                    DZRenderCommand::BindBuffer(
                        DZBufferBinding::Fragment(
                            visible[i]->local_uniforms_buffer, 1)));

            renderer.enqueueCommand(
                     DZRenderCommand::DrawMesh(
                         visible[i]->mesh));
        }

        renderer.executeCommandQueue();

        //term.clear();

        //terrain.termRender(term, camera.taDZBuffer bufferrget.xy());

        //term.display()
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
