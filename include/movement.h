#ifndef _MOVEMENT_H
#define _MOVEMENT_H

#include "transform.h"
#include "geometry.h"
#include "terrain.h"

#include <PerlinNoise.hpp>


#define TIMESCALE 0.1
#define UP glm::vec3(0.0f, 0.0f, 1.0f)

void updateMovement(float move_speed, Transform &transform, v3f target, u32 seed, Terrain &terrain)
{
    glm::vec3 to_target = glm::vec3(target.x, target.y, target.z) - transform.pos;
    glm::vec3 dir = glm::length(to_target) == 0.0f ? glm::vec3(0.0f) : glm::normalize(to_target);

    //transform.rotation.z = glm::degrees(atan2(dir.z, dir.x));
    glm::vec3 mov = dir * move_speed;
   
    glm::vec3 newpos = transform.pos + mov;
    Chunk *chunk = terrain.getChunkFromPos(v2f{newpos.x, newpos.y});

    if (chunk == nullptr) return;

    if(chunk->navigable[terrain.getTileIndexFromPos(v2f{newpos.x, newpos.y})] == 0)
    {
    transform.pos += mov;
    
    v2f v2target {target.x, target.y};
    v2f self {transform.pos.x, transform.pos.y};
    // TODO: MAKE BETTER CHECK IF UNIT IS IN VISIBLE
    if(v2target.distanceFrom(self) < 140.0)
        {
            const siv::PerlinNoise perlin(seed);
            f32 perlin_scale = 0.005f;
            f32 noise_scale = 16.0f;
            u32 octaves = 9;

            transform.pos.z = noise_scale * perlin.octave2D((self.x) * perlin_scale, (self.y) * perlin_scale, octaves);
        }
    }
}

#endif // _MOVEMENT_H
