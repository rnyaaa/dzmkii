#ifndef _SYSTEMS_H
#define _SYSTEMS_H

#include<SDL.h>

#include "common.h"
#include "scene.h"
#include "entity.h"
#include "renderer.h"
#include "input.h"
#include "player.h"
#include "boid.h"
//#define INPUTSYSTEM_ARGS Scene &scene, GUI &gui, SDL_Event e, const u8 *key_state, const u8 *prev_key_state, double delta_time
//namespace InputSystem 
//{
//    void handleInputQueue(INPUTSYSTEM_ARGS);
//    void debugCamera(INPUTSYSTEM_ARGS);
//    void worldCamera(INPUTSYSTEM_ARGS);
//}

#define GAMESYSTEM_ARGS DZRenderer &renderer, Scene &scene, InputState &input, Player &player, double delta_time

namespace GameSystem
{
    void inputActions(GAMESYSTEM_ARGS);
    void scrollWheel(GAMESYSTEM_ARGS);
    void cameraMovement(GAMESYSTEM_ARGS);
    void updatePlayer(GAMESYSTEM_ARGS);
    void terrainGeneration(GAMESYSTEM_ARGS);
    void dayNight(GAMESYSTEM_ARGS);
    void updateBoid(GAMESYSTEM_ARGS);
}

#define RENDERSYSTEM_ARGS DZRenderer &renderer, Scene &scene, InputState &input, const glm::vec2 &screen_dim, float elapsed_time

namespace RenderSystem
{
    void terrain(RENDERSYSTEM_ARGS);
    void skybox(RENDERSYSTEM_ARGS);
    void skyscrapers(RENDERSYSTEM_ARGS);
    void boids(RENDERSYSTEM_ARGS);
    void updateData(RENDERSYSTEM_ARGS);
}

#endif // _SYSTEMS_H
