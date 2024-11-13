#ifndef _WINDOW_H
#define _WINDOW_H

#include <SDL_video.h>
#include <SDL.h>

#include "geometry.h"

struct DZWindow
{
    SDL_Window *sdl_window;

    DZWindow(std::string name, u32 width, u32 height)
    {
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            Log::error("SDL could not initialize:\n\t%s", SDL_GetError());
            exit(1);
        }

        SDL_Window *sdl_window = SDL_CreateWindow(
                    name.c_str(),
                    SDL_WINDOWPOS_UNDEFINED,
                    SDL_WINDOWPOS_UNDEFINED,
                    width,
                    height,
                    SDL_WINDOW_ALLOW_HIGHDPI
                    | SDL_WINDOW_BORDERLESS
                );

        if (sdl_window == NULL)
        {
            Log::error("SDL_Window could not be created:\n\t%s", SDL_GetError());
            exit(1);
        }
    }

    ~DZWindow()
    {
        SDL_DestroyWindow(sdl_window);
    }

    v2i getWindowSize()
    {
        v2i ret;
        SDL_GetWindowSize(sdl_window, &ret.x, &ret.y);
        return ret;
    }

};

#endif // _WINDOW_H
