#include <camera.h>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
{
    this->position = glm::vec3(10.0f, 10.0f, 10.0f);
    this->target = glm::vec3(0.0f, 0.0f, 0.0f);
    this->zoom_level = 24.0f;
    
    this->move_speed = 1.0f;
    this->zoom_speed = 1.0f;
    this->zoom_max = 64.0f;
    this->zoom_min = 12.0f;
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(this->position, 
                       this->target, 
                       glm::vec3(0.0f, 0.0f, 1.0f)
            );
}

glm::mat4 Camera::getProjectionMatrix(glm::vec2 screen_dim) const
{
    return glm::ortho(
            screen_dim.x / -this->zoom_level,
            screen_dim.x /  this->zoom_level,
            screen_dim.y / -this->zoom_level,
            screen_dim.y /  this->zoom_level,
            -1000.f,
            1000.f
        );
}


CameraData Camera::getCameraData(glm::vec2 screen_dim) const
{
    return CameraData {
        this->getViewMatrix(),
        this->getProjectionMatrix(screen_dim)
    };
}

void Camera::move(glm::vec3 direction)
{
    this->position += this->move_speed * direction;
    this->target   += this->move_speed * direction;
}

void Camera::zoom(f32 change)
{
    this->zoom_level = fmin(fmax(this->zoom_level + this->zoom_speed * change, this->zoom_min), this->zoom_max);
} 
