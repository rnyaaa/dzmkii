#include "boid.h"
#include "renderer.h"
#include "light.h"
#include <random>

bool Boid::mesh_generated = false;
bool Boid::mesh_registered = false;
DZMesh Boid::mesh = DZInvalid;
MeshData Boid::mesh_data;

Boid::Boid(glm::vec3 pos){
    this->velocity = {
        rand() % 50 / 50.0f - 0.5f, 
        rand() % 50 / 50.0f - 0.5f, 
        rand() % 50 / 50.0f - 0.5f
    };
    this->boid_detection_range = 20.f;
    this->boid_avoidance_range = 5.f;
    this->target_detection_range = 20.f;
    this->damage = 1.f;
    this->health = 1.f;
    this->scale = (rand()%25 / 25.f) + 0.25f;
    this->ischasing = false;
    this->light = DynamicPointLightData{this->pos, glm::vec3(1.0, 1.0, 1.0), 1.f}; 
    this->chasing_color = glm::vec3(0.096, 0.0012, 0.012);
    this->color = glm::vec3(0.024, 0.0024, 0.024);
    this->pos = pos;
    if (!Boid::mesh_generated)
    {
        Boid::mesh_data = MeshData::UnitSphere();
    }
};

void Boid::updateUniforms(DZRenderer &renderer)
{
    if (!Boid::mesh_registered)
    {
        Log::verbose("\tRegistering mesh with renderer...");
        Boid::mesh = renderer.createMesh(Boid::mesh_data);
        Log::verbose("\tMesh registered...");
        Boid::mesh_registered = true;
    }

    if (!this->has_uniforms_buffer)
    {
        this->local_uniforms_buffer = 
            renderer.createBufferOfSize(sizeof(BoidData), StorageMode::MANAGED);
        this->has_uniforms_buffer = true;
    }

    BoidData boid_data;

    Transform transform;
    transform.pos = this->pos;
    transform.scale = glm::vec3(this->scale);
    boid_data.model_matrix = transform.asMat4();
    boid_data.ischasing = this->ischasing;

    renderer.setBufferOfSize(
            local_uniforms_buffer, 
            &boid_data, 
            sizeof(BoidData)
        );
}

