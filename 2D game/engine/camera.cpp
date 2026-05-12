#include "../Header Files/camera.h"
#include "../Header Files/sprite_sheet.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
    Position = position;
    world_pos_y_up = up;
    Yaw = yaw;
    Pitch = pitch;
    update_camera_vectors();
}

void Camera::update_camera_vectors()
{
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, world_pos_y_up));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up    = glm::normalize(glm::cross(Right, Front));
}

glm::mat4 Camera::get_view_matrix(glm::vec3 camera_pos, glm::vec3 target_pos, bool fix_camera)
{   
    if (fix_camera)
    {
        return glm::lookAt(camera_pos, target_pos, world_pos_y_up);
    }
    else 
    {
        return glm::lookAt(Position, Position + Front, Up);
    }
}

void Camera::process_camera_movement(Camera_Movement direction, float deltaTime, SpriteSheet& player)
{
    float velocity = MovementSpeed * deltaTime;
    glm::vec3 delta = glm::vec3(0.0f);

    if (direction == FORWARD)  delta = Up * velocity;
    if (direction == BACKWARD) delta = -(Up * velocity);
    if (direction == LEFT)     delta = -(Right * velocity);
    if (direction == RIGHT)    delta = Right * velocity;

    player.object_world_center.x += delta.x;
    Position.x += delta.x;

    player.object_world_center.y += delta.y;
    Position.y += delta.y;

}

void Camera::process_mouse_scroll(float yoffset)
{
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

void Camera::set_relative_camera_position(glm::vec3 player_world_position)
{
    this->Position = player_world_position + camera_offset_relative;
}