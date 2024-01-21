#include <SDL_events.h>
#include <SDL_render.h>
#include <SDL.h>

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>


struct Renderer 
{
    SDL_Renderer *sdl_renderer;
    CA::MetalLayer *swapchain;
    MTL::Device *device;
    MTL::CommandQueue *queue;
    MTL::ClearColor clear_color;

    Renderer(SDL_Window *window);
    ~Renderer();
    void draw();
};
