#include "glm/ext/matrix_clip_space.hpp"
#include <camera.h>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
{
    this->position = glm::vec3(0.f, 0.f, 10.f);
    this->target = glm::vec3(0.0f, 0.0f, 0.0f);
    this->up = glm::vec3(0.0, 1.0, 0.0);
    this->forward = glm::vec3(0.0,.0, -1.0);//this->target - this->position;
    this->right  = glm::vec3(1.0, 0.0, 0.0);//glm::cross(this->right, this->forward);
    this->zoom_level = 1.0f;
   
    this->move_speed = 1.0f;
    this->zoom_speed = 0.9f;
    this->zoom_max   = 128.0f;
    this->zoom_min   = 6.0f;

    this->ortho = true;
    this->theta = 0;
    this->phi   = 0;

    this->pitch = 0;
    this->yaw   = 0;
    this->mouse_sensitivity = 0.5;
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(this->position, 
                    this->target, 
                        this->up 
            );
}

glm::mat4 Camera::getProjectionMatrix(glm::vec2 screen_dim) const
{
    if(this->ortho)
        return glm::ortho(
                -screen_dim.x / this->zoom_level,
                screen_dim.x / this->zoom_level,
              -screen_dim.y / this->zoom_level,
                  screen_dim.y / this->zoom_level,
               -10000.f,
                 1000.f
        );

    
    float aspect_ratio = screen_dim.x / screen_dim.y;
    return glm::perspective(glm::radians(90.0f), aspect_ratio, 0.001f, 1000.f);
}


CameraData Camera::getCameraData(glm::vec2 screen_dim) const
{
    return CameraData {
        glm::vec4(this->position, 0.0),
        this->getViewMatrix(),
        this->getProjectionMatrix(screen_dim)
    };
}

void Camera::move(glm::vec3 direction)
{
    this->position += this->move_speed * direction;
    this->target += this->move_speed * direction;
}

void Camera::zoom(f32 change)
{
    this->zoom_level += this->zoom_level / 16.0f * change;
    this->zoom_level = fmin(fmax(this->zoom_level, this->zoom_min), this->zoom_max);
}

void Camera::processMouseMovement(f32 deltaX, f32 deltaY)
{
    deltaX *= this->mouse_sensitivity;
    deltaY *= this->mouse_sensitivity;
    
    this->yaw += deltaX;
    this->pitch -= deltaY;

    if (this->yaw > 89.0f)
        this->yaw = 89.0f;
    if (this->yaw < -89.0f)
        this->yaw = -89.0f;

    updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    // Calculate the new forward vector
    glm::vec3 forward;
    forward.x = cos(glm::radians(this->pitch)) * cos(glm::radians(this->yaw));
    forward.y = sin(glm::radians(this->pitch)) * cos(glm::radians(this->yaw));
    forward.z = sin(glm::radians(-this->yaw));

    // Normalize the forward vector
    this->forward = glm::normalize(forward);

    // Recalculate the right and up vectors
    this->up = glm::vec3(0.f, 0.f, 1.f); 
    this->right = glm::normalize(glm::cross(this->up, this->forward));
}

glm::vec3 Camera::getViewDirection() const
{
    return this->position - this->target;
}

