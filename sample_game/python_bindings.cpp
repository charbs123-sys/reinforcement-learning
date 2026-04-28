#include <array>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "engine/game_api_state.h"

namespace py = pybind11;

PYBIND11_MODULE(game_api, m) {
    m.doc() = "Python bridge for runtime game state";

    m.def("get_player_position", []() {
        const glm::vec3 pos = api_get_player_position();
        return std::array<float, 3>{pos.x, pos.y, pos.z};
    });

    m.def("get_enemy_positions", []() {
        const std::vector<glm::vec3> enemyPositions = api_get_enemy_positions();
        std::vector<std::array<float, 3>> result;
        result.reserve(enemyPositions.size());
        for (const auto& enemyPos : enemyPositions) {
            result.push_back(std::array<float, 3>{enemyPos.x, enemyPos.y, enemyPos.z});
        }
        return result;
    });

    m.def("reset_environment", []() {
        api_request_reset();
    });

    m.def("get_and_reset_hit_count", []() {
        return api_get_and_reset_hit_count();
    });

    py::enum_<ApiMovement>(m, "Movement")
        .value("NONE",     ApiMovement::NONE)
        .value("FORWARD",  ApiMovement::FORWARD)
        .value("BACKWARD", ApiMovement::BACKWARD)
        .value("LEFT",     ApiMovement::LEFT)
        .value("RIGHT",    ApiMovement::RIGHT);

    m.def("set_movement", [](ApiMovement dir) {
        api_set_movement(dir);
    });
}
