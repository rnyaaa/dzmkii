#include "glm/ext/matrix_clip_space.hpp"
#include <camera.h>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
{
    this->position = glm::vec3(10.0f, 10.0f, 10.0f);
    this->target = glm::vec3(0.0f, 0.0f, 0.0f);
    this->zoom_level = 24.0f;
   
    this->move_speed = 1.0f;
    this->zoom_speed = 0.9f;
    this->zoom_max = 128.0f;
    this->zoom_min = 6.0f;

    this->ortho = true;
    this->theta = 0;
    this->phi = 0;
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
    return glm::perspective(glm::radians(90.0f), aspect_ratio, 0.01f, 1000.f);
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
    this->target += this->move_speed * direction;
}

void Camera::zoom(f32 change)
{
    this->zoom_level += this->zoom_level / 16.0f * change;
    this->zoom_level = fmin(fmax(this->zoom_level, this->zoom_min), this->zoom_max);
}

void Camera::rotateWithOrigin(v2f dir)
{
    f64 radius = v3f{position.x, position.y, position.z}
                .distanceFrom(v3f{target.x, target.y, target.z});

    printf("Target: %f %f %f\n", this->target.x,this->target.y,this->target.z);
    printf("Position: %f %f %f\n", this->position.x,this->position.y,this->position.z);
    printf("Radius: %f\n", radius);

    theta += dir.x / 50.0f;
    phi += dir.y / 50.0f;

    f32 a = radius * cos(theta);

    f32 Cx = a * cos(phi);
    f32 Cy = a * sin(phi);
    f32 Cz = radius * sin(theta);
    
    this->position = glm::vec3(Cx, Cy, Cz);

    printf("Updated position: %f %f %f\n", this->position.x,this->position.y,this->position.z);
}

glm::vec3 Camera::getViewDirection() const
{
    return this->position - this->target;
}

std::array<glm::vec3, 4> Camera::getFrustrumBounds(glm::vec2 screen_dim) const
{
    glm::vec3 view_dir  = this->getViewDirection();
    glm::vec3 left_vec = glm::cross(view_dir, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec3 camera_up = glm::cross(left_vec, view_dir);

    left_vec = glm::normalize(left_vec);
    camera_up = glm::normalize(camera_up);

    return {
        this->position 
            + screen_dim.y / this->zoom_level * camera_up
            + screen_dim.x / this->zoom_level * left_vec,
        this->position 
            + screen_dim.y / this->zoom_level * camera_up
            - screen_dim.x / this->zoom_level * left_vec,
        this->position 
            - screen_dim.y / this->zoom_level * camera_up
            - screen_dim.x / this->zoom_level * left_vec,
        this->position 
            - screen_dim.y / this->zoom_level * camera_up
            + screen_dim.x / this->zoom_level * left_vec,
    };
}
