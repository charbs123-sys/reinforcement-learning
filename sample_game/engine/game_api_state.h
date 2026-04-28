#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "functions.h"

void api_update_snapshot(const glm::vec3& playerPos, const std::vector<EnemyProperties>& enemies);
glm::vec3 api_get_player_position();
std::vector<glm::vec3> api_get_enemy_positions();

void api_request_reset();
bool api_consume_reset_request();

void api_record_enemy_hit();
int api_get_and_reset_hit_count();

enum class ApiMovement { NONE = 0, FORWARD = 1, BACKWARD = 2, LEFT = 3, RIGHT = 4 };
void api_set_movement(ApiMovement dir);
ApiMovement api_consume_movement();
