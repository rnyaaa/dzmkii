#include "renderer.h"
#include "logger.h"

Renderer::Renderer(SDL_Window *window, std::string shader_src)
{
    sdl_renderer = SDL_CreateRenderer(
            window, 
            -1, 
            SDL_RENDERER_PRESENTVSYNC
        );

    swapchain = (CA::MetalLayer *) SDL_RenderGetMetalLayer(sdl_renderer);

    device = swapchain->device();

    queue = device->newCommandQueue();

    using NS::StringEncoding::UTF8StringEncoding;

    NS::Error* error = nullptr;
    MTL::Library *library = device->newLibrary(
            NS::String::string(shader_src.c_str(), UTF8StringEncoding),
            nullptr,
            &error
        );

    if (library == nullptr)
    {
        Log::error(
                "Error initializing shader library\n\t%s\n", 
                error->localizedDescription()->utf8String()
            );
    }

    MTL::Function *vertex_fn = library->newFunction(
            NS::String::string("vertexMain", UTF8StringEncoding)
        );

    MTL::Function *frag_fn = library->newFunction(
            NS::String::string("fragmentMain", UTF8StringEncoding)
        );

    auto pipeline_desc = MTL::RenderPipelineDescriptor::alloc()->init();
    pipeline_desc->setVertexFunction(vertex_fn);
    pipeline_desc->setFragmentFunction(frag_fn);
    pipeline_desc
        ->colorAttachments()
        ->object(0)
        ->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);

    pipeline_state = device->newRenderPipelineState(pipeline_desc, &error);

    if (pipeline_state == nullptr)
    {
        Log::error(
                "Error initializing pipeline state\n\t%s\n", 
                error->localizedDescription()->utf8String()
            );
    }

    vertex_fn->release();
    frag_fn->release();
    pipeline_desc->release();
    library->release();

    camera_buffer = device->newBuffer(
            sizeof(CameraData),
            MTL::ResourceStorageModeShared
        );

    clear_color = MTL::ClearColor(1.0, 0.0, 1.0, 1.0);
}

Renderer::~Renderer()
{
    pipeline_state->release();
    queue->release();
    device->release();

    SDL_DestroyRenderer(sdl_renderer);
}

void Renderer::draw(
        Camera camera, 
        glm::vec2 screen_dim, 
        std::vector<Mesh> meshes
    )
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

    encoder->setRenderPipelineState(pipeline_state);
    
    // Update Camera for frame
    CameraData camera_data = camera.getCameraData(screen_dim);
    memcpy(camera_buffer->contents(), &camera_data, sizeof(CameraData));
    camera_buffer->didModifyRange(NS::Range::Make(0, sizeof(Camera)));
    encoder->setVertexBuffer(camera_buffer, 0, 2);

    for (auto &mesh : meshes)
    {
        encoder->setVertexBuffer(mesh.vertices_buffer, 0, 0);
        encoder->setVertexBuffer(mesh.colors_buffer, 0, 1);
        encoder->drawPrimitives(
                MTL::PrimitiveType::PrimitiveTypeTriangle,
                NS::UInteger(0),
                NS::UInteger(mesh.num_vertices)
            );
    }

    encoder->endEncoding();

    buffer->presentDrawable(surface);
    buffer->commit();

    auto_release_pool->release();

}
