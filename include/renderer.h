#ifndef _RENDERER_H
#define _RENDERER_H

#include <string>
#include <vector>

#include <SDL_render.h>

#include <QuartzCore/QuartzCore.hpp>

#include <Metal/Metal.hpp>
#include <Metal/MTLBuffer.hpp>
#include <Metal/MTLRenderCommandEncoder.hpp>
#include <Metal/MTLResource.hpp>
#include <Metal/MTLTexture.hpp>
#include <Metal/MTLEvent.hpp>

#include "mesh.h"
#include "camera.h"
#include "sun.h"
#include "texture.h"
#include "window.h"

#define EVENT_INIT               0
#define EVENT_WAITING_FOR_RENDER 1
#define EVENT_RENDER_FINISH      2

#define DEFAULT_PIXEL_FORMAT MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB

typedef size_t DZMesh;
typedef size_t DZBuffer;
typedef size_t DZTexture;
typedef size_t DZTextureArray;
typedef size_t DZShader;
typedef size_t DZPipeline;

// TODO: Assumes 64 bit size_t, this is to avoid
//       optionals because Ronja doesn't like good code
//
//       Another probably more sane option is to reserve
//       index 0 for any registered asset with the renderer
//       with some sort of ERROR asset like in source eng
const size_t DZInvalid = 0xffffffffdeadc0de;

enum class ShaderStage
{
    VERTEX, FRAGMENT
};

enum class StorageMode
{
    SHARED, MANAGED, PRIVATE
};

template<typename T>
struct Binding
{
    ShaderStage shader_stage;
    T resource;
    u32 binding;

    static Binding Fragment(T resource, u32 binding)
    {
        return Binding { ShaderStage::FRAGMENT, resource, binding };
    }

    static Binding Vertex(T resource, u32 binding)
    {
        return Binding { ShaderStage::VERTEX, resource, binding };
    }
};

struct DZRenderCommand
{
    enum RenderCommandType
    {
        SET_PIPELINE,
        SET_CLEAR_COLOR,
        BIND_BUFFER,
        BIND_TEXTURE,
        DRAW_MESH
    } type;

    union 
    {
        glm::vec3 clear_color;
        Binding<DZBuffer>  buffer_binding;
        Binding<DZTexture> texture_binding;
        DZTexture texture;
        DZMesh mesh;
        DZPipeline pipeline;
    };

    DZRenderCommand() {}

    static DZRenderCommand SetPipeline(DZPipeline pipeline)
    {
        DZRenderCommand ret;
        ret.type = SET_PIPELINE;
        ret.pipeline = pipeline;
        return ret;
    }

    static DZRenderCommand SetClearColor(glm::vec3 clear_color)
    {
        DZRenderCommand ret;
        ret.type = SET_CLEAR_COLOR;
        ret.clear_color = clear_color;
        return ret;
    }

    static DZRenderCommand BindBuffer(Binding<DZBuffer> buffer_binding)
    {
        DZRenderCommand ret;
        ret.type = BIND_BUFFER;
        ret.buffer_binding = buffer_binding;
        return ret;
    }

    static DZRenderCommand BindTexture(Binding<DZTexture> texture_binding)
    {
        DZRenderCommand ret;
        ret.type = BIND_TEXTURE;
        ret.texture_binding = texture_binding;
        return ret;
    }

    static DZRenderCommand DrawMesh(DZMesh mesh)
    {
        DZRenderCommand ret;
        ret.type = DRAW_MESH;
        ret.mesh = mesh;
        return ret;
    }

};

struct DZRenderer 
{
    SDL_Renderer *sdl_renderer;

    CA::MetalLayer *swapchain;

    MTL::Device *device;
    MTL::CommandQueue *queue;

    MTL::SharedEvent *render_event;

    MTL::ClearColor clear_color;

    std::vector<DZRenderCommand> command_queue;

    // SOA meshes
    struct 
    {
        DZMesh num = 0;
        std::vector<u32> num_elements;
        std::vector<MTL::PrimitiveType> primitive_type;
        std::vector<MTL::Buffer *> vertex;
        std::vector<MTL::Buffer *> index;
    } mesh_buffers;

    std::vector<MTL::Function *> shaders;
    std::vector<MTL::RenderPipelineState *> pipelines;

    std::vector<MTL::Buffer *> general_buffers;

    std::vector<MTL::Texture *> textures;
    std::vector<MTL::Texture *> texture_arrays;

    MTL::SamplerState *sampler_state;

    DZRenderer(DZWindow &window);

    ~DZRenderer();

    void waitForRenderFinish();

    void enqueueCommand(DZRenderCommand command);
    void executeCommandQueue();

    std::vector<DZShader> compileShaders(std::string shader_src, std::vector<std::string> main_fns);

    DZPipeline createPipeline(
            DZShader vertex_shader,
            DZShader fragment_shader
        );

    std::vector<DZMesh> createMeshes(const std::vector<MeshData> &mesh_datas);
    DZMesh createMesh(const MeshData &mesh_data);

    DZBuffer createBufferOfSize(size_t size, StorageMode mode = StorageMode::SHARED);
    void setBufferOfSize(DZBuffer buffer, void *data, size_t size);

    DZTexture createTexture(TextureData &texture_data);

    DZTextureArray createTextureArray(
            std::vector<TextureData> texture_datas
        );

private:
    template <typename T>
    MTL::Buffer* newBufferFromData(
            std::vector<T> data, 
            MTL::ResourceOptions options 
                = MTL::ResourceStorageModeManaged
        );

    // Remove copy constructor
    DZRenderer(const DZRenderer&) = delete;
};

#endif // _RENDERER_H
