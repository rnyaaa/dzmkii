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
};

struct Model
{
    Transform transform;
    DZBuffer uniform_buffer;
    std::vector<DZMesh> meshes;
    bool textured;
    bool lit;

    static Model fromMeshDatas(DZRenderer &renderer, const std::vector<MeshData> &mesh_datas)
    {
        Model model;

        for (const auto &mesh : mesh_datas)
        {
            DZMesh dz_mesh = renderer.createMesh(mesh);
            model.meshes.push_back(dz_mesh);
        }

        model.transform.pos = glm::vec3(0.0);
        model.transform.scale = glm::vec3(1.0);
        model.transform.rotation = glm::vec3(0.0);

        model.textured = false;
        model.lit = false;

        model.uniform_buffer = 
            renderer.createBufferOfSize(
                sizeof(ModelUniforms), StorageMode::SHARED);

        return model;
    }

    void render(DZRenderer &renderer)
    {
        ModelUniforms uniforms {
            this->transform.asMat4(),
            this->lit,
            this->textured
        };

        renderer.setBufferOfSize(
                this->uniform_buffer, &uniforms, sizeof(ModelUniforms));

        renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    DZBufferBinding::Vertex(
                        this->uniform_buffer, 2)));

        renderer.enqueueCommand(
                DZRenderCommand::BindBuffer(
                    DZBufferBinding::Fragment(
                        this->uniform_buffer, 1)));

        for (const auto &mesh : this->meshes)
        {
            renderer.enqueueCommand(
                    DZRenderCommand::DrawMesh(mesh));
        }
    }
};

#endif
