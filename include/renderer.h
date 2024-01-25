#ifndef _RENDERER_H
#define _RENDERER_H

#include <string>
#include <vector>

#include <SDL_events.h>
#include <SDL_render.h>
#include <SDL.h>

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "Metal/MTLBuffer.hpp"
#include "mesh.h"
#include "camera.h"

struct Renderer 
{
    SDL_Renderer *sdl_renderer;

    CA::MetalLayer *swapchain;

    MTL::Device *device;
    MTL::CommandQueue *queue;
    MTL::RenderPipelineState *pipeline_state;

    MTL::Buffer *camera_buffer;

    MTL::ClearColor clear_color;

    Renderer(SDL_Window *window, std::string shader_src);
    ~Renderer();
    void draw(Camera camera, glm::vec2 screen_dim, std::vector<Mesh>);
};

#endif // _RENDERER_H
