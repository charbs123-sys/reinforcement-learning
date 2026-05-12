#include "../Header Files/player.h"

#include <glm/glm.hpp>
#include <algorithm>
#include <limits>

Player::Player(glm::vec3 player_world_position)
{
    this->player_world_pos = player_world_position;
}

void Player::set_player_model(const Model& player_model)
{
    this->player_model = player_model;
}

void Player::compute_and_set_player_local_center()
{
    if (!this->player_model.has_value())
    {
        this->player_local_center = glm::vec3(0.0f);
        return;
    }

    glm::vec3 minPoint(std::numeric_limits<float>::max());
    glm::vec3 maxPoint(std::numeric_limits<float>::lowest());
    bool hasVertex = false;

    for (const auto& mesh : this->player_model->meshes)
    {
        for (const auto& vertex : mesh.vertices)
        {
            hasVertex = true;
            minPoint.x = std::min(minPoint.x, vertex.Position.x);
            minPoint.y = std::min(minPoint.y, vertex.Position.y);
            minPoint.z = std::min(minPoint.z, vertex.Position.z);
            maxPoint.x = std::max(maxPoint.x, vertex.Position.x);
            maxPoint.y = std::max(maxPoint.y, vertex.Position.y);
            maxPoint.z = std::max(maxPoint.z, vertex.Position.z);
        }
    }

    if (!hasVertex)
    {
        this->player_local_center = glm::vec3(0.0f);
        return;
    }

    this->player_local_center = (minPoint + maxPoint) * 0.5f;
}