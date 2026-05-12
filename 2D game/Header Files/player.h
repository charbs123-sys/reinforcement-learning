#ifndef PLAYER_H
#define PLAYER_H

#include "model.h"

#include <glm/glm.hpp>
#include <optional>


class Player
{
    public:
        glm::vec3 player_local_center;
        glm::vec3 player_world_pos;
        std::optional<Model> player_model;

        Player(glm::vec3 player_world_position);
        void set_player_model(const Model& player_model);
        void compute_and_set_player_local_center();

    private:
};

#endif