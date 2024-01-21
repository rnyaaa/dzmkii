#include "renderer.h"

Renderer::Renderer(SDL_Window *window)
{
    sdl_renderer = SDL_CreateRenderer(
            window, 
            -1, 
            SDL_RENDERER_PRESENTVSYNC
        );
    swapchain 
        = (CA::MetalLayer *) SDL_RenderGetMetalLayer(sdl_renderer);
    device = swapchain->device();
    queue = device->newCommandQueue();
    clear_color = MTL::ClearColor(1.0, 0.0, 1.0, 1.0);
}

Renderer::~Renderer()
{
    SDL_DestroyRenderer(sdl_renderer);
}

void Renderer::draw()
{
    NS::AutoreleasePool* auto_release_pool 
        = NS::AutoreleasePool::alloc()->init();

    CA::MetalDrawable *surface = swapchain->nextDrawable();
    auto pass_descriptor 
        = MTL::RenderPassDescriptor::alloc()->init();
    auto attachment 
        = pass_descriptor->colorAttachments()->object(0);
    attachment->setClearColor(clear_color);
    attachment->setLoadAction(MTL::LoadActionClear);
    attachment->setTexture(surface->texture());

    auto buffer = queue->commandBuffer();
    auto encoder 
        = buffer->renderCommandEncoder(pass_descriptor);
    encoder->endEncoding();
    buffer->presentDrawable(surface);
    buffer->commit();
    auto_release_pool->release();
}
