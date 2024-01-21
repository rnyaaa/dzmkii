#include <SDL_events.h>
#include <SDL_metal.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <SDL.h>

#include <chrono>
#include <iostream>
#include <stdio.h>

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <simd/simd.h>

#include <renderer.h>

#define SDL_ERR(msg) \
    printf("[ERROR] %s\n\t%s\n", msg, SDL_GetError())

int main(int argc, char *argv[])
{

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        SDL_ERR("SDL could not initialize!");

    SDL_Window *window = SDL_CreateWindow(
            "SDL2Test",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            640,
            480,
            SDL_WINDOW_ALLOW_HIGHDPI
        );

    if (window == NULL)
        SDL_ERR("SDL_Window could not be created!");
    
    Renderer renderer(window);

    SDL_Event e;
    while(true)
    {
        const auto start{std::chrono::steady_clock::now()};
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) goto quit;
            if (e.type == SDL_KEYDOWN) goto quit;
        }

        renderer.draw();
    }

quit:

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
