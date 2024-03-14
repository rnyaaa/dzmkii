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
#include "Metal/MTLRenderCommandEncoder.hpp"
#include "Metal/MTLResource.hpp"
#include "Metal/MTLTexture.hpp"
#include "mesh.h"
#include "camera.h"
#include "sun.h"
#include "texture.h"

typedef size_t DZMesh;
typedef size_t DZTexture;

struct DZUniforms
{
    CameraData camera;
    glm::vec4 sun;
};

struct DZRenderer 
{

    SDL_Renderer *sdl_renderer;

    CA::MetalLayer *swapchain;

    MTL::Device *device;
    MTL::CommandQueue *queue;
    MTL::RenderPipelineState *pipeline_state;

    MTL::Buffer *uniforms_buffer;

    MTL::ClearColor clear_color;

    struct 
    {
        DZMesh num = 0;
        std::vector<u32> num_elements;
        std::vector<MTL::PrimitiveType> primitive_type;
        std::vector<MTL::Buffer*> vertex;
        std::vector<MTL::Buffer*> index;
    } buffers;

    std::vector<MTL::Texture*> textures;

    MTL::SamplerState *sampler_state;

    DZRenderer(
            SDL_Window *window, 
            std ::string shader_src
        );
    ~DZRenderer();

    void draw(
            std::vector<DZMesh>
        );

    void updateUniforms(const DZUniforms &uniforms);

    std::vector<DZMesh> registerMeshes(
            std::vector<MeshData> mesh_datas
        );
    DZMesh registerMesh(MeshData &mesh_data);
    
    DZTexture registerTexture(TextureData &texture_data);
    

private:
    template <typename T>
    MTL::Buffer* newBufferFromData(
            std::vector<T> data, 
            MTL::ResourceOptions options 
                = MTL::ResourceStorageModeManaged
        );
};

#endif // _RENDERER_H
