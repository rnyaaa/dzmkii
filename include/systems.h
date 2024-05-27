#ifndef _SYSTEMS_H
#define _SYSTEMS_H

#include "common.h"
#include "scene.h"
#include "entity.h"
#include "renderer.h"

#define GAMESYSTEM_ARGS DZRenderer &renderer, Scene &scene, const u8 *key_state, double delta_time

namespace GameSystem
{
    void cameraMovement(GAMESYSTEM_ARGS);
    void unitMovement(GAMESYSTEM_ARGS);
    void LOS(GAMESYSTEM_ARGS);
    void terrainGeneration(GAMESYSTEM_ARGS);
}

#define RENDERSYSTEM_ARGS DZRenderer &renderer, const Scene &scene, const glm::vec2 &screen_dim

namespace RenderSystem
{
    void updateData(RENDERSYSTEM_ARGS);
    void terrain(RENDERSYSTEM_ARGS);
    void models(RENDERSYSTEM_ARGS);
}

#endif // _SYSTEMS_H
