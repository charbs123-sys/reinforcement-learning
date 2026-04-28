#include "game_api_state.h"

#include <atomic>
#include <mutex>

namespace {
std::mutex g_stateMutex;
glm::vec3 g_playerPos(0.0f, 0.0f, 0.0f);
std::vector<glm::vec3> g_enemyPositions;
std::atomic<bool> g_resetRequested{false};
std::atomic<int> g_hitCount{0};
std::atomic<int> g_movementAction{0};
}

void api_update_snapshot(const glm::vec3& playerPos, const std::vector<EnemyProperties>& enemies) {
    std::lock_guard<std::mutex> lock(g_stateMutex);
    g_playerPos = playerPos;
    g_enemyPositions.clear();
    g_enemyPositions.reserve(enemies.size());
    for (const auto& enemy : enemies) {
        g_enemyPositions.push_back(enemy.enemyPos);
    }
}

glm::vec3 api_get_player_position() {
    std::lock_guard<std::mutex> lock(g_stateMutex);
    return g_playerPos;
}

std::vector<glm::vec3> api_get_enemy_positions() {
    std::lock_guard<std::mutex> lock(g_stateMutex);
    return g_enemyPositions;
}

void api_request_reset() {
    g_resetRequested.store(true, std::memory_order_release);
}

bool api_consume_reset_request() {
    return g_resetRequested.exchange(false, std::memory_order_acq_rel);
}

void api_record_enemy_hit() {
    g_hitCount.fetch_add(1, std::memory_order_relaxed);
}

int api_get_and_reset_hit_count() {
    return g_hitCount.exchange(0, std::memory_order_acq_rel);
}

void api_set_movement(ApiMovement dir) {
    g_movementAction.store(static_cast<int>(dir), std::memory_order_release);
}

ApiMovement api_consume_movement() {
    return static_cast<ApiMovement>(g_movementAction.exchange(0, std::memory_order_acq_rel));
}
