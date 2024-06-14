#include<stdlib.h>
#include<vector>

#include "renderer.h"
#include "transform.h"

#ifndef _MODEL_H
#define _MODEL_H

struct ModelUniforms
{
    glm::mat4 model_matrix;
    bool textured;
    bool lit;
    u32 material_index;
};

struct Model
{
    DZBuffer uniform_buffer;
    std::vector<DZMesh> meshes;
    bool textured;
    bool lit;

    static Model fromMeshes(DZRenderer &renderer, const std::vector<DZMesh> &meshes)
    {
        auto uniform_buffer = 
            renderer.createBufferOfSize(
                sizeof(ModelUniforms), StorageMode::SHARED);

        return { uniform_buffer, meshes, false, false };
    }

    static Model fromMeshDatas(DZRenderer &renderer, const std::vector<MeshData> &mesh_datas)
    {
        std::vector<DZMesh> meshes;

        for (const auto &mesh : mesh_datas)
        {
            DZMesh dz_mesh = renderer.createMesh(mesh);
            meshes.push_back(dz_mesh);
        }

        return fromMeshes(renderer, meshes);
    }

    void render(DZRenderer &renderer, const Transform &transform) const
    {
        ModelUniforms uniforms {
            transform.asMat4(),
            this->lit,
            this->textured
        };

        renderer.setBufferOfSize(
                this->uniform_buffer, &uniforms, sizeof(ModelUniforms));

        renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Vertex(
                        this->uniform_buffer, 2)));

        renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    Binding<DZBuffer>::Fragment(
                        this->uniform_buffer, 1)));

        for (const auto &mesh : this->meshes)
        {
            renderer.enqueueCommand(
                    DZRenderCommand::DrawMesh(mesh));
        }
    }
};

#endif
