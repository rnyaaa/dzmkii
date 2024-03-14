#include "renderer.h"
#include "Foundation/NSTypes.hpp"
#include "Metal/MTLRenderCommandEncoder.hpp"
#include "Metal/MTLSampler.hpp"
#include "Metal/MTLTexture.hpp"
#include "logger.h"

DZRenderer::DZRenderer(SDL_Window *window, std::string shader_src)
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
            NS::String::string(
                "fragmentMain", 
                UTF8StringEncoding
            ));

    auto pipeline_desc 
        = MTL::RenderPipelineDescriptor::alloc()->init();
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
                "Error initializing "
                "pipeline state\n\t%s\n", 
                error->localizedDescription()
                    ->utf8String()
            );
    }

    auto sampler_desc = MTL::SamplerDescriptor::alloc()->init();
    sampler_desc->setRAddressMode(MTL::SamplerAddressMode::SamplerAddressModeRepeat);
    sampler_desc->setSAddressMode(MTL::SamplerAddressMode::SamplerAddressModeRepeat);
    sampler_desc->setTAddressMode(MTL::SamplerAddressMode::SamplerAddressModeRepeat);

    sampler_state = device->newSamplerState(sampler_desc);

    sampler_desc->release();
    vertex_fn->release();
    frag_fn->release();
    pipeline_desc->release();
    library->release();

    uniforms_buffer = device->newBuffer(
            sizeof(DZUniforms),
            MTL::ResourceStorageModeShared
        );

    clear_color = MTL::ClearColor(1.0, 0.0, 1.0, 1.0);
}

DZRenderer::~DZRenderer()
{
    pipeline_state->release();
    queue->release();
    device->release();

    SDL_DestroyRenderer(sdl_renderer);
}

void DZRenderer::updateUniforms(const DZUniforms &uniforms)
{
    // Update Camera for frame
    memcpy(
            uniforms_buffer->contents(), 
            &uniforms, 
            sizeof(DZUniforms)
        );
    uniforms_buffer->didModifyRange(
            NS::Range::Make(0, sizeof(DZUniforms))
        );

}

void DZRenderer::draw(
        std::vector<DZMesh> meshes
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
    
    encoder->setVertexBuffer(uniforms_buffer, 0, 1);
    encoder->setFragmentBuffer(uniforms_buffer, 0, 0);

    encoder->setFragmentSamplerState(sampler_state, 0);

    // Make better texture system lol
    if (!textures.empty())
    {
        encoder->setFragmentTexture(this->textures[0], 0u);
    }

    using MTL::PrimitiveType::PrimitiveTypeTriangle;

    for (auto &mesh : meshes)
    {
        encoder->setVertexBuffer(
                buffers.vertex[mesh], 0, 0);

        encoder->drawPrimitives(
                buffers.primitive_type[mesh],
                NS::UInteger(0),
                NS::UInteger(buffers.num_elements[mesh])
            );
    }

    encoder->endEncoding();

    buffer->presentDrawable(surface);
    buffer->commit();

    pass_descriptor->release();
    auto_release_pool->release();

}

std::vector<DZMesh> DZRenderer::registerMeshes(
        std::vector<MeshData> mesh_datas
    )
{
    std::vector<DZMesh> ret;
    for (auto &mesh_data : mesh_datas)
        ret.push_back(registerMesh(mesh_data));
    return ret;
}

DZMesh DZRenderer::registerMesh(MeshData &mesh_data)
{
    u32 num_elements = 
        !mesh_data.indices.empty() 
        ? mesh_data.indices.size() 
        : mesh_data.vertices.size();

    buffers.num_elements.push_back(num_elements);

    buffers.primitive_type.push_back(
            MTL::PrimitiveType::PrimitiveTypeTriangle
        );

    buffers.vertex.push_back(
            newBufferFromData(mesh_data.vertices)
        );

    buffers.index
        .push_back(
                !mesh_data.indices.empty()
                ? newBufferFromData(mesh_data.indices)
                : nullptr
            );

    return buffers.num++;
}

DZTexture DZRenderer::registerTexture(TextureData &texture_data)
{
    Log::verbose("Registering Texture...");
    DZTexture ret = this->textures.size();

    Log::verbose("\tCreating Texture Descriptor");
    MTL::TextureDescriptor *td = MTL::TextureDescriptor::alloc()->init();
    td->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm);
    td->setWidth(texture_data.width);
    td->setHeight(texture_data.height);

    Log::verbose("\tCreating Texture Write Region");
    MTL::Region region(0u, 0u, texture_data.width, texture_data.height);

    Log::verbose("\tCreating GPU Texture");
    MTL::Texture *texture = this->device->newTexture(td);

    Log::verbose("\tWriting to Texture");
    texture->replaceRegion(
            region, 
            0, 
            texture_data.data.data(), 
            texture_data.width * texture_data.num_channels
        );

    this->textures.push_back(texture);

    Log::verbose("\tTexture Registered");

    return ret;
}

template <typename T>
MTL::Buffer* DZRenderer::newBufferFromData(
        std::vector<T> data, 
        MTL::ResourceOptions options
    )
{
    MTL::Buffer* ret = device->newBuffer(
            data.size() * sizeof(T),
            options
        );

    memcpy(
            ret->contents(),
            data.data(),
            ret->length()
        );

    ret->didModifyRange(
            NS::Range::Make(0, ret->length())
        );

    return ret;
}
