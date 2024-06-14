#ifndef _SYSTEMS_H
#define _SYSTEMS_H

#include<SDL.h>

#include "common.h"
#include "scene.h"
#include "entity.h"
#include "renderer.h"
#include "input.h"
#include "gui.h"
//#define INPUTSYSTEM_ARGS Scene &scene, GUI &gui, SDL_Event e, const u8 *key_state, const u8 *prev_key_state, double delta_time
//namespace InputSystem 
//{
//    void handleInputQueue(INPUTSYSTEM_ARGS);
//    void debugCamera(INPUTSYSTEM_ARGS);
//    void worldCamera(INPUTSYSTEM_ARGS);
//}

#define GAMESYSTEM_ARGS DZRenderer &renderer, Scene &scene, InputState &input, GUI &gui, double delta_time

namespace GameSystem
{
    void inputActions(GAMESYSTEM_ARGS);
    void scrollWheel(GAMESYSTEM_ARGS);
    void debugControl(GAMESYSTEM_ARGS);
    void cameraMovement(GAMESYSTEM_ARGS);
    void unitMovement(GAMESYSTEM_ARGS);
    void LOS(GAMESYSTEM_ARGS);
    void terrainGeneration(GAMESYSTEM_ARGS);
}

#define RENDERSYSTEM_ARGS DZRenderer &renderer, const Scene &scene, InputState &input, const glm::vec2 &screen_dim, GUI &gui, float elapsed_time

namespace RenderSystem
{
    void updateData(RENDERSYSTEM_ARGS);
    void terrain(RENDERSYSTEM_ARGS);
    void models(RENDERSYSTEM_ARGS);
    void gui(RENDERSYSTEM_ARGS);
    void fow(RENDERSYSTEM_ARGS);
}

#endif // _SYSTEMS_H
