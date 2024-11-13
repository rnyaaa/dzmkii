#include <unistd.h>

#include <Foundation/NSTypes.hpp>
#include <Metal/MTLRenderCommandEncoder.hpp>
#include <Metal/MTLRenderPipeline.hpp>
#include <Metal/MTLResource.hpp>
#include <Metal/MTLSampler.hpp>
#include <Metal/MTLStageInputOutputDescriptor.hpp>
#include <Metal/MTLTexture.hpp>

#include "logger.h"

#include "renderer.h"

DZRenderer::DZRenderer(DZWindow &window)
{
    sdl_renderer = SDL_CreateRenderer(
            window.sdl_window, 
            -1, 
            SDL_RENDERER_PRESENTVSYNC
        );

    swapchain = (CA::MetalLayer *) SDL_RenderGetMetalLayer(sdl_renderer);

    device = swapchain->device();

    render_event = device->newSharedEvent();

    render_event->signaledValue();
    render_event->setSignaledValue(EVENT_INIT);

    queue = device->newCommandQueue();

    // TODO: What is this doing here?
    auto sampler_desc = MTL::SamplerDescriptor::alloc()->init();
    sampler_desc->setRAddressMode(MTL::SamplerAddressMode::SamplerAddressModeRepeat);
    sampler_desc->setSAddressMode(MTL::SamplerAddressMode::SamplerAddressModeRepeat);
    sampler_desc->setTAddressMode(MTL::SamplerAddressMode::SamplerAddressModeRepeat);

    sampler_state = device->newSamplerState(sampler_desc);

    sampler_desc->release();

    clear_color = MTL::ClearColor(1.0, 0.0, 1.0, 1.0);
}

DZRenderer::~DZRenderer()
{
    // TODO: Release all managed resources (pointers in vectors)

    queue->release();
    device->release();

    SDL_DestroyRenderer(sdl_renderer);
}

void DZRenderer::waitForRenderFinish()
{
    if (render_event->signaledValue() < EVENT_WAITING_FOR_RENDER)
        return;

    // TODO: use mutex, conditional_variable and a dispatch queue
    while (render_event->signaledValue() != EVENT_RENDER_FINISH);

    render_event->release();

    render_event = device->newSharedEvent();
    render_event->setSignaledValue(EVENT_INIT);
}

void DZRenderer::enqueueCommand(DZRenderCommand command)
{
    this->command_queue.push_back(command);
}

void DZRenderer::executeCommandQueue()
{
    NS::AutoreleasePool* auto_release_pool 
        = NS::AutoreleasePool::alloc()->init();

    render_event->setSignaledValue(EVENT_WAITING_FOR_RENDER);

    CA::MetalDrawable *surface = swapchain->nextDrawable();
    auto pass_descriptor 
        = MTL::RenderPassDescriptor::alloc()->init();
    auto attachment 
        = pass_descriptor->colorAttachments()->object(0);
    attachment->setClearColor(clear_color);
    attachment->setLoadAction(MTL::LoadActionClear);
    attachment->setTexture(surface->texture());

    // TODO: Verify integrity of command queue
    //       ... Commands may possibly be in an invalid
    //       order, etc.

    auto buffer = queue->commandBuffer();
    auto encoder 
        = buffer->renderCommandEncoder(pass_descriptor);

    for (const auto &command : command_queue)
    {
        if (command.type == DZRenderCommand::SET_PIPELINE)
        {
            encoder->setRenderPipelineState(
                this->pipelines[command.pipeline]);

            // TODO: Make programmable
            encoder->setFragmentSamplerState(sampler_state, 0);

            // Make better texture system lol
            if (!texture_arrays.empty())
            {
                encoder->setFragmentTexture(this->texture_arrays[0], 0u);
            }
        }
        else if (command.type == DZRenderCommand::SET_CLEAR_COLOR)
        {
            glm::vec3 col = command.clear_color;
            attachment->setClearColor(
                MTL::ClearColor(col.r, col.g, col.b, 1.0));
        }
        else if (command.type == DZRenderCommand::BIND_BUFFER)
        {
            Binding<DZBuffer> binding = command.buffer_binding;

            MTL::Buffer *buf = this->general_buffers[
                    binding.resource
                ];

            switch (binding.shader_stage)
            {
                case ShaderStage::VERTEX:
                    switch (binding.binding)
                    {
                        case 1:
                            Log::warning("Attempting to bind "
                                         "to reserved binding "
                                         "in vertex shader");
                            break;
                        default:
                            encoder
                                ->setVertexBuffer(
                                        buf,
                                        0,
                                        binding.binding
                                    );
                            break;
                    }
                    break;
                case ShaderStage::FRAGMENT:
                    encoder
                        ->setFragmentBuffer(
                                buf,
                                0,
                                binding.binding
                            );
                    break;
                default:
                    Log::warning("Bogus shader stage during "
                                 "buffer binding?");
                    break;
            }
        }
        else if (command.type == DZRenderCommand::BIND_TEXTURE)
        {
            Binding<DZTexture> binding = command.texture_binding;

            MTL::Texture *tex = this->textures[
                    binding.resource
                ];

            switch (binding.shader_stage)
            {
                case ShaderStage::VERTEX:
                    switch (binding.binding)
                    {
                        case 1:
                            Log::warning("Attempting to bind "
                                         "to reserved binding "
                                         "in vertex shader");
                            break;
                        default:
                            encoder
                                ->setVertexTexture(
                                        tex,
                                        binding.binding
                                    );
                            break;
                    }
                    break;
                case ShaderStage::FRAGMENT:
                    encoder
                        ->setFragmentTexture(
                                tex,
                                binding.binding
                            );
                    break;
                default:
                    Log::warning("Bogus shader stage during "
                                 "buffer binding?");
                    break;
            }
        }
        else if (command.type == DZRenderCommand::DRAW_MESH)
        {
            encoder->setVertexBuffer(
                    mesh_buffers.vertex[command.mesh], 0, 1);

            if (mesh_buffers.index[command.mesh])
            {
                encoder->drawIndexedPrimitives(
                            mesh_buffers.primitive_type[command.mesh],
                            mesh_buffers.num_elements[command.mesh],
                            MTL::IndexTypeUInt32,
                            mesh_buffers.index[command.mesh],
                            NS::UInteger(0)
                        );
            }
            else
            {
                encoder->drawPrimitives(
                        mesh_buffers.primitive_type[command.mesh],
                        NS::UInteger(0),
                        mesh_buffers.num_elements[command.mesh]
                    );
            }
        }
        else
        {
            Log::warning("Invalid render command encountered");
        }
    }

    encoder->endEncoding();

    buffer->presentDrawable(surface);

    buffer->encodeSignalEvent(render_event, EVENT_RENDER_FINISH);

    buffer->commit();

    pass_descriptor->release();
    auto_release_pool->release();

    command_queue.clear();
}

std::vector<DZShader> DZRenderer::compileShaders(
        std::string shader_src, std::vector<std::string> main_fns
    )
{
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
                "Error initializing shader library\n%s\n", 
                error->localizedDescription()->utf8String()
            );
        return {};
    }

    std::vector<DZShader> ret = {};

    for (auto &fn : main_fns)
    {
        MTL::Function *mtl_fn = library->newFunction(
            NS::String::string(fn.c_str(), UTF8StringEncoding)
        );
        // TODO: Successful?
        ret.push_back(this->shaders.size());
        this->shaders.push_back(mtl_fn);
    }

    library->release();

    return ret;
}

DZPipeline DZRenderer::createPipeline(
        DZShader vertex_shader, DZShader fragment_shader
    )
{
    NS::Error* error = nullptr;

    auto pipeline_desc 
        = MTL::RenderPipelineDescriptor::alloc()->init();
    pipeline_desc->setVertexFunction(shaders[vertex_shader]);
    pipeline_desc->setFragmentFunction(shaders[fragment_shader]);
    pipeline_desc
        ->colorAttachments()
        ->object(0)
        ->setPixelFormat(DEFAULT_PIXEL_FORMAT);
    pipeline_desc
        ->colorAttachments()
        ->object(0)
        ->setBlendingEnabled(true);
    pipeline_desc
        ->colorAttachments()
        ->object(0)
        ->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    pipeline_desc
        ->colorAttachments()
        ->object(0)
        ->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    pipeline_desc
        ->colorAttachments()
        ->object(0)
        ->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
    pipeline_desc
        ->colorAttachments()
        ->object(0)
        ->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);

    MTL::RenderPipelineState *pipeline_state = device->newRenderPipelineState(pipeline_desc, &error);

    if (pipeline_state == nullptr)
    {
        Log::error(
                "Error initializing "
                "pipeline state\n%s\n", 
                error->localizedDescription()
                    ->utf8String()
            );

        pipeline_desc->release();

        return DZInvalid;
    }

    DZPipeline ret = this->pipelines.size();
    this->pipelines.push_back(pipeline_state);

    return ret;
}

std::vector<DZMesh> DZRenderer::createMeshes(
        const std::vector<MeshData> &mesh_datas
    )
{
    std::vector<DZMesh> ret;
    for (auto &mesh_data : mesh_datas)
        ret.push_back(createMesh(mesh_data));
    return ret;
}

DZMesh DZRenderer::createMesh(const MeshData &mesh_data)
{
    u32 num_elements = 
        !mesh_data.indices.empty() 
        ? mesh_data.indices.size() 
        : mesh_data.vertices.size();

    mesh_buffers.num_elements.push_back(num_elements);

    MTL::PrimitiveType primitive_type;

    switch(mesh_data.primitive_type)
    {
        case PrimitiveType::LINE:
            primitive_type = MTL::PrimitiveTypeLine;
            break;
        case PrimitiveType::LINE_STRIP:
            primitive_type = MTL::PrimitiveTypeLineStrip;
            break;
        case PrimitiveType::TRIANGLE:
            primitive_type = MTL::PrimitiveTypeTriangle;
            break;
        case PrimitiveType::TRIANGLE_STRIP:
            primitive_type = MTL::PrimitiveTypeTriangleStrip;
            break;
        case PrimitiveType::POINT:
            primitive_type = MTL::PrimitiveTypePoint;
            break;
        default:
            Log::error("Unknown primitive type passed to createMesh, MeshData corrupted?");
            break;
    }

    mesh_buffers.primitive_type.push_back(primitive_type);

    mesh_buffers.vertex.push_back(
            newBufferFromData(mesh_data.vertices)
        );

    mesh_buffers.index
        .push_back(
                !mesh_data.indices.empty()
                ? newBufferFromData(mesh_data.indices)
                : nullptr
            );

    return mesh_buffers.num++;
}

DZBuffer DZRenderer::createBufferOfSize(size_t size, StorageMode mode)
{
    MTL::ResourceOptions storage_mode;
    switch (mode)
    {
        case StorageMode::MANAGED:
            storage_mode = MTL::ResourceStorageModeManaged;
            break;
        case StorageMode::SHARED:
            storage_mode = MTL::ResourceStorageModeShared;
            break;
        case StorageMode::PRIVATE:
            storage_mode = MTL::ResourceStorageModePrivate;
            break;
        default:
            Log::warning("Invalid storage mode requested.");
            storage_mode = MTL::ResourceStorageModeShared;
            break;
    }
    MTL::Buffer *new_buffer = this->device->newBuffer(
            size,
            storage_mode
        );

    DZBuffer ret = this->general_buffers.size();
    this->general_buffers.push_back(new_buffer);

    return ret;
}

void DZRenderer::setBufferOfSize(DZBuffer buffer, void *data, size_t size)
{
    MTL::Buffer *mtl_buffer = this->general_buffers[buffer];
    memcpy(mtl_buffer->contents(), data, size);
    mtl_buffer->didModifyRange(NS::Range::Make(0, size));
}

DZTextureArray DZRenderer::createTextureArray
    (
        std::vector<TextureData> texture_datas
    )
{
    Log::verbose("Creating TextureArray...");
    Log::verbose("\tNum textures: %d", texture_datas.size());

    if (texture_datas.empty())
    {
        Log::error("No textures provided for texture array.");
        return DZInvalid;
    }

    u32 tex_num_channels  = texture_datas[0].num_channels;
    u32 tex_width         = texture_datas[0].width;
    u32 tex_height        = texture_datas[0].height;

    for (auto &td : texture_datas)
    {
        if (td.width != tex_width || td.height != tex_height)
        {
            Log::error("Not all textures provided for texture "
                       "array have the same dimensions.");
            return DZInvalid;
        }
        if (td.num_channels != tex_num_channels)
        {
            Log::error("Not all textures provided for texture "
                       "array have the same number of channels.");
            return DZInvalid;
        }
    }

    Log::verbose("\tCreating Texture Descriptor");
    MTL::TextureDescriptor *td = MTL::TextureDescriptor::alloc()
        ->init();
    td->setTextureType(MTL::TextureType::TextureType2DArray);
    td->setPixelFormat(DEFAULT_PIXEL_FORMAT);
    td->setWidth(tex_width);
    td->setHeight(tex_height);
    td->setArrayLength(texture_datas.size());

    Log::verbose("\tCreating GPU Texture");
    MTL::Texture *texture = this->device->newTexture(td);

    Log::verbose("\tWriting to Texture");

    u32 bytes_per_row = tex_width * tex_num_channels;
    u32 bytes_per_tex = tex_height * bytes_per_row;

    for (u32 slice = 0; slice < texture_datas.size(); slice++)
    {
        MTL::Region region(0u, 0u, tex_width, tex_height);
        texture->replaceRegion(
                region, 
                0, 
                slice,
                texture_datas[slice].data.data(), 
                bytes_per_row,
                bytes_per_tex
            );
    }

    DZTextureArray ret = this->texture_arrays.size();

    this->texture_arrays.push_back(texture);

    Log::verbose("\tTextureArray created");

    return ret;
}

DZTexture DZRenderer::createTexture(TextureData &texture_data)
{
    Log::verbose("Creating Texture...");
    DZTexture ret = this->textures.size();

    Log::verbose("\tCreating Texture Descriptor");
    MTL::TextureDescriptor *td = MTL::TextureDescriptor::alloc()
        ->init();
    td->setPixelFormat(DEFAULT_PIXEL_FORMAT);
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

    Log::verbose("\tTexture createed");

    return ret;
}

template <typename T>
MTL::Buffer* DZRenderer::newBufferFromData
    (
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
