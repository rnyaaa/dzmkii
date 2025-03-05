#include <glm/glm.hpp>
#include <array>
#include <math.h>
#include "common.h"
#include "geometry.h"

#ifndef _CAMERA_H
#define _CAMERA_H

struct CameraData
{
    glm::vec4 position;
    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
};

struct Camera
{
    glm::vec3 position; // 10.0f, 10.0f, 10.0f
    glm::vec3 target;   // 0.0f, 0.0f, 0.0f
    glm::vec3 up;       // 0.0f, 0.0f, 1.0f
    glm::vec3 forward;  // target - position
    glm::vec3 right;

    f32 zoom_level;

    f32 move_speed;
    f32 zoom_speed;
    f32 zoom_max;
    f32 zoom_min;

    f32 theta;
    f32 phi;

    f32 pitch;
    f32 yaw;

    f32 mouse_sensitivity;

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(glm::vec2 screen_dim) const;
    glm::vec3 getViewDirection() const;
    std::array<glm::vec3, 4> getFrustrumBounds(glm::vec2 screen_dim) const; 

    CameraData getCameraData(glm::vec2 screen_dim) const;

    void move(glm::vec3 direction);
    void rotateWithOrigin(v2f dir);
    void zoom(f32 change);
    void processMouseMovement(f32 deltax, f32 deltay);
    void updateCameraVectors();
    bool ortho;

    Camera();

};


#endif
